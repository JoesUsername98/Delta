# WebAssembly build for Delta++UI
cmake_minimum_required(VERSION 3.16)

# Check if we're building with Emscripten
if(EMSCRIPTEN)
    project(Delta++WebUI)
    
    # Set C++ standard
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    
    # Find ImGui - we'll need to download it
    find_path(IMGUI_DIR
        NAMES imgui.h
        PATHS 
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../imgui
            ${CMAKE_CURRENT_SOURCE_DIR}/../../imgui  
            ${CMAKE_CURRENT_SOURCE_DIR}/../imgui
        DOC "ImGui directory"
    )
    
    if(NOT IMGUI_DIR)
        message(FATAL_ERROR "ImGui directory not found. Please clone ImGui to a parent directory or set IMGUI_DIR")
    endif()
    
    message(STATUS "Found ImGui at: ${IMGUI_DIR}")
    
    # ImGui sources
    set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp  
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )\n    \n    # Delta++ Math sources\n    set(DELTA_MATH_SOURCES\n        ../Delta++Math/src/normal_distribution.cpp\n        ../Delta++Math/src/stats_calculator.cpp\n    )\n    \n    # Delta++ Core sources\n    set(DELTA_CORE_SOURCES\n        ../Delta++/src/black_scholes_engine.cpp\n        ../Delta++/src/engine_factory.cpp\n        ../Delta++/src/monte_carlo_engine.cpp\n        ../Delta++/src/binomial_engine.cpp\n    )\n    \n    # Main WebUI source\n    set(WEBUI_SOURCES\n        main.cpp\n    )\n    \n    # Create executable\n    add_executable(Delta++WebUI\n        ${WEBUI_SOURCES}\n        ${IMGUI_SOURCES}\n        ${DELTA_MATH_SOURCES}\n        ${DELTA_CORE_SOURCES}\n    )\n    \n    # Include directories\n    target_include_directories(Delta++WebUI PRIVATE\n        ${IMGUI_DIR}\n        ${IMGUI_DIR}/backends\n        ../Delta++Math/inc\n        ../Delta++/inc\n        ../configured/include\n    )\n    \n    # Emscripten-specific settings\n    set_target_properties(Delta++WebUI PROPERTIES\n        COMPILE_FLAGS \"-s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1\"\n        LINK_FLAGS \"-s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 -s ASSERTIONS=1 -o delta.js\"\n    )\n    \n    # Set output directory\n    set_target_properties(Delta++WebUI PROPERTIES\n        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}\n    )\n    \nelse()\n    message(WARNING \"This CMakeLists.txt is intended for Emscripten builds only\")\nendif()