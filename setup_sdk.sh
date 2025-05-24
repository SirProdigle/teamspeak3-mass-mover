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

# Set SDK version and URLs
SDK_VERSION=26
SDK_ARCHIVE="${SDK_VERSION}.tar.gz"
SDK_URL="https://github.com/teamspeak/ts3client-pluginsdk/archive/refs/tags/${SDK_VERSION}.tar.gz"
SDK_DIR="ts3client-pluginsdk-${SDK_VERSION}"

# Download the SDK
if [ -d "$SDK_DIR" ]; then
    echo -e "${GREEN}SDK directory already exists: $SDK_DIR${NC}"
    exit 0
fi

echo -e "${YELLOW}Downloading TeamSpeak 3 Client SDK...${NC}"
wget -O "$SDK_ARCHIVE" "$SDK_URL"
if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Failed to download the SDK${NC}"
    exit 1
fi

echo -e "${YELLOW}Extracting SDK...${NC}"
tar -xzf "$SDK_ARCHIVE"
if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Failed to extract the SDK${NC}"
    rm -f "$SDK_ARCHIVE"
    exit 1
fi

# Rename the extracted folder to match README instructions
EXTRACTED_DIR="ts3client-pluginsdk-${SDK_VERSION}"
if [ ! -d "$EXTRACTED_DIR" ]; then
    # Try to find the extracted directory
    EXTRACTED_DIR=$(tar -tzf "$SDK_ARCHIVE" | head -1 | cut -f1 -d"/")
fi

if [ "$EXTRACTED_DIR" != "$SDK_DIR" ] && [ -d "$EXTRACTED_DIR" ]; then
    mv "$EXTRACTED_DIR" "$SDK_DIR"
fi

# Clean up
rm -f "$SDK_ARCHIVE"

echo -e "${GREEN}SDK setup complete!${NC}"
echo "SDK directory: $SDK_DIR"
echo "You can now run ./build.sh to build the plugin" 