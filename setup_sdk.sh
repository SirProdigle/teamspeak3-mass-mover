#!/bin/bash

# TeamSpeak 3 SDK Setup Script
# This script downloads and sets up the TeamSpeak 3 Client SDK

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}TeamSpeak 3 SDK Setup Script${NC}"
echo "This script will download and set up the TeamSpeak 3 Client SDK"
echo

# Check if wget is installed
if ! command -v wget &> /dev/null; then
    echo -e "${RED}Error: wget is not installed${NC}"
    echo "Please install wget and try again"
    exit 1
fi

# Create a temporary directory
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

echo -e "${YELLOW}Downloading TeamSpeak 3 Client SDK...${NC}"
# Download the SDK
wget -q --show-progress https://files.teamspeak-services.com/releases/client/3.6.1/TeamSpeak3-Client-sdk_3.6.1.tar.bz2

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Failed to download the SDK${NC}"
    rm -rf "$TEMP_DIR"
    exit 1
fi

echo -e "${YELLOW}Extracting SDK...${NC}"
# Extract the SDK
tar xf TeamSpeak3-Client-sdk_3.6.1.tar.bz2

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Failed to extract the SDK${NC}"
    rm -rf "$TEMP_DIR"
    exit 1
fi

# Move the SDK to the project directory
echo -e "${YELLOW}Moving SDK to project directory...${NC}"
mv ts3client-pluginsdk-* "$OLDPWD"

# Clean up
cd "$OLDPWD"
rm -rf "$TEMP_DIR"

echo -e "${GREEN}SDK setup complete!${NC}"
echo "You can now run ./build.sh to build the plugin" 