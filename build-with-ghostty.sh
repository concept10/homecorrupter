#!/bin/bash

# Build Homecorrupter with Ghostty terminal emulator integration
# This script will:
# 1. Build libghostty-vt from the Ghostty source
# 2. Build the Homecorrupter plugin with Ghostty support enabled

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GHOSTTY_DIR="$SCRIPT_DIR/external/ghostty"

echo "================================================"
echo "Homecorrupter + Ghostty Integration Build"
echo "================================================"
echo ""

# Check if Zig is available
if ! command -v zig &> /dev/null; then
    echo "ERROR: Zig compiler not found!"
    echo ""
    echo "Ghostty requires Zig 0.15.2 or newer to build."
    echo "Download from: https://ziglang.org/download/"
    echo ""
    echo "After installing Zig, run this script again."
    exit 1
fi

ZIG_VERSION=$(zig version)
echo "Found Zig: $ZIG_VERSION"

# Check if Ghostty source exists
if [ ! -d "$GHOSTTY_DIR" ]; then
    echo ""
    echo "Ghostty source not found at: $GHOSTTY_DIR"
    echo ""
    echo "Cloning Ghostty repository..."
    mkdir -p "$SCRIPT_DIR/external"
    git clone --depth 1 https://github.com/ghostty-org/ghostty.git "$GHOSTTY_DIR"
    echo "Ghostty cloned successfully!"
fi

echo ""
echo "================================================"
echo "Step 1: Building libghostty-vt"
echo "================================================"
echo ""

cd "$GHOSTTY_DIR"

# Build libghostty-vt
echo "Running: zig build lib-vt -Doptimize=ReleaseSafe"
zig build lib-vt -Doptimize=ReleaseSafe

# Check if build succeeded
if [ -f "zig-out/lib/libghostty-vt.so" ] || [ -f "zig-out/lib/libghostty-vt.dylib" ] || [ -f "zig-out/lib/libghostty-vt.a" ]; then
    echo ""
    echo "✓ libghostty-vt built successfully!"
    echo "  Location: $GHOSTTY_DIR/zig-out/lib/"
    ls -lh zig-out/lib/libghostty-vt* || true
else
    echo ""
    echo "ERROR: libghostty-vt build failed!"
    exit 1
fi

cd "$SCRIPT_DIR"

echo ""
echo "================================================"
echo "Step 2: Building Homecorrupter Plugin"
echo "================================================"
echo ""

# Parse command line arguments
BUILD_TYPE="vst3"
while [ $# -gt 0 ]; do
    case "$1" in
        --type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [--type vst3|vst2|au]"
            exit 1
            ;;
    esac
done

echo "Build type: $BUILD_TYPE"
echo ""

# Run the standard build script with Ghostty enabled
if [ -f "build.sh" ]; then
    # Set environment variable to enable Ghostty support
    export USE_GHOSTTY=ON
    
    echo "Running: sh build.sh --type $BUILD_TYPE"
    sh build.sh --type "$BUILD_TYPE"
else
    echo "ERROR: build.sh not found!"
    exit 1
fi

echo ""
echo "================================================"
echo "Build Complete!"
echo "================================================"
echo ""
echo "The plugin has been built with Ghostty integration enabled."
echo ""
echo "Features enabled:"
echo "  ✓ libghostty-vt terminal sequence parsing"
echo "  ✓ Ghostty-powered terminal emulation"
echo "  ✓ VT100/ANSI escape sequence support"
echo ""
echo "Plugin location: ./build/VST3/homecorrupter.vst3"
echo ""
echo "To test the terminal, load the plugin in a DAW and click on"
echo "the monitor screen to interact with the terminal."
echo ""
