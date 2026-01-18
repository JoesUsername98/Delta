CXX = emcc
OUTPUT = delta.js
IMGUI_DIR = ../../../imgui

# Delta++ includes
DELTA_MATH_DIR = ../Delta++Math
DELTA_CORE_DIR = ../Delta++

SOURCES = main.cpp
SOURCES += $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SOURCES += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_widgets.cpp $(IMGUI_DIR)/imgui_tables.cpp

# Delta++ Math sources
SOURCES += $(DELTA_MATH_DIR)/src/distributions.cpp

# Delta++ Core sources  
SOURCES += $(DELTA_CORE_DIR)/src/black_scholes_engine.cpp
SOURCES += $(DELTA_CORE_DIR)/src/abstract_engine.cpp

LIBS = -lGL
WEBGL_VER = -s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1
USE_WASM = -s WASM=1
EMCC_FLAGS = -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 -s ASSERTIONS=1

INCLUDE_DIRS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends 
INCLUDE_DIRS += -I$(DELTA_MATH_DIR)/inc -I$(DELTA_CORE_DIR)/inc
INCLUDE_DIRS += -I../configured/include

all: $(SOURCES) $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CXX) $(SOURCES) -std=c++23 -o $(OUTPUT) $(LIBS) $(WEBGL_VER) $(USE_WASM) $(EMCC_FLAGS) -O2 $(INCLUDE_DIRS)

clean:
	rm -f $(OUTPUT) delta.wasm delta.data

.PHONY: all clean