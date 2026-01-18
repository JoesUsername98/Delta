@echo off
echo Building simple WebAssembly test...

set EMSDK_PATH=C:\repos\emsdk
set EMSCRIPTEN_PATH=%EMSDK_PATH%\upstream\emscripten
set NODE_PATH=%EMSDK_PATH%\node\22.16.0_64bit\bin
set PYTHON_PATH=%EMSDK_PATH%\python\3.13.3_64bit

REM Clean previous build
if exist test.js del test.js
if exist test.wasm del test.wasm

echo Building simple test with minimal dependencies...

%PYTHON_PATH%\python.exe %EMSCRIPTEN_PATH%\emcc.py ^
    test_simple.cpp ^
    ../../imgui/backends/imgui_impl_glfw.cpp ^
    ../../imgui/backends/imgui_impl_opengl3.cpp ^
    ../../imgui/imgui.cpp ^
    ../../imgui/imgui_draw.cpp ^
    ../../imgui/imgui_widgets.cpp ^
    ../../imgui/imgui_tables.cpp ^
    -std=c++17 ^
    -o test.js ^
    -s USE_WEBGL2=1 ^
    -s USE_GLFW=3 ^
    -s FULL_ES3=1 ^
    -s WASM=1 ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s NO_EXIT_RUNTIME=1 ^
    -O2 ^
    -I../../imgui ^
    -I../../imgui/backends

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Simple test build successful!
    echo Generated: test.js and test.wasm
    echo.
) else (
    echo Simple test build failed!
)