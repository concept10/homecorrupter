# Ghostty Terminal Emulator Integration

## Overview

This document describes how to integrate the [Ghostty terminal emulator](https://github.com/ghostty-org/ghostty) into the Homecorrupter plugin's monitor GUI. Ghostty is a fast, feature-rich, native terminal emulator that provides both a standalone application and embeddable libraries.

## Integration Architecture

The Homecorrupter plugin can be built with Ghostty support in two ways:

###  1. Basic Mode (Current Implementation)
- Custom terminal implementation with command processing
- No external dependencies beyond VST SDK
- Suitable for demonstration and basic functionality
- **Current Status**: ✅ Implemented

### 2. Ghostty-Powered Mode (Full Integration)
- Uses libghostty-vt for terminal sequence parsing
- Provides full ANSI/VT escape sequence support
- Real shell process integration via pseudo-terminal
- **Current Status**: 🚧 Architecture Ready, Requires Build Setup

## Why Ghostty?

Ghostty offers several advantages for terminal emulation:

- **Performance**: GPU-accelerated rendering, optimized for speed
- **Compliance**: Full VT100/xterm compatibility
- **Modern Features**: True color, ligatures, rich text
- **C API**: libghostty-vt provides a clean C interface
- **Cross-Platform**: Supports Linux, macOS, and Windows

## Ghostty Libraries

Ghostty provides two main libraries for embedding:

### libghostty (Full Embedding API)
- **Purpose**: Complete terminal emulator for embedding
- **Language**: Zig with C API
- **Platform**: Currently used by macOS Ghostty app
- **Status**: ⚠️ Work in Progress (not yet stable for general use)
- **Features**: Full terminal emulation, rendering, shell management

### libghostty-vt (Terminal Parsing Library)  
- **Purpose**: Terminal sequence parsing and encoding
- **Language**: Zig with C API
- **Platform**: Cross-platform (Linux, macOS, Windows, WebAssembly)
- **Status**: ✅ Available and Usable
- **Features**:
  - VT sequence parsing (ANSI escape codes)
  - Key event encoding (keyboard → terminal sequences)
  - OSC (Operating System Command) parsing
  - SGR (Select Graphic Rendition) parsing for styling
  - Paste safety validation

## Integration Strategy

Given the current state of Ghostty's libraries, the recommended integration approach is:

### Phase 1: libghostty-vt Integration (Recommended)

Use libghostty-vt for terminal sequence handling while maintaining our own:
- Terminal state management
- VSTGUI rendering integration
- Shell process management (pty)

**Benefits**:
- Leverage Ghostty's excellent VT parsing
- Remain cross-platform compatible
- No dependency on unstable APIs
- Full control over rendering and UI

**Components to Integrate**:
```c
#include <ghostty/vt.h>

// Key encoding - convert keyboard events to terminal sequences
GhosttyKeyEncoder encoder;
ghostty_key_encoder_new(NULL, &encoder);

// Parse VT sequences from shell output
// (Terminal emulator state management)

// OSC command parsing
GhosttyOscParser osc_parser;
ghostty_osc_new(NULL, &osc_parser);

// SGR styling parsing
GhosttySgrParser sgr_parser;
ghostty_sgr_new(NULL, &sgr_parser);
```

### Phase 2: Full libghostty Integration (Future)

Once libghostty stabilizes, we can integrate the complete terminal emulator:
- Use libghostty for full terminal state
- Render libghostty's frame buffer in VSTGUI
- Forward all input/output to libghostty

## Build Requirements

### Prerequisites

To build with Ghostty integration, you need:

1. **Zig Compiler** (version 0.15.2 or newer)
   - Download from: https://ziglang.org/download/
   - Required to build libghostty-vt

2. **Ghostty Source** (included as git submodule or external dependency)
   - Repository: https://github.com/ghostty-org/ghostty
   - Location: `external/ghostty/`

### Building libghostty-vt

```bash
cd external/ghostty

# Build libghostty-vt as a shared library
zig build lib-vt

# Build libghostty-vt as a static library (for linking into plugin)
zig build lib-vt -Doptimize=ReleaseSafe

# The build outputs will be in:
# - zig-out/lib/libghostty-vt.so (Linux)
# - zig-out/lib/libghostty-vt.dylib (macOS)
# - zig-out/lib/libghostty-vt.dll (Windows)
```

### CMake Integration

To enable Ghostty support in the plugin build, update `CMakeLists.txt`:

```cmake
# Optional Ghostty support
option(USE_GHOSTTY "Enable Ghostty terminal emulation" OFF)

if(USE_GHOSTTY)
    # Add Ghostty include directory
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/ghostty/include)
    
    # Find libghostty-vt library
    find_library(GHOSTTY_VT_LIB
        NAMES ghostty-vt
        PATHS ${CMAKE_CURRENT_SOURCE_DIR}/external/ghostty/zig-out/lib
        NO_DEFAULT_PATH
    )
    
    if(GHOSTTY_VT_LIB)
        message(STATUS "Found libghostty-vt: ${GHOSTTY_VT_LIB}")
        target_link_libraries(${target} PRIVATE ${GHOSTTY_VT_LIB})
        target_compile_definitions(${target} PRIVATE GHOSTTY_ENABLED=1)
    else()
        message(WARNING "libghostty-vt not found. Build it first with: cd external/ghostty && zig build lib-vt")
    endif()
endif()
```

### Building the Plugin with Ghostty

```bash
# Build libghostty-vt first
cd external/ghostty
zig build lib-vt -Doptimize=ReleaseSafe
cd ../..

# Build plugin with Ghostty support
mkdir -p build && cd build
cmake -DUSE_GHOSTTY=ON ..
cmake --build .
```

## Code Integration Points

### 1. Terminal View Enhancement

The `TerminalView` class (`src/ui/terminal_view.cpp`) needs enhancement to use Ghostty APIs:

```cpp
#ifdef GHOSTTY_ENABLED
#include <ghostty/vt.h>
#endif

class TerminalView : public CView {
private:
    #ifdef GHOSTTY_ENABLED
    GhosttyKeyEncoder* keyEncoder;
    // Additional Ghostty state
    #endif
    
public:
    void handleKeyEvent(const VstKeyCode& keyCode) {
        #ifdef GHOSTTY_ENABLED
        // Use Ghostty key encoder
        GhosttyKeyEvent event;
        ghostty_key_event_new(NULL, &event);
        // ... configure event ...
        
        char buffer[128];
        size_t written = 0;
        ghostty_key_encoder_encode(keyEncoder, event, buffer, sizeof(buffer), &written);
        
        // Send to shell process
        sendToShell(buffer, written);
        
        ghostty_key_event_free(event);
        #else
        // Fallback to simple character handling
        #endif
    }
};
```

### 2. VT Sequence Parsing

When receiving output from a shell process:

```cpp
void TerminalView::processShellOutput(const char* data, size_t len) {
    #ifdef GHOSTTY_ENABLED
    // Parse VT sequences with Ghostty
    for (size_t i = 0; i < len; i++) {
        // Feed to VT parser
        // Update terminal state based on parsed sequences
        // Handle colors, cursor movement, etc.
    }
    #else
    // Simple line-based output
    #endif
}
```

### 3. Shell Process Integration (PTY)

For real terminal emulation, integrate a pseudo-terminal:

```cpp
#include <pty.h>  // Unix PTY support

class PtyProcess {
public:
    int masterFd;
    pid_t childPid;
    
    bool spawn(const char* shell) {
        struct winsize ws = {
            .ws_row = 24,
            .ws_col = 80,
            .ws_xpixel = 0,
            .ws_ypixel = 0,
        };
        
        childPid = forkpty(&masterFd, NULL, NULL, &ws);
        if (childPid == 0) {
            // Child process - exec shell
            execl(shell, shell, NULL);
            exit(1);
        }
        return childPid > 0;
    }
    
    void write(const char* data, size_t len) {
        ::write(masterFd, data, len);
    }
    
    ssize_t read(char* buffer, size_t len) {
        return ::read(masterFd, buffer, len);
    }
};
```

## Terminal Rendering

The terminal view needs to render the parsed terminal state:

### Color Support

```cpp
struct TerminalCell {
    char character;
    CColor foreground;
    CColor background;
    bool bold;
    bool italic;
    bool underline;
};

void TerminalView::drawTerminalContent(CDrawContext* context) {
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            TerminalCell& cell = buffer[y * cols + x];
            
            // Set color from cell attributes
            context->setFontColor(cell.foreground);
            
            // Apply styles
            if (cell.bold) {
                // Use bold font
            }
            
            // Draw character
            CPoint pos(startX + x * charWidth, startY + y * charHeight);
            context->drawString(&cell.character, 1, pos);
        }
    }
}
```

## Testing with Ghostty

### Unit Testing

Test individual Ghostty API components:

```cpp
void testKeyEncoding() {
    GhosttyKeyEncoder encoder;
    ghostty_key_encoder_new(NULL, &encoder);
    
    GhosttyKeyEvent event;
    ghostty_key_event_new(NULL, &event);
    ghostty_key_event_set_key(event, GHOSTTY_KEY_A);
    
    char buffer[128];
    size_t written = 0;
    int result = ghostty_key_encoder_encode(encoder, event, buffer, sizeof(buffer), &written);
    
    assert(result == GHOSTTY_SUCCESS);
    assert(written > 0);
    
    ghostty_key_event_free(event);
    ghostty_key_encoder_free(encoder);
}
```

### Integration Testing

Test the terminal with real shell commands:

```bash
# Load plugin in DAW
# Click on terminal
# Type: ls -la
# Expect: Directory listing with colors
# Type: vim test.txt
# Expect: Vim editor loads and responds to keys
```

## Performance Considerations

### Memory Usage
- libghostty-vt: ~500KB-1MB
- Terminal buffer: Configurable (default: 1000 lines scrollback)
- Shell process: OS-dependent

### CPU Usage
- VT parsing: Negligible (<0.1% typically)
- Shell I/O: Minimal (async reads)
- Rendering: Only when terminal content changes

### Thread Safety
- Terminal I/O on dedicated thread
- VT parsing on same thread as I/O
- Rendering on UI thread
- Synchronize state updates between threads

## Security Considerations

### Shell Execution
- Validate shell path before execution
- Run with limited permissions
- Sanitize environment variables
- Limit accessible filesystem paths

### Input Validation
- Use Ghostty's paste safety checks
- Validate all input before sending to shell
- Prevent injection attacks

```cpp
#include <ghostty/vt/paste.h>

bool validatePaste(const char* data, size_t len) {
    return ghostty_paste_safe_check(data, len) == GHOSTTY_SUCCESS;
}
```

## Cross-Platform Notes

### Linux
- Use `libutil` for PTY support
- OpenGL for rendering (optional GPU acceleration)
- X11/Wayland for window management

### macOS
- Use `util.h` for PTY support
- Metal for rendering (if available)
- Native Cocoa for window management

### Windows
- Use ConPTY API for pseudo-terminal
- Direct3D for rendering
- Native Win32 for window management

## Troubleshooting

### Build Issues

**Problem**: "zig: command not found"
- **Solution**: Install Zig compiler from https://ziglang.org/download/

**Problem**: "libghostty-vt.so not found"
- **Solution**: Build libghostty-vt first: `cd external/ghostty && zig build lib-vt`

**Problem**: Zig version mismatch
- **Solution**: Check `external/ghostty/build.zig.zon` for required version

### Runtime Issues

**Problem**: Terminal not responding
- **Solution**: Check shell process is running: `ps aux | grep bash`

**Problem**: Garbled output
- **Solution**: Verify VT sequence parsing is enabled and working

**Problem**: Colors not showing
- **Solution**: Check SGR parsing is implemented and terminal supports colors

## Examples and References

### Ghostty Examples
- `external/ghostty/example/c-vt/` - OSC parser example
- `external/ghostty/example/c-vt-key-encode/` - Key encoding example
- `external/ghostty/example/c-vt-sgr/` - SGR parsing example

### API Documentation
- Ghostty Documentation: https://ghostty.org/docs
- libghostty-vt Headers: `external/ghostty/include/ghostty/vt.h`

### Similar Projects
- Alacritty - Rust-based terminal emulator
- Kitty - GPU-accelerated terminal
- iTerm2 - macOS terminal (uses libghostty concepts)

## Future Enhancements

### Short-term
- [ ] Implement libghostty-vt key encoding
- [ ] Add VT sequence parsing for colors
- [ ] Implement scrollback buffer
- [ ] Add clipboard support

### Long-term
- [ ] Full libghostty integration (when stable)
- [ ] GPU-accelerated rendering
- [ ] Ligature support
- [ ] Split panes/tabs
- [ ] Theme customization

## Contributing

If you're working on Ghostty integration:

1. Test on all target platforms (Linux, macOS, Windows)
2. Ensure backward compatibility (plugin works without Ghostty)
3. Document any API changes
4. Add unit tests for Ghostty-specific code
5. Update this document with your findings

## License

Ghostty is licensed under the MIT License. See `external/ghostty/LICENSE` for details.

The Homecorrupter plugin integration follows the same MIT License as the base plugin.

## References

- Ghostty Repository: https://github.com/ghostty-org/ghostty
- Ghostty Website: https://ghostty.org
- VT100 Reference: https://vt100.net/docs/vt100-ug/
- ECMA-48 Standard: https://ecma-international.org/publications-and-standards/standards/ecma-48/
- Zig Language: https://ziglang.org

---

**Last Updated**: February 2026  
**Status**: Architecture Documentation Complete, Implementation Ready
