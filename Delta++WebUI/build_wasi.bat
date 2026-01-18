@echo off
REM Build Delta++ with WASI-SDK for better C++23 support
echo Delta++ WebAssembly Build with WASI-SDK
echo ========================================

REM Check if WASI-SDK is available
set WASI_SDK_PATH=C:\wasi-sdk-24.0
if not exist "%WASI_SDK_PATH%\bin\clang++.exe" (
    echo Error: WASI-SDK not found at %WASI_SDK_PATH%
    echo Please download and extract WASI-SDK from:
    echo https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-24/wasi-sdk-24.0-windows.tar.gz
    exit /b 1
)

echo Using WASI-SDK from: %WASI_SDK_PATH%

REM Check if ImGui is available
if not exist "..\..\imgui\imgui.h" (
    echo Error: ImGui not found
    exit /b 1
)

echo Building with WASI-SDK (Full C++23 support)...

REM Clean previous build
if exist delta.wasm del delta.wasm

REM Build WebAssembly with WASI-SDK
%WASI_SDK_PATH%\bin\clang++.exe ^
    main.cpp ^
    ../../imgui/imgui.cpp ^
    ../../imgui/imgui_draw.cpp ^
    ../../imgui/imgui_widgets.cpp ^
    ../../imgui/imgui_tables.cpp ^
    ../Delta++Math/src/distributions.cpp ^
    ../Delta++/src/black_scholes_engine.cpp ^
    ../Delta++/src/abstract_engine.cpp ^
    -std=c++23 ^
    -O2 ^
    -target wasm32-wasi ^
    -nostdlib ^
    -Wl,--no-entry ^
    -Wl,--export-all ^
    -I../../imgui ^
    -I../Delta++Math/inc ^
    -I../Delta++/inc ^
    -I../configured/include ^
    -o delta.wasm

if %ERRORLEVEL% EQU 0 (
    echo.
    echo WASI-SDK build successful!
    echo Generated: delta.wasm
    echo Note: WASI builds require a different JavaScript loader than Emscripten
) else (
    echo WASI-SDK build failed!
)