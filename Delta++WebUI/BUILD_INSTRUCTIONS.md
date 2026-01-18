# Delta++ WebAssembly Build Instructions

## Prerequisites

1. **Install Emscripten SDK**:
   ```powershell
   # Clone Emscripten SDK
   git clone https://github.com/emscripten-core/emsdk.git C:\repos\emsdk
   cd C:\repos\emsdk
   
   # Install and activate latest stable version
   .\emsdk install latest
   .\emsdk activate latest
   ```

2. **Install Ninja Build System**:
   ```powershell
   winget install ninja-build.ninja
   ```
   Note: You may need to restart your terminal after installation.

3. **Clone ImGui**:
   ```powershell
   cd C:\repos\Delta
   git clone https://github.com/ocornut/imgui.git --depth 1
   ```
## Windows Build Process - WORKING METHOD

1. **Open PowerShell and navigate to the project**:
   ```powershell
   cd C:\repos\Delta\Delta++WebUI
   ```

2. **Create and enter build directory** (builds to build/Delta++WebUI):
   ```powershell
   mkdir ..\build\Delta++WebUI -Force
   cd ..\build\Delta++WebUI
   ```

3. **Set up Ninja in PATH** (if needed):
   ```powershell
   $env:PATH += ";$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"
   ```

4. **Configure with CMake using direct Python paths**:
   ```powershell
   C:\repos\emsdk\python\3.13.3_64bit\python.exe C:\repos\emsdk\upstream\emscripten\emcmake.py cmake ..\..\Delta++WebUI -G "Ninja"
   ```

5. **Build the WebAssembly application**:
   ```powershell
   ninja DeltaWebUI
   ```
   
   **Note**: If you see "ninja: no work to do.", your build is already complete!

## Build Output

After successful build, you will have:
- `DeltaWebUI.js` - The JavaScript loader
- `DeltaWebUI.wasm` - The WebAssembly binary  
- `index.html` - The web page to run the application

## Testing Locally

1. **Navigate to the build output directory**:
   ```powershell
   cd C:\repos\Delta\build\Delta++WebUI
   ```

2. **Start a local web server**:
   ```powershell
   python -m http.server 8080
   ```

3. **Open your browser and navigate to**:
   ```
   http://localhost:8080/index.html
   ```

## Alternative: Simple Test Build

To build a simple ImGui test instead of the full Delta++ application:
```powershell
ninja DeltaWebUITest
```

## Clean Build

To clean and rebuild:
```powershell
ninja clean
ninja DeltaWebUI
```

## Troubleshooting

### "ninja: no work to do."
This message means **your build is complete and successful**! Check for these files in your build directory:
- `DeltaWebUI.js` 
- `DeltaWebUI.wasm`
- `index.html`

If these files exist, proceed to the testing step.

### "make: not recognized"
The CMake output shows `make` commands, but you're using Ninja. Always use:
- `ninja DeltaWebUI` (not `make DeltaWebUI`)
- `python -m http.server 8080` (not `make serve`)

### "ninja not recognized"
If ninja is not found in your PATH, use the full path:
```powershell
$env:PATH += ";$env:LOCALAPPDATA\Microsoft\WinGet\Packages\Ninja-build.Ninja_Microsoft.Winget.Source_8wekyb3d8bbwe"
```

### "ImGui not found"
Ensure ImGui is cloned in the correct location:
```powershell
cd C:\repos\Delta
git clone https://github.com/ocornut/imgui.git --depth 1
```

### CMake Cache Issues
If switching generators or having configuration issues:
```powershell
Remove-Item -Recurse CMakeCache.txt, CMakeFiles
```

## What This Builds

The CMake build system creates a complete WebAssembly application:

1. **ImGui Library** - Compiles Dear ImGui with WebGL2 backend
2. **Delta++Math** - The core mathematical libraries (distributions, etc.)
3. **Delta++Core** - Black-Scholes engine and abstract pricing engine
4. **Main Application** - Complete derivatives pricing interface
5. **Web Assets** - HTML file configured for WebAssembly loading

The resulting application provides a full Delta++ derivatives pricing interface that runs in any modern web browser with complete C++23 support including `std::expected`.

## File Outputs Explained

- **DeltaWebUI.js**: JavaScript glue code that loads and initializes the WebAssembly module
- **DeltaWebUI.wasm**: The compiled C++ application in WebAssembly format  
- **index.html**: Pre-configured web page with proper canvas setup and loading code
- **lib*.a**: Static libraries for ImGui, DeltaMath, and DeltaCore components

The total output is optimized for web deployment and can be hosted on any static web server, including GitHub Pages.