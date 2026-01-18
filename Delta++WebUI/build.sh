#!/bin/bash

# Build script for Delta++ WebAssembly version
# This script sets up the environment and builds the WebAssembly version

set -e

echo "Delta++ WebAssembly Build Script"
echo "================================="

# Check if Emscripten is installed
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten not found. Please install Emscripten SDK:"
    echo "1. git clone https://github.com/emscripten-core/emsdk.git"
    echo "2. cd emsdk"  
    echo "3. ./emsdk install latest"
    echo "4. ./emsdk activate latest"
    echo "5. source ./emsdk_env.sh"
    exit 1
fi

echo "Emscripten found: $(emcc --version | head -n 1)"

# Check if ImGui is available
IMGUI_PATHS=(
    "../../imgui"
    "../../../imgui"
    "../imgui"
    "./imgui"
)

IMGUI_DIR=""
for path in "${IMGUI_PATHS[@]}"; do
    if [ -f "$path/imgui.h" ]; then
        IMGUI_DIR="$path"
        break
    fi
done

if [ -z "$IMGUI_DIR" ]; then
    echo "Error: ImGui not found. Please clone ImGui:"
    echo "cd .. && git clone https://github.com/ocornut/imgui.git"
    exit 1
fi

echo "ImGui found at: $IMGUI_DIR"

# Build using Makefile
echo "Building with Makefile..."
make clean
make

if [ $? -eq 0 ]; then
    echo ""
    echo "Build successful!"
    echo "Generated files:"
    echo "  - delta.js"
    echo "  - delta.wasm"
    echo ""
    echo "To test locally:"
    echo "  python3 -m http.server 8080"
    echo "  Then open: http://localhost:8080/index.html"
    echo ""
    echo "To deploy to GitHub Pages:"
    echo "  Copy delta.js, delta.wasm, and index.html to your GitHub Pages repository"
else
    echo "Build failed!"
    exit 1
fi