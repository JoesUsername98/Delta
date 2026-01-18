@echo off
REM Build script for Delta++ WebAssembly version on Windows
REM This script sets up the environment and builds the WebAssembly version

echo Delta++ WebAssembly Build Script
echo =================================

REM Check if Emscripten is installed
where emcc >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo Error: Emscripten not found. Please install Emscripten SDK:
    echo 1. git clone https://github.com/emscripten-core/emsdk.git
    echo 2. cd emsdk
    echo 3. emsdk install latest
    echo 4. emsdk activate latest
    echo 5. emsdk_env.bat
    exit /b 1
)

echo Emscripten found
emcc --version | findstr "emcc"

REM Check if ImGui is available
set IMGUI_DIR=
if exist "..\\..\\imgui\\imgui.h" set IMGUI_DIR=..\\..\\imgui
if exist "..\\..\\..\\imgui\\imgui.h" set IMGUI_DIR=..\\..\\..\\imgui
if exist "..\\imgui\\imgui.h" set IMGUI_DIR=..\\imgui
if exist ".\\imgui\\imgui.h" set IMGUI_DIR=.\\imgui

if "%IMGUI_DIR%"=="" (
    echo Error: ImGui not found. Please clone ImGui:
    echo cd .. && git clone https://github.com/ocornut/imgui.git
    exit /b 1
)

echo ImGui found at: %IMGUI_DIR%

REM Build using make (requires make to be available)
echo Building with make...
make clean
make

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful!
    echo Generated files:
    echo   - delta.js
    echo   - delta.wasm
    echo.
    echo To test locally:
    echo   python -m http.server 8080
    echo   Then open: http://localhost:8080/index.html
    echo.
    echo To deploy to GitHub Pages:
    echo   Copy delta.js, delta.wasm, and index.html to your GitHub Pages repository
) else (
    echo Build failed!
    exit /b 1
)