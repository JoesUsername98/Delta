# Delta++ WebUI

WebAssembly interface for Delta++ derivatives pricing library using Dear ImGui.

## Project Structure

```
Delta++WebUI/
├── src/
│   ├── main.cpp              # Main WebAssembly application  
│   └── test_simple.cpp       # Simple ImGui test application
├── inc/
│   └── compatibility.hpp     # C++23 std::expected polyfill
├── CMakeLists.txt            # Build configuration
├── index.html               # Web page template
├── BUILD_INSTRUCTIONS.md    # Complete build guide
└── README.md               # This file
```

**Build Output**: `../build/Delta++WebUI/` (matches Delta++UI pattern)

## Features

- **Modern C++23**: Full support including `std::expected`
- **WebAssembly Compilation**: Optimized WASM output via Emscripten
- **Dear ImGui Interface**: Professional derivatives trading UI

## Building

### Prerequisites

1. **Emscripten SDK**: Required for WebAssembly compilation
   ```bash
   git clone https://github.com/emscripten-core/emsdk.git
   cd emsdk
   ./emsdk install latest
   ./emsdk activate latest
   source ./emsdk_env.sh  # Linux/Mac
   # or emsdk_env.bat     # Windows
   ```

2. **Dear ImGui**: Required for UI rendering
   ```bash
   cd .. # Go to parent directory
   git clone https://github.com/ocornut/imgui.git
   ```

### Build Instructions

#### Linux/Mac
```bash
./build.sh
```

#### Windows
```batch
build.bat
```

#### Manual Build
```bash
make
```

### Generated Files

After building, you'll have:
- `delta.js` - The main JavaScript module
- `delta.wasm` - The WebAssembly binary
- `index.html` - The web page (already provided)

## Testing Locally

Start a local web server:
```bash
python3 -m http.server 8080
```

Then open: http://localhost:8080/index.html

## Deployment to GitHub Pages

1. Copy the generated files to your GitHub Pages repository:
   - `delta.js`
   - `delta.wasm` 
   - `index.html`

2. Push to your GitHub Pages branch (usually `main` or `gh-pages`)

3. Your application will be available at: `https://yourusername.github.io/`

## Current Limitations

- **Engine Support**: Only Black-Scholes engine is currently implemented
- **C++23 Features**: Some C++23 features may not be supported by Emscripten
- **File I/O**: No file import/export functionality in browser version
- **Performance**: Slightly slower than native desktop version

## Planned Features

- [ ] Monte Carlo engine with web workers
- [ ] Binomial engine implementation  
- [ ] Export results to CSV/JSON
- [ ] Mobile-optimized interface
- [ ] Progressive web app (PWA) support

## Architecture

The WebAssembly version maintains the same calculation engines as the desktop version but replaces:

- **Walnut Framework** → Direct ImGui + WebGL2
- **Vulkan Graphics** → OpenGL ES3
- **Desktop GLFW** → Emscripten GLFW
- **Native Threading** → Web Workers (planned)

## Browser Compatibility

- Chrome 57+
- Firefox 52+
- Safari 11+
- Edge 79+

Requires WebGL2 and WebAssembly support.

## Development Notes

### Adding New Engines

To add support for Monte Carlo or Binomial engines:

1. Include the engine source in `Makefile` SOURCES
2. Add the engine logic in `calculatePricing()` function
3. Handle any threading with web workers
4. Test performance implications

### Memory Management

The WebAssembly version uses:
- `ALLOW_MEMORY_GROWTH=1` for dynamic allocation
- `NO_EXIT_RUNTIME=1` for persistent execution  
- Automatic garbage collection for ImGui resources

### Debugging

Enable debug mode by modifying Makefile:
```makefile
EMCC_FLAGS = -s ASSERTIONS=1 -s SAFE_HEAP=1 -g
```