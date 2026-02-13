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
#include "terminal_controller.h"
#include "vstgui/uidescription/uiattributes.h"

namespace Steinberg {
namespace Vst {

//------------------------------------------------------------------------
// TerminalController Implementation
//------------------------------------------------------------------------

TerminalController::TerminalController()
    : terminalView(nullptr)
{
}

TerminalController::~TerminalController()
{
    if (terminalView) {
        terminalView->unregisterViewListener(this);
        terminalView = nullptr;
    }
}

CView* TerminalController::createView(const UIAttributes& attributes, 
                                      const IUIDescription* description)
{
    // Check if this is a terminal view
    const std::string* customViewName = attributes.getAttributeValue("custom-view-name");
    if (customViewName && *customViewName == "TerminalView") {
        // Create the terminal view
        CPoint origin, size;
        attributes.getPointAttribute("origin", origin);
        attributes.getPointAttribute("size", size);
        
        CRect rect(origin, size);
        
        terminalView = new TerminalView(rect);
        if (terminalView) {
            terminalView->registerViewListener(this);
            terminalView->startTerminal();
        }
        return terminalView;
    }
    
    return nullptr;
}

CView* TerminalController::verifyView(CView* view, const UIAttributes& attributes,
                                      const IUIDescription* description)
{
    // Check if this is our terminal view
    TerminalView* tView = dynamic_cast<TerminalView*>(view);
    if (tView) {
        if (!terminalView) {
            terminalView = tView;
            terminalView->registerViewListener(this);
            terminalView->startTerminal();
        }
    }
    
    return view;
}

void TerminalController::viewWillDelete(CView* view)
{
    if (view == terminalView) {
        terminalView = nullptr;
    }
}

void TerminalController::startTerminal()
{
    if (terminalView) {
        terminalView->startTerminal();
    }
}

void TerminalController::stopTerminal()
{
    if (terminalView) {
        terminalView->stopTerminal();
    }
}

bool TerminalController::isTerminalRunning() const
{
    return terminalView && terminalView->isRunning();
}

} // namespace Vst
} // namespace Steinberg
