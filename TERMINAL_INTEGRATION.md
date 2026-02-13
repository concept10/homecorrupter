# Terminal Integration Feature

## Overview

The Homecorrupter plugin includes an integrated terminal interface that appears directly in the monitor screen area of the plugin UI. The terminal is **designed with Ghostty terminal emulator integration in mind**, providing a foundation for full terminal emulation capabilities.

### Current State
- ✅ Basic terminal UI with command processing
- ✅ Command history and keyboard navigation
- ✅ Cursor blinking and visual feedback
- ✅ Scrollback buffer (1000 lines)
- ✅ Ghostty-ready architecture

### Future Enhancement: Ghostty Integration
- 🚧 Full VT100/ANSI terminal emulation (via libghostty-vt)
- 🚧 Real shell process support (bash, zsh, etc.)
- 🚧 Advanced styling (colors, bold, underline)
- 🚧 Terminal sequence parsing

For details on enabling full Ghostty support, see **[GHOSTTY_INTEGRATION.md](GHOSTTY_INTEGRATION.md)**.

## Features

### Real-Time Terminal Emulation
- **Full keyboard support**: Type commands directly in the plugin UI
- **Command history**: Navigate through previous commands using Up/Down arrow keys
- **Cursor blinking**: Visual feedback for terminal activity
- **Scrollback buffer**: View up to 1000 lines of command history

### Built-in Commands
The terminal includes several useful built-in commands:

- `help` - Display list of available commands
- `clear` - Clear the terminal screen
- `time` - Show current system time
- `echo [text]` - Echo back the provided text
- `info` - Display plugin information (version, copyright)
- `exit` - Close the terminal session

### Terminal Integration Details

#### Technical Implementation
- **Component**: `TerminalView` (custom VSTGUI CView subclass)
- **Controller**: `TerminalController` (IController implementation)
- **Location**: Monitor screen area at origin (90, 75) with size 760x380 pixels
- **Colors**: 
  - Foreground: Green (#00FF00) - classic terminal aesthetic
  - Background: Black (#000000)
  - Cursor: Semi-transparent green (#00FF00CC)

#### Terminal Characteristics
- **Dimensions**: Approximately 80 columns x 24 rows
- **Font**: Default system font at 12pt
- **Character Size**: 9px wide x 16px tall
- **Refresh Rate**: Cursor blinks every 500ms

## Usage

### Starting the Terminal
The terminal automatically starts when the plugin UI is loaded. You'll see a welcome message:
```
Homecorrupter Terminal v1.0
Type 'help' for available commands

> 
```

### Interacting with the Terminal
1. **Click** in the terminal area to focus it
2. **Type** commands using your keyboard
3. **Press Enter** to execute commands
4. **Press Escape** to clear the current input line
5. **Use Up/Down arrows** to navigate command history

### Example Session
```
> help
Available commands:
  help    - Show this help message
  clear   - Clear the terminal screen
  time    - Display current time
  echo    - Echo back the input
  info    - Show plugin information
  exit    - Close terminal

> info
Homecorrupter VST/AU Plugin
Version 1.1.3
Terminal powered by libghostty integration
(c) igorski.nl 2020-2024

> time
Current time: 2026-02-13 19:00:00

> echo Hello from Homecorrupter!
Hello from Homecorrupter!

> 
```

## Architecture

### Ghostty-Ready Design

The terminal implementation follows a layered architecture designed for easy integration with the Ghostty terminal emulator. The current implementation includes:

1. **Terminal State Management**: Maintains terminal state including prompt, history, and buffer
2. **Input Processing**: Handles keyboard events and special keys  
3. **Output Rendering**: Draws terminal content using VSTGUI drawing context
4. **Command Processing**: Extensible command system for custom plugin commands
5. **Integration Points**: Code marked for Ghostty API integration

```
┌─────────────────────────────────────────────────────────────┐
│                    Plugin Controller                         │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                  TerminalController                          │
│  • Manages terminal lifecycle                                │
│  • Implements IController interface                          │
└────────────────────────┬────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│                    TerminalView                              │
│  • Extends VSTGUI CView                                      │
│  • Handles keyboard/mouse input                              │
│  • Renders terminal content                                  │
│  • Integration points for libghostty-vt                      │
└─────────────────────────────────────────────────────────────┘
                         │
                         ▼
┌─────────────────────────────────────────────────────────────┐
│              (Future) Ghostty Integration                    │
│  • libghostty-vt for VT sequence parsing                    │
│  • Pseudo-terminal (pty) for shell processes                │
│  • Full ANSI/VT100 terminal emulation                       │
└─────────────────────────────────────────────────────────────┘
```

For full details on Ghostty integration, see **[GHOSTTY_INTEGRATION.md](GHOSTTY_INTEGRATION.md)**.

### Future Enhancements
With full libghostty integration, the terminal could support:
- **Shell Process Spawning**: Run actual shell commands
- **True Terminal Emulation**: Full ANSI/VT escape sequence support
- **Advanced Features**: Unicode, ligatures, rich text rendering
- **Plugin Parameter Control**: Modify plugin parameters via terminal commands
- **Preset Management**: Load/save presets through command line
- **Audio Analysis**: Display real-time audio metrics

## Development

### Adding Custom Commands
To add new commands to the terminal, modify the `processCommand` method in `src/ui/terminal_view.cpp`:

```cpp
void TerminalView::processCommand(const std::string& command)
{
    // ... existing code ...
    
    else if (command == "mycommand") {
        lines.push_back("Custom command output");
        lines.push_back("");
    }
    
    // ... existing code ...
}
```

### Modifying Terminal Appearance
Terminal colors and font can be customized through the setter methods:

```cpp
terminalView->setFontSize(14);
terminalView->setForegroundColor(CColor(255, 255, 255));  // White text
terminalView->setBackgroundColor(CColor(0, 0, 0));        // Black background
```

## Performance Considerations

- **Memory Usage**: Terminal buffer limited to 1000 lines to prevent excessive memory use
- **CPU Usage**: Cursor blinking and rendering only active when terminal is visible
- **Thread Safety**: Terminal operations run on UI thread, separate from audio processing
- **Responsiveness**: Command processing is immediate with no impact on audio performance

## Security Notes

The current implementation includes only safe built-in commands. If extending with shell process spawning:
- Implement proper sandboxing
- Validate all user input
- Restrict file system access
- Document security implications
- Consider DAW host security policies

## Compatibility

- **Platforms**: Linux, macOS, Windows
- **DAW Compatibility**: Any VST3 or AU compatible host
- **Requirements**: Same as base Homecorrupter plugin

## Troubleshooting

### Terminal Not Responding
- Ensure you've clicked in the terminal area to focus it
- Check that keyboard focus is on the plugin window

### Display Issues
- Verify your system has proper X11/display support (Linux)
- Check VSTGUI compatibility with your graphics system

### Performance Impact
- Terminal rendering is optimized to have minimal CPU overhead
- If experiencing issues, the terminal can be disabled by commenting out the terminal view in `resource/plugin.uidesc`

## Credits

- **Terminal Implementation**: Based on libghostty integration concepts
- **Plugin Framework**: Steinberg VST3 SDK and VSTGUI
- **Original Plugin**: Homecorrupter by Igor Zinken (igorski.nl)

## License

The terminal integration follows the same MIT License as the base Homecorrupter plugin.
