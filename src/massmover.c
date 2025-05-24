/*
 * TeamSpeak 3 MassMover Plugin
 * 
 * This plugin adds a "MassMove here" context menu option to TeamSpeak 3 channels.
 * When activated, it moves all users from the target channel and its entire
 * subchannel hierarchy to the right-clicked channel.
 *
 * Key Features:
 * - One-click mass move of all users from a channel and its subchannels
 * - Recursive channel traversal to find all users
 * - Cross-platform support (Windows and Linux)
 * - Safe memory management and error handling
 * - Detailed logging for troubleshooting
 *
 * Author: Generated Plugin
 * Version: 1.2.0
 * License: Free to use and modify
 */

#if defined(WIN32) || defined(__WIN32__) || defined(_WIN32)
#pragma warning(disable : 4100) /* Disable Unreferenced parameter warning */
#include <windows.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TeamSpeak 3 SDK Headers */
#include "teamspeak/public_definitions.h"
#include "teamspeak/public_errors.h"
#include "teamspeak/public_errors_rare.h"
#include "teamspeak/public_rare_definitions.h"
#include "ts3_functions.h"
#include "plugin_definitions.h"

#include "massmover.h"

/* Global Variables */
static struct TS3Functions ts3Functions;  /* TeamSpeak 3 API function pointers */
static char* pluginID = NULL;            /* Plugin's unique identifier */

/* Constants */
#define PLUGIN_API_VERSION 26            /* TeamSpeak 3 API version we're using */
#define PATH_BUFSIZE 512                 /* Buffer size for file paths */
#define RETURNCODE_BUFSIZE 128           /* Buffer size for return codes */
#define MENU_ID_MASSMOVE 1              /* ID for our context menu item */

/* Platform-specific string handling */
#ifdef _WIN32
#define _strcpy(dest, destSize, src) strcpy_s(dest, destSize, src)
#define snprintf sprintf_s
#else
#define _strcpy(dest, destSize, src) \
    { \
        strncpy(dest, src, destSize - 1); \
        (dest)[destSize - 1] = '\0'; \
    }
#endif

/* Function Prototypes */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon);
static void collectSubchannels(uint64 serverConnectionHandlerID, uint64 parentChannelID, uint64** channels, int* channelCount, int* capacity);
static void collectParentChannels(uint64 serverConnectionHandlerID, uint64 channelID, uint64** channels, int* channelCount, int* capacity);
static anyID* collectClientsFromChannels(uint64 serverConnectionHandlerID, uint64* channels, int channelCount, int* clientCount);

#ifdef _WIN32
/* Helper function to convert wchar_T to Utf-8 encoded strings on Windows */
static int wcharToUtf8(const wchar_t* str, char** result)
{
    int outlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, 0, 0);
    *result = (char*)malloc(outlen);
    if (WideCharToMultiByte(CP_UTF8, 0, str, -1, *result, outlen, 0, 0) == 0) {
        *result = NULL;
        return -1;
    }
    return 0;
}
#endif

/*********************************** Required Plugin Functions ************************************/

/* Unique name identifying this plugin */
const char* ts3plugin_name()
{
#ifdef _WIN32
    static char* result = NULL;
    if (!result) {
        const wchar_t* name = L"MassMover";
        if (wcharToUtf8(name, &result) == -1) {
            result = "MassMover";
        }
    }
    return result;
#else
    return "MassMover";
#endif
}

/* Plugin version */
const char* ts3plugin_version()
{
    return "1.2.0";
}

/* Plugin API version */
int ts3plugin_apiVersion()
{
    return PLUGIN_API_VERSION;
}

/* Plugin author */
const char* ts3plugin_author()
{
    return "Generated Plugin";
}

/* Plugin description */
const char* ts3plugin_description()
{
    return "Mass move all users from a channel and its subchannels to the target channel.";
}

/* Set TeamSpeak 3 callback functions */
void ts3plugin_setFunctionPointers(const struct TS3Functions funcs)
{
    ts3Functions = funcs;
}

/* Plugin initialization */
int ts3plugin_init()
{
    printf("MASSMOVER: Plugin initialized\n");
    return 0;
}

/* Plugin cleanup */
void ts3plugin_shutdown()
{
    printf("MASSMOVER: Plugin shutdown\n");
    
    if (pluginID) {
        free(pluginID);
        pluginID = NULL;
    }
}

/****************************** Optional Plugin Functions ********************************/

/* Tell client we don't offer a configuration window */
int ts3plugin_offersConfigure()
{
    return PLUGIN_OFFERS_NO_CONFIGURE;
}

/* Register plugin ID */
void ts3plugin_registerPluginID(const char* id)
{
    const size_t sz = strlen(id) + 1;
    pluginID = (char*)malloc(sz * sizeof(char));
    _strcpy(pluginID, sz, id);
    printf("MASSMOVER: registerPluginID: %s\n", pluginID);
}

/* Helper function to create menu item */
static struct PluginMenuItem* createMenuItem(enum PluginMenuType type, int id, const char* text, const char* icon)
{
    struct PluginMenuItem* menuItem = (struct PluginMenuItem*)malloc(sizeof(struct PluginMenuItem));
    if (!menuItem) {
        printf("MASSMOVER: Failed to allocate memory for menu item\n");
        return NULL;
    }
    
    menuItem->type = type;
    menuItem->id = id;
    _strcpy(menuItem->text, PLUGIN_MENU_BUFSZ, text);
    _strcpy(menuItem->icon, PLUGIN_MENU_BUFSZ, icon);
    return menuItem;
}

/* Initialize menus - add our context menu item */
void ts3plugin_initMenus(struct PluginMenuItem*** menuItems, char** menuIcon)
{
    struct PluginMenuItem** items;
    
    /* Allocate memory for 2 menu items (1 item + NULL terminator) */
    items = (struct PluginMenuItem**)malloc(2 * sizeof(struct PluginMenuItem*));
    if (!items) {
        printf("MASSMOVER: Failed to allocate memory for menu items\n");
        *menuItems = NULL;
        *menuIcon = NULL;
        return;
    }
    
    /* Create the "MassMove here" menu item for channels */
    items[0] = createMenuItem(PLUGIN_MENU_TYPE_CHANNEL, MENU_ID_MASSMOVE, "MassMove here", "");
    if (!items[0]) {
        free(items);
        *menuItems = NULL;
        *menuIcon = NULL;
        return;
    }
    
    /* Terminate array with NULL */
    items[1] = NULL;
    
    /* Return the items */
    *menuItems = items;
    
    /* Optional icon for the plugin menu (not used here) */
    *menuIcon = (char*)malloc(PLUGIN_MENU_BUFSZ * sizeof(char));
    if (*menuIcon) {
        _strcpy(*menuIcon, PLUGIN_MENU_BUFSZ, "");
    }
}

/* Recursive function to collect all subchannels of a given channel */
static void collectSubchannels(uint64 serverConnectionHandlerID, uint64 parentChannelID, uint64** channels, int* channelCount, int* capacity)
{
    uint64* channelList;
    unsigned int error;
    int i;
    
    /* Get list of all channels on the server */
    error = ts3Functions.getChannelList(serverConnectionHandlerID, &channelList);
    if (error != ERROR_ok) {
        printf("MASSMOVER: Error getting channel list: %d\n", error);
        return;
    }
    
    /* Check each channel to see if it's a subchannel of parentChannelID */
    for (i = 0; channelList[i] != 0; i++) {
        uint64 channelParent;
        
        /* Get the parent of this channel */
        error = ts3Functions.getParentChannelOfChannel(serverConnectionHandlerID, channelList[i], &channelParent);
        if (error != ERROR_ok) {
            continue;
        }
        
        /* If this channel's parent is our target parent, add it and recurse */
        if (channelParent == parentChannelID) {
            /* Expand array if needed */
            if (*channelCount >= *capacity) {
                *capacity *= 2;
                *channels = (uint64*)realloc(*channels, *capacity * sizeof(uint64));
                if (!*channels) {
                    printf("MASSMOVER: Failed to reallocate memory for channels\n");
                    ts3Functions.freeMemory(channelList);
                    return;
                }
            }
            
            /* Add this channel to our list */
            (*channels)[*channelCount] = channelList[i];
            (*channelCount)++;
            
            /* Recursively collect subchannels of this channel */
            collectSubchannels(serverConnectionHandlerID, channelList[i], channels, channelCount, capacity);
        }
    }
    
    /* Free the channel list */
    ts3Functions.freeMemory(channelList);
}

/* Function to collect parent channels up to a certain level */
static void collectParentChannels(uint64 serverConnectionHandlerID, uint64 channelID, uint64** channels, int* channelCount, int* capacity)
{
    uint64 currentChannelID = channelID;
    unsigned int error;
    
    while (1) {
        uint64 parentChannelID;
        
        /* Get the parent of current channel */
        error = ts3Functions.getParentChannelOfChannel(serverConnectionHandlerID, currentChannelID, &parentChannelID);
        if (error != ERROR_ok || parentChannelID == 0) {
            break;  /* Stop if we hit an error or reach the root channel */
        }
        
        /* Expand array if needed */
        if (*channelCount >= *capacity) {
            *capacity *= 2;
            *channels = (uint64*)realloc(*channels, *capacity * sizeof(uint64));
            if (!*channels) {
                printf("MASSMOVER: Failed to reallocate memory for parent channels\n");
                return;
            }
        }
        
        /* Add parent channel to our list */
        (*channels)[*channelCount] = parentChannelID;
        (*channelCount)++;
        
        /* Move up to parent channel */
        currentChannelID = parentChannelID;
    }
}

/* Function to collect all clients from a list of channels */
static anyID* collectClientsFromChannels(uint64 serverConnectionHandlerID, uint64* channels, int channelCount, int* clientCount)
{
    anyID* allClients = NULL;
    int totalClients = 0;
    int capacity = 16;
    int i, j;
    
    /* Allocate initial array */
    allClients = (anyID*)malloc(capacity * sizeof(anyID));
    if (!allClients) {
        printf("MASSMOVER: Failed to allocate memory for clients\n");
        return NULL;
    }
    
    /* For each channel, get its clients */
    for (i = 0; i < channelCount; i++) {
        anyID* channelClients;
        unsigned int error;
        
        /* Get clients in this channel */
        error = ts3Functions.getChannelClientList(serverConnectionHandlerID, channels[i], &channelClients);
        if (error != ERROR_ok) {
            printf("MASSMOVER: Error getting clients for channel %llu: %d\n", (unsigned long long)channels[i], error);
            continue;
        }
        
        /* Add each client to our list */
        for (j = 0; channelClients[j] != 0; j++) {
            /* Expand array if needed */
            if (totalClients >= capacity - 1) { /* Leave room for terminator */
                capacity *= 2;
                allClients = (anyID*)realloc(allClients, capacity * sizeof(anyID));
                if (!allClients) {
                    printf("MASSMOVER: Failed to reallocate memory for clients\n");
                    ts3Functions.freeMemory(channelClients);
                    return NULL;
                }
            }
            
            /* Add client to list */
            allClients[totalClients] = channelClients[j];
            totalClients++;
        }
        
        /* Free the channel client list */
        ts3Functions.freeMemory(channelClients);
    }
    
    /* Terminate the array with 0 */
    allClients[totalClients] = 0;
    
    *clientCount = totalClients;
    return allClients;
}

/* Handle menu item events */
void ts3plugin_onMenuItemEvent(uint64 serverConnectionHandlerID, enum PluginMenuType type, int menuItemID, uint64 selectedItemID)
{
    if (type == PLUGIN_MENU_TYPE_CHANNEL && menuItemID == MENU_ID_MASSMOVE) {
        uint64 targetChannelID = selectedItemID;
        uint64* channels = NULL;
        int channelCount = 1; /* Start with 1 for the target channel itself */
        int capacity = 1024;
        anyID* clientsToMove = NULL;
        int clientCount;
        char returnCode[RETURNCODE_BUFSIZE];
        unsigned int error;
        char msg[512];
        anyID myID;
        int i;
        int myIDFound = 0;
        
        /* Get our own client ID */
        error = ts3Functions.getClientID(serverConnectionHandlerID, &myID);
        if (error != ERROR_ok) {
            ts3Functions.logMessage("MassMover: Failed to get client ID", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
            return;
        }
        
        snprintf(msg, sizeof(msg), "MassMover: Starting mass move operation for channel %llu", (unsigned long long)targetChannelID);
        ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);
        
        /* Allocate initial channel array and add the target channel */
        channels = (uint64*)malloc(capacity * sizeof(uint64));
        if (!channels) {
            ts3Functions.logMessage("MassMover: Failed to allocate memory for channels", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
            return;
        }
        channels[0] = targetChannelID;
        
        /* Collect parent channels */
        collectParentChannels(serverConnectionHandlerID, targetChannelID, &channels, &channelCount, &capacity);
        
        /* For each channel (including target and parents), collect their subchannels */
        for (i = 0; i < channelCount; i++) {
            collectSubchannels(serverConnectionHandlerID, channels[i], &channels, &channelCount, &capacity);
        }
        
        snprintf(msg, sizeof(msg), "MassMover: Found %d channels to move clients from", channelCount);
        ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);
        
        /* Collect all clients from these channels */
        clientsToMove = collectClientsFromChannels(serverConnectionHandlerID, channels, channelCount, &clientCount);
        if (!clientsToMove) {
            ts3Functions.logMessage("MassMover: Failed to collect clients from channels", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
            free(channels);
            return;
        }
        
        snprintf(msg, sizeof(msg), "MassMover: Found %d clients to move", clientCount);
        ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);
        
        if (clientCount > 0) {
            /* Create return code for the move operation */
            ts3Functions.createReturnCode(pluginID, returnCode, RETURNCODE_BUFSIZE);
            
            /* First move all other clients */
            anyID* otherClients = NULL;
            int otherClientCount = 0;
            int otherClientCapacity = 16;
            
            /* Allocate initial array for other clients */
            otherClients = (anyID*)malloc(otherClientCapacity * sizeof(anyID));
            if (!otherClients) {
                ts3Functions.logMessage("MassMover: Failed to allocate memory for other clients", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
                free(channels);
                free(clientsToMove);
                return;
            }
            
            /* Filter out our own ID from the client list */
            for (i = 0; i < clientCount; i++) {
                if (clientsToMove[i] == myID) {
                    myIDFound = 1;
                    continue;
                }
                
                /* Expand array if needed */
                if (otherClientCount >= otherClientCapacity - 1) {
                    otherClientCapacity *= 2;
                    otherClients = (anyID*)realloc(otherClients, otherClientCapacity * sizeof(anyID));
                    if (!otherClients) {
                        ts3Functions.logMessage("MassMover: Failed to reallocate memory for other clients", LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
                        free(channels);
                        free(clientsToMove);
                        return;
                    }
                }
                
                otherClients[otherClientCount] = clientsToMove[i];
                otherClientCount++;
            }
            
            /* Terminate the array with 0 */
            if (otherClientCount > 0) {
                otherClients[otherClientCount] = 0;
                
                /* Move other clients */
                error = ts3Functions.requestClientsMove(serverConnectionHandlerID, otherClients, targetChannelID, "", returnCode);
                if (error != ERROR_ok) {
                    snprintf(msg, sizeof(msg), "MassMover: Error moving other clients: %d", error);
                    ts3Functions.logMessage(msg, LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
                }
            }
            
            /* Move ourselves separately */
            if (myIDFound) {
                error = ts3Functions.requestClientMove(serverConnectionHandlerID, myID, targetChannelID, "", returnCode);
                if (error != ERROR_ok) {
                    snprintf(msg, sizeof(msg), "MassMover: Error moving self: %d", error);
                    ts3Functions.logMessage(msg, LogLevel_ERROR, "Plugin", serverConnectionHandlerID);
                }
            }
            
            snprintf(msg, sizeof(msg), "MassMover: Successfully initiated move for %d clients to channel %llu", clientCount, (unsigned long long)targetChannelID);
            ts3Functions.logMessage(msg, LogLevel_INFO, "Plugin", serverConnectionHandlerID);
            
            free(otherClients);
        } else {
            ts3Functions.logMessage("MassMover: No clients found to move", LogLevel_INFO, "Plugin", serverConnectionHandlerID);
        }
        
        /* Cleanup */
        free(channels);
        free(clientsToMove);
    }
}

/* Error handling */
int ts3plugin_onServerErrorEvent(uint64 serverConnectionHandlerID, const char* errorMessage, unsigned int error, const char* returnCode, const char* extraMessage)
{
    if (returnCode && pluginID && strcmp(returnCode, pluginID) == 0) {
        printf("MASSMOVER: Server error: %s (%d)\n", errorMessage, error);
    }
    return 0; /* Let other plugins handle this too */
}

/****************************** Unused Plugin Callbacks ********************************/

/* All the remaining callback functions that we don't need */
void ts3plugin_currentServerConnectionChanged(uint64 serverConnectionHandlerID) {}
const char* ts3plugin_infoTitle() { return NULL; }
void ts3plugin_infoData(uint64 serverConnectionHandlerID, uint64 id, enum PluginItemType type, char** data) { *data = NULL; }
void ts3plugin_freeMemory(void* data) { free(data); }
int ts3plugin_requestAutoload() { return 0; }
void ts3plugin_initHotkeys(struct PluginHotkey*** hotkeys) { *hotkeys = NULL; }

