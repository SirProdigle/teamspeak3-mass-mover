# TeamSpeak 3 MassMover Plugin

A TeamSpeak 3 client plugin that adds a convenient "MassMove here" context menu option to channels. When activated, it moves all users from the target channel and its entire subchannel hierarchy to the right-clicked channel.

## Features

- 🚀 **One-click mass move**: Right-click any channel and select "MassMove here"
- 🌳 **Recursive channel traversal**: Automatically finds and moves users from all subchannels
- 🔧 **Cross-platform**: Supports both Windows and Linux
- ⚡ **Lightweight**: Minimal resource usage with efficient algorithms
- 🛡️ **Safe**: Proper error handling and memory management

## How It Works

1. Right-click on any channel in the TeamSpeak channel tree
2. Select "MassMove here" from the context menu
3. The plugin will:
   - Find all users in the target channel
   - Recursively search all subchannels of that channel
   - Move all found users to the right-clicked channel

## Building the Plugin

### Prerequisites

#### Linux
- GCC compiler
- Standard C development tools
- TeamSpeak 3 Client SDK (included)

#### Windows
- One of the following:
  - MinGW-w64 (recommended)
  - Microsoft Visual Studio/Visual C++
  - Or use cross-compilation from Linux

### Build Instructions

#### Option 1: Cross-platform build script (recommended)
```bash
chmod +x build.sh
./build.sh
```

#### Option 2: Linux-specific build
```bash
make -f Makefile.linux
```

#### Option 3: Windows batch script
```cmd
build_windows.bat
```

#### Option 4: Manual compilation

**Linux:**
```bash
mkdir -p build/linux bin/linux
gcc -c -O2 -Wall -fPIC -std=gnu99 -Its3client-pluginsdk-26/include src/massmover.c -o build/linux/massmover.o
gcc -shared -o bin/linux/massmover.so build/linux/massmover.o
```

**Windows (MinGW):**
```cmd
mkdir build\windows bin\windows
gcc -c -O2 -Wall -DWIN32 -Its3client-pluginsdk-26/include src/massmover.c -o build/windows/massmover.o
gcc -shared -o bin/windows/massmover.dll build/windows/massmover.o
```

## Installation

### Linux
1. Build the plugin (see above)
2. Copy `bin/linux/massmover.so` to `~/.ts3client/plugins/`
3. Restart TeamSpeak 3
4. Go to Settings > Plugins and enable "MassMover"

### Windows
1. Build the plugin (see above)
2. Copy `bin/windows/massmover.dll` to `%APPDATA%\TS3Client\plugins\`
3. Restart TeamSpeak 3
4. Go to Settings > Plugins and enable "MassMover"

## Usage

1. Connect to a TeamSpeak server
2. Right-click on any channel in the channel list
3. Select "MassMove here" from the context menu
4. All users from that channel and its subchannels will be moved to the target channel

## Use Cases

- **Event organization**: Quickly gather all participants from various discussion channels
- **Server management**: Consolidate users during maintenance or restructuring
- **Gaming sessions**: Move team members from different voice channels to a main channel
- **Meetings**: Collect attendees from breakout rooms into the main conference channel

## Technical Details

### Plugin Architecture
- **Language**: C (C99 standard)
- **API**: TeamSpeak 3 Client Plugin SDK v26
- **Memory management**: Proper allocation/deallocation with error handling
- **Algorithm**: Recursive depth-first search for channel hierarchy traversal

### Key Functions
- `collectSubchannels()`: Recursively finds all subchannels of a given channel
- `collectClientsFromChannels()`: Gathers all clients from a list of channels  
- `ts3plugin_onMenuItemEvent()`: Handles the menu click event and orchestrates the move operation

### Error Handling
- Graceful handling of API errors
- Proper cleanup of allocated memory
- Logging of operations and errors to TeamSpeak's log system

## Permissions

This plugin requires the following TeamSpeak permissions:
- **i_client_move_power**: Ability to move clients between channels
- The user must have sufficient permissions to move clients to the target channel

## Compatibility

- **TeamSpeak Client**: 3.x series
- **Operating Systems**: Windows 7+, Linux (most distributions)
- **Architecture**: 32-bit and 64-bit

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is provided as-is for educational and practical use. Feel free to modify and distribute according to your needs.

## Troubleshooting

### Plugin doesn't appear in TeamSpeak
- Ensure the plugin file is in the correct plugins directory
- Check that TeamSpeak was restarted after copying the plugin
- Verify the plugin is enabled in Settings > Plugins

### "MassMove here" option doesn't appear
- Make sure you're right-clicking on a channel (not a client)
- Ensure the plugin is properly loaded and enabled
- Check TeamSpeak logs for any error messages

### Move operation fails
- Verify you have sufficient permissions to move clients
- Check that the target channel allows the clients being moved
- Review TeamSpeak's server logs for permission-related errors

### Build errors
- Ensure all development tools are properly installed
- Check that the SDK include path is correct
- Verify you're using a compatible compiler version

## Support

For issues, questions, or feature requests, please:
1. Check the troubleshooting section above
2. Review TeamSpeak's plugin documentation
3. Create an issue with detailed information about your problem

---

**Happy TeamSpeaking! 🎙️** 