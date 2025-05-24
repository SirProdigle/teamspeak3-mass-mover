# TeamSpeak 3 MassMover Plugin

A lightweight TeamSpeak 3 plugin that adds a "MassMove here" context menu option to channels. With a single click, it moves all users from a channel and its entire subchannel hierarchy to the target channel.

## 🚀 Features

- **One-click mass move**: Right-click any channel and select "MassMove here"
- **Recursive channel traversal**: Automatically finds and moves users from all subchannels
- **Cross-platform**: Works on both Windows and Linux
- **Lightweight**: Minimal resource usage with efficient algorithms
- **Safe**: Proper error handling and memory management

## 🎯 How It Works

### User Perspective
1. Right-click on any channel in the TeamSpeak channel tree
2. Select "MassMove here" from the context menu
3. All users from that channel and its subchannels will be moved to the target channel

### Technical Details
The plugin uses a recursive depth-first search algorithm to:
1. Find all subchannels of the target channel
2. Collect all users from these channels
3. Move users in batches to the target channel
4. Handle the current user separately to avoid permission issues

## 🛠️ Building

### Prerequisites
- GCC compiler
- TeamSpeak 3 Client SDK

### Getting the SDK
1. Download the TeamSpeak 3 Client SDK from the [official website](https://www.teamspeak.com/en/downloads/#sdk)
2. Extract the SDK to the project root directory
3. The SDK directory should be named `ts3client-pluginsdk-26` (or similar)

Or use our automatic setup script:
```bash
chmod +x setup_sdk.sh
./setup_sdk.sh
```

### Quick Build
```bash
chmod +x build.sh
./build.sh
```

## 📦 Installation

### Linux
```bash
cp bin/linux/massmover.so ~/.ts3client/plugins/
```

### Windows
```cmd
copy bin\windows\massmover.dll %APPDATA%\TS3Client\plugins\
```

Then restart TeamSpeak and enable the plugin in Settings > Plugins.

## 🔒 Permissions

The plugin requires:
- `i_client_move_power`: Ability to move clients between channels
- Sufficient permissions to move clients to the target channel

## 🐛 Troubleshooting

If the plugin doesn't work as expected:
1. Check that you have sufficient permissions
2. Verify the plugin is enabled in Settings > Plugins
3. Check TeamSpeak's logs for any error messages

## 📝 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
