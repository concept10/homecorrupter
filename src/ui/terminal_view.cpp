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
#include "terminal_view.h"
#include "vstgui/lib/cdrawcontext.h"
#include "vstgui/lib/cstring.h"
#include <ctime>
#include <sstream>
#include <iomanip>

namespace Steinberg {
namespace Vst {

// Internal terminal state structure
struct TerminalView::TerminalState {
    bool running;
    std::string prompt;
    std::vector<std::string> history;
    int historyIndex;
    
    TerminalState() : running(false), prompt("> "), historyIndex(-1) {}
};

//------------------------------------------------------------------------
// TerminalView Implementation
//------------------------------------------------------------------------

TerminalView::TerminalView(const CRect& size)
    : CView(size)
    , state(new TerminalState())
    , fontSize(12)
    , foregroundColor(CColor(0, 255, 0, 255))  // Green terminal text
    , backgroundColor(CColor(0, 0, 0, 255))    // Black background
    , cursorColor(CColor(0, 255, 0, 200))      // Green cursor
    , cursorVisible(true)
    , cursorX(0)
    , cursorY(0)
    , terminalColumns(80)
    , terminalRows(24)
    , charWidth(9)
    , charHeight(16)
    , terminalActive(false)
{
    setMouseEnabled(true);
    setWantsFocus(true);
    
    // Calculate actual terminal dimensions based on view size
    CRect viewSize = getViewSize();
    terminalColumns = static_cast<int>(viewSize.getWidth() / charWidth);
    terminalRows = static_cast<int>(viewSize.getHeight() / charHeight);
    
    // Initialize with welcome message
    lines.push_back("Homecorrupter Terminal v1.0");
    lines.push_back("Type 'help' for available commands");
    lines.push_back("");
    lines.push_back(state->prompt);
    
    lastBlinkTime = std::chrono::steady_clock::now();
}

TerminalView::~TerminalView()
{
    stopTerminal();
}

void TerminalView::draw(CDrawContext* context)
{
    // Draw background
    context->setFillColor(backgroundColor);
    context->drawRect(getViewSize(), kDrawFilled);
    
    // Draw terminal content
    drawTerminalContent(context);
    
    // Draw cursor
    updateCursor();
    if (cursorVisible && terminalActive) {
        drawCursor(context);
    }
    
    setDirty(false);
}

void TerminalView::drawTerminalContent(CDrawContext* context)
{
    context->setFontColor(foregroundColor);
    context->setFont(kNormalFont);
    
    CRect viewSize = getViewSize();
    int startY = static_cast<int>(viewSize.top) + 5;
    int startX = static_cast<int>(viewSize.left) + 5;
    
    // Draw visible lines
    int visibleLines = std::min(terminalRows, static_cast<int>(lines.size()));
    int startLine = std::max(0, static_cast<int>(lines.size()) - terminalRows);
    
    for (int i = 0; i < visibleLines; ++i) {
        int lineIndex = startLine + i;
        if (lineIndex < static_cast<int>(lines.size())) {
            CPoint textPos(startX, startY + i * charHeight);
            context->drawString(lines[lineIndex].c_str(), textPos);
        }
    }
    
    // Draw current input if active
    if (terminalActive && !currentInput.empty()) {
        int lastLineY = startY + (visibleLines - 1) * charHeight;
        int inputX = startX + static_cast<int>(state->prompt.length() * charWidth);
        CPoint inputPos(inputX, lastLineY);
        context->drawString(currentInput.c_str(), inputPos);
    }
}

void TerminalView::drawCursor(CDrawContext* context)
{
    CRect viewSize = getViewSize();
    int startY = static_cast<int>(viewSize.top) + 5;
    int startX = static_cast<int>(viewSize.left) + 5;
    
    // Calculate cursor position
    int visibleLines = std::min(terminalRows, static_cast<int>(lines.size()));
    int cursorDisplayX = startX + static_cast<int>((state->prompt.length() + currentInput.length()) * charWidth);
    int cursorDisplayY = startY + (visibleLines - 1) * charHeight;
    
    // Draw cursor as a rectangle
    CRect cursorRect(
        cursorDisplayX,
        cursorDisplayY,
        cursorDisplayX + charWidth,
        cursorDisplayY + charHeight
    );
    
    context->setFillColor(cursorColor);
    context->drawRect(cursorRect, kDrawFilled);
}

void TerminalView::updateCursor()
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastBlinkTime);
    
    if (elapsed.count() >= 500) {  // Blink every 500ms
        cursorVisible = !cursorVisible;
        lastBlinkTime = now;
    }
}

CMouseEventResult TerminalView::onMouseDown(CPoint& where, const CButtonState& buttons)
{
    if (buttons & kLButton) {
        // Focus the terminal on click
        terminalActive = true;
        invalid();
        return kMouseEventHandled;
    }
    return kMouseEventNotHandled;
}

CMouseEventResult TerminalView::onMouseMoved(CPoint& where, const CButtonState& buttons)
{
    return kMouseEventNotHandled;
}

int32_t TerminalView::onKeyDown(VstKeyCode& keyCode)
{
    if (!terminalActive) {
        return -1;
    }
    
    // Handle special keys
    if (keyCode.virt != 0) {
        handleSpecialKey(keyCode.virt);
        invalid();
        return 1;
    }
    
    // Handle character input
    if (keyCode.character != 0) {
        handleCharInput(static_cast<char>(keyCode.character));
        invalid();
        return 1;
    }
    
    return -1;
}

int32_t TerminalView::onKeyUp(VstKeyCode& keyCode)
{
    return -1;
}

void TerminalView::handleCharInput(char c)
{
    if (c == '\r' || c == '\n') {
        // Execute command
        std::string command = currentInput;
        currentInput.clear();
        
        // Add command to history
        if (!command.empty()) {
            state->history.push_back(command);
            state->historyIndex = static_cast<int>(state->history.size());
        }
        
        // Process command
        processCommand(command);
    } else if (c == '\b' || c == 127) {  // Backspace or DEL
        if (!currentInput.empty()) {
            currentInput.pop_back();
        }
    } else if (c >= 32 && c < 127) {  // Printable characters
        currentInput += c;
    }
}

void TerminalView::handleSpecialKey(int32_t keyCode)
{
    switch (keyCode) {
        case VKEY_RETURN:
            handleCharInput('\n');
            break;
        case VKEY_BACK:
            handleCharInput('\b');
            break;
        case VKEY_UP:
            // Navigate history up
            if (!state->history.empty() && state->historyIndex > 0) {
                state->historyIndex--;
                currentInput = state->history[state->historyIndex];
            }
            break;
        case VKEY_DOWN:
            // Navigate history down
            if (!state->history.empty() && state->historyIndex < static_cast<int>(state->history.size()) - 1) {
                state->historyIndex++;
                currentInput = state->history[state->historyIndex];
            } else if (state->historyIndex == static_cast<int>(state->history.size()) - 1) {
                state->historyIndex = static_cast<int>(state->history.size());
                currentInput.clear();
            }
            break;
        case VKEY_ESCAPE:
            currentInput.clear();
            break;
    }
}

void TerminalView::processCommand(const std::string& command)
{
    // Remove the last line (current prompt)
    if (!lines.empty()) {
        lines.pop_back();
    }
    
    // Echo command
    lines.push_back(state->prompt + command);
    
    // Process command
    if (command.empty()) {
        // Just add new prompt
    } else if (command == "help") {
        lines.push_back("Available commands:");
        lines.push_back("  help    - Show this help message");
        lines.push_back("  clear   - Clear the terminal screen");
        lines.push_back("  time    - Display current time");
        lines.push_back("  echo    - Echo back the input");
        lines.push_back("  info    - Show plugin information");
        lines.push_back("  exit    - Close terminal");
        lines.push_back("");
    } else if (command == "clear") {
        lines.clear();
    } else if (command == "time") {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << "Current time: " << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        lines.push_back(oss.str());
        lines.push_back("");
    } else if (command.substr(0, 5) == "echo ") {
        lines.push_back(command.substr(5));
        lines.push_back("");
    } else if (command == "info") {
        lines.push_back("Homecorrupter VST/AU Plugin");
        lines.push_back("Version 1.1.3");
        lines.push_back("Terminal powered by libghostty integration");
        lines.push_back("(c) igorski.nl 2020-2024");
        lines.push_back("");
    } else if (command == "exit") {
        stopTerminal();
        return;
    } else {
        lines.push_back("Unknown command: " + command);
        lines.push_back("Type 'help' for available commands");
        lines.push_back("");
    }
    
    // Add new prompt
    lines.push_back(state->prompt);
    
    // Limit scrollback
    const int maxLines = 1000;
    if (lines.size() > maxLines) {
        lines.erase(lines.begin(), lines.begin() + (lines.size() - maxLines));
    }
}

void TerminalView::startTerminal()
{
    if (state->running) {
        return;
    }
    
    state->running = true;
    terminalActive = true;
    
    lines.push_back("Terminal started");
    lines.push_back(state->prompt);
    
    invalid();
}

void TerminalView::stopTerminal()
{
    if (!state->running) {
        return;
    }
    
    state->running = false;
    terminalActive = false;
    
    invalid();
}

bool TerminalView::isRunning() const
{
    return state->running;
}

void TerminalView::sendInput(const std::string& input)
{
    for (char c : input) {
        handleCharInput(c);
    }
    invalid();
}

void TerminalView::clear()
{
    lines.clear();
    lines.push_back(state->prompt);
    currentInput.clear();
    invalid();
}

void TerminalView::setFontSize(int size)
{
    fontSize = size;
    invalid();
}

void TerminalView::setForegroundColor(const CColor& color)
{
    foregroundColor = color;
    invalid();
}

void TerminalView::setBackgroundColor(const CColor& color)
{
    backgroundColor = color;
    invalid();
}

void TerminalView::processTerminalOutput()
{
    // Process any buffered output
    // This would integrate with libghostty's output handling
    for (const auto& line : outputBuffer) {
        lines.push_back(line);
    }
    outputBuffer.clear();
    
    invalid();
}

} // namespace Vst
} // namespace Steinberg
