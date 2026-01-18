@echo off
REM Build script for Delta++ WebAssembly version on Windows
REM Works around PATH issues by using full paths to Emscripten tools

echo Delta++ WebAssembly Build Script (Windows)
echo ==========================================

REM Check if emsdk is available
if not exist "C:\repos\emsdk\upstream\emscripten\emcc.py" (
    echo Error: Emscripten not found at expected location
    echo Please ensure you've run:
    echo   cd C:\repos\emsdk
    echo   python emsdk.py install latest
    echo   python emsdk.py activate latest
    exit /b 1
)

echo Using Emscripten from: C:\repos\emsdk\upstream\emscripten\

REM Check if ImGui is available
if not exist "..\..\imgui\imgui.h" (
    echo Error: ImGui not found. Please run:
    echo   cd C:\repos
    echo   git clone https://github.com/ocornut/imgui.git --depth 1
    exit /b 1
)

echo ImGui found at: ..\..\imgui\

REM Set up paths
set EMSDK_PATH=C:\repos\emsdk
set EMSCRIPTEN_PATH=%EMSDK_PATH%\upstream\emscripten
set NODE_PATH=%EMSDK_PATH%\node\22.16.0_64bit\bin
set PYTHON_PATH=%EMSDK_PATH%\python\3.13.3_64bit

REM Add to PATH for this session
set PATH=%EMSCRIPTEN_PATH%;%NODE_PATH%;%PYTHON_PATH%;%PATH%

REM Set Emscripten environment variables
set EMSDK=%EMSDK_PATH%
set EMSDK_NODE=%NODE_PATH%\node.exe
set EMSDK_PYTHON=%PYTHON_PATH%\python.exe

echo Building WebAssembly with full paths...

REM Use python to run emcc directly
%PYTHON_PATH%\python.exe %EMSCRIPTEN_PATH%\emcc.py --version

if %ERRORLEVEL% NEQ 0 (
    echo Error: emcc test failed
    exit /b 1
)

echo emcc is working! Building Delta++WebUI...

REM Clean previous build
if exist delta.js del delta.js
if exist delta.wasm del delta.wasm

REM Build using emcc directly
%PYTHON_PATH%\python.exe %EMSCRIPTEN_PATH%\emcc.py ^
    main.cpp ^
    ../../imgui/backends/imgui_impl_glfw.cpp ^
    ../../imgui/backends/imgui_impl_opengl3.cpp ^
    ../../imgui/imgui.cpp ^
    ../../imgui/imgui_draw.cpp ^
    ../../imgui/imgui_demo.cpp ^
    ../../imgui/imgui_widgets.cpp ^
    ../../imgui/imgui_tables.cpp ^
    ../Delta++Math/src/distributions.cpp ^
    ../Delta++/src/black_scholes_engine.cpp ^
    ../Delta++/src/abstract_engine.cpp ^
    -std=c++23 ^
    -fexperimental-library ^
    -D__cpp_lib_expected=202211L ^
    -o delta.js ^
    -s USE_WEBGL2=1 ^
    -s USE_GLFW=3 ^
    -s FULL_ES3=1 ^
    -s WASM=1 ^
    -s ALLOW_MEMORY_GROWTH=1 ^
    -s NO_EXIT_RUNTIME=1 ^
    -s ASSERTIONS=1 ^
    -O2 ^
    -I../../imgui ^
    -I../../imgui/backends ^
    -I../Delta++Math/inc ^
    -I../Delta++/inc ^
    -I../configured/include ^
    -I.

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
) else (
    echo Build failed!
    exit /b 1
)