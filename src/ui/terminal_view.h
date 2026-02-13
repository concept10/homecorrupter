/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2024 Igor Zinken - https://www.igorski.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef __TERMINAL_VIEW_HEADER__
#define __TERMINAL_VIEW_HEADER__

#include "vstgui/vstgui.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>

namespace Steinberg {
namespace Vst {

using namespace VSTGUI;

/**
 * TerminalView provides a terminal interface within the plugin UI
 * 
 * ARCHITECTURE: Designed for integration with Ghostty terminal emulator
 * - Current: Basic command processing implementation
 * - Future: Full Ghostty integration via libghostty-vt
 * 
 * See GHOSTTY_INTEGRATION.md for details on enabling full Ghostty support.
 * 
 * When built with -DGHOSTTY_ENABLED:
 * - Uses libghostty-vt for VT sequence parsing
 * - Supports full ANSI/VT escape sequences
 * - Real shell process integration via pseudo-terminal
 * - GPU-accelerated rendering (optional)
 */
class TerminalView : public CView
{
public:
    TerminalView(const CRect& size);
    ~TerminalView();
    
    // CView overrides
    void draw(CDrawContext* context) override;
    CMouseEventResult onMouseDown(CPoint& where, const CButtonState& buttons) override;
    CMouseEventResult onMouseMoved(CPoint& where, const CButtonState& buttons) override;
    int32_t onKeyDown(VstKeyCode& keyCode) override;
    int32_t onKeyUp(VstKeyCode& keyCode) override;
    
    // Terminal control methods
    void startTerminal();
    void stopTerminal();
    bool isRunning() const;
    void sendInput(const std::string& input);
    void clear();
    
    // Terminal configuration
    void setFontSize(int size);
    void setForegroundColor(const CColor& color);
    void setBackgroundColor(const CColor& color);
    
    // Disable copy constructor and assignment (unique_ptr doesn't support copying)
    TerminalView(const TerminalView&) = delete;
    TerminalView& operator=(const TerminalView&) = delete;
    
private:
    // Terminal state
    struct TerminalState;
    std::unique_ptr<TerminalState> state;
    
    // Rendering helpers
    void drawCursor(CDrawContext* context);
    void drawTerminalContent(CDrawContext* context);
    void processTerminalOutput();
    void updateCursor();
    
    // Input handling
    void handleCharInput(char c);
    void handleSpecialKey(int32_t keyCode);
    void processCommand(const std::string& command);
    
    // Terminal configuration
    int fontSize;
    CColor foregroundColor;
    CColor backgroundColor;
    CColor cursorColor;
    
    // Cursor state
    bool cursorVisible;
    int cursorX;
    int cursorY;
    std::chrono::steady_clock::time_point lastBlinkTime;
    
    // Terminal dimensions
    int terminalColumns;
    int terminalRows;
    int charWidth;
    int charHeight;
    
    // Terminal buffer
    std::vector<std::string> lines;
    std::string currentInput;
    bool terminalActive;
    
    // Thread-safe output buffer
    std::vector<std::string> outputBuffer;
};

} // namespace Vst
} // namespace Steinberg

#endif
