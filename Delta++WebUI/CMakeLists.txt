# Delta++ WebAssembly CMake Configuration
cmake_minimum_required(VERSION 3.20)

# Only set project if this is the root CMakeLists.txt
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(DeltaWebUI VERSION 1.0.0)
    
    # Set build output directory when building standalone
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../build/Delta++WebUI)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../build/Delta++WebUI)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/../build/Delta++WebUI)
else()
    # When building as subdirectory, use standard CMake output directories
    # The files will be in the main build directory alongside other executables
endif()

# Ensure we're building with Emscripten
if(NOT EMSCRIPTEN)
    message(WARNING "Delta++WebUI requires Emscripten for WebAssembly build. Skipping...")
    return()
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Emscripten-specific settings
set(CMAKE_EXECUTABLE_SUFFIX ".js")

# Find ImGui directory automatically
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Building standalone - look for imgui in parent directories
    set(IMGUI_SEARCH_PATHS 
        "${CMAKE_CURRENT_SOURCE_DIR}/../imgui"
        "C:/repos/Delta/imgui"
    )
else()
    # Building as subdirectory - look for imgui relative to project root
    set(IMGUI_SEARCH_PATHS 
        "${CMAKE_SOURCE_DIR}/imgui"
        "${CMAKE_CURRENT_SOURCE_DIR}/../imgui"
    )
endif()

# Find the first existing ImGui directory
set(IMGUI_ROOT "")
foreach(search_path ${IMGUI_SEARCH_PATHS})
    if(EXISTS "${search_path}/imgui.h")
        set(IMGUI_ROOT "${search_path}")
        break()
    endif()
endforeach()

if(NOT IMGUI_ROOT)
    message(STATUS "Searched for ImGui in:")
    foreach(search_path ${IMGUI_SEARCH_PATHS})
        message(STATUS "  ${search_path}")
    endforeach()
    message(FATAL_ERROR "
================================================================================
ImGui not found! Please install ImGui:

  cd ${CMAKE_SOURCE_DIR}
  git clone https://github.com/ocornut/imgui.git --depth 1

================================================================================
")
endif()

message(STATUS "Found ImGui at: ${IMGUI_ROOT}")

# Create ImGui library
add_library(imgui STATIC
    ${IMGUI_ROOT}/imgui.cpp
    ${IMGUI_ROOT}/imgui_draw.cpp
    ${IMGUI_ROOT}/imgui_demo.cpp  
    ${IMGUI_ROOT}/imgui_widgets.cpp
    ${IMGUI_ROOT}/imgui_tables.cpp
    ${IMGUI_ROOT}/backends/imgui_impl_glfw.cpp
    ${IMGUI_ROOT}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${IMGUI_ROOT}
    ${IMGUI_ROOT}/backends
)

# Delta++ libraries - use existing targets if available (when building as subdirectory)
if(TARGET ${DPP_MATH_LIB_NAME} AND TARGET ${DPP_LIB_NAME})
    # Use existing targets from main build
    set(DELTA_MATH_LIB ${DPP_MATH_LIB_NAME})
    set(DELTA_CORE_LIB ${DPP_LIB_NAME})
    message(STATUS "Using existing Delta++ library targets")
else()
    # Create local targets for standalone build
    # Delta++ Math Library
    add_library(DeltaMath STATIC
        ../Delta++Math/src/distributions.cpp
    )

    target_include_directories(DeltaMath PUBLIC
        ../Delta++Math/inc
    )

    # Delta++ Core Library  
    add_library(DeltaCore STATIC
        ../Delta++/src/black_scholes_engine.cpp
        ../Delta++/src/abstract_engine.cpp
    )

    target_include_directories(DeltaCore PUBLIC
        ../Delta++/inc
        ../configured/include
    )

    target_link_libraries(DeltaCore PUBLIC DeltaMath)
    
    set(DELTA_MATH_LIB DeltaMath)
    set(DELTA_CORE_LIB DeltaCore)
    message(STATUS "Created local Delta++ library targets")
endif()

# Main WebAssembly Application
add_executable(DeltaWebUI
    src/main.cpp
)

target_include_directories(DeltaWebUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/inc  # For headers
    ${CMAKE_CURRENT_SOURCE_DIR}      # For compatibility
)

target_link_libraries(DeltaWebUI PRIVATE
    imgui
    ${DELTA_CORE_LIB}
    ${DELTA_MATH_LIB}
)

# Emscripten compiler flags
target_compile_options(DeltaWebUI PRIVATE
    -std=c++23
    -fexperimental-library
)

target_compile_definitions(DeltaWebUI PRIVATE
    __cpp_lib_expected=202211L
)

# Emscripten linker flags
set_target_properties(DeltaWebUI PROPERTIES
    LINK_FLAGS "-s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 -s NO_EXIT_RUNTIME=1 -s ASSERTIONS=1 -O2 -o delta.js"
)

# Custom target to copy HTML files
add_custom_command(TARGET DeltaWebUI POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/index.html
        $<TARGET_FILE_DIR:DeltaWebUI>/index.html
    COMMENT "Copying HTML files"
)

# Test target for simple ImGui demo
add_executable(DeltaWebUITest
    src/test_simple.cpp
)

target_link_libraries(DeltaWebUITest PRIVATE imgui)

set_target_properties(DeltaWebUITest PROPERTIES
    LINK_FLAGS "
        -s USE_WEBGL2=1 
        -s USE_GLFW=3 
        -s FULL_ES3=1
        -s WASM=1
        -s ALLOW_MEMORY_GROWTH=1
        -s NO_EXIT_RUNTIME=1
        -O2
        -o test.js
    "
)

add_custom_command(TARGET DeltaWebUITest POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/test.html  
        ${CMAKE_CURRENT_BINARY_DIR}/test.html
    COMMENT "Copying test HTML"
)

# Development server target
find_program(PYTHON_EXECUTABLE python3 python)
if(PYTHON_EXECUTABLE)
    add_custom_target(serve
        COMMAND ${PYTHON_EXECUTABLE} -m http.server 8080
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Starting development server on http://localhost:8080"
        DEPENDS DeltaWebUI
    )
endif()

# Help target
add_custom_target(deltahelp
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Delta++ WebAssembly Build Targets:"
    COMMAND ${CMAKE_COMMAND} -E echo "================================="
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "  DeltaWebUI     - Build full Delta++ WebAssembly application"
    COMMAND ${CMAKE_COMMAND} -E echo "  DeltaWebUITest - Build simple ImGui test"
    COMMAND ${CMAKE_COMMAND} -E echo "  serve          - Start development server"
    COMMAND ${CMAKE_COMMAND} -E echo "  help           - Show this help"
    COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${CMAKE_COMMAND} -E echo "Usage:"
    COMMAND ${CMAKE_COMMAND} -E echo "  emcmake cmake .. -G Ninja"
    COMMAND ${CMAKE_COMMAND} -E echo "  ninja DeltaWebUI"
    COMMAND ${CMAKE_COMMAND} -E echo "  python -m http.server 8080"
    COMMAND ${CMAKE_COMMAND} -E echo ""
)

# Status information
message(STATUS "=============================================================")
message(STATUS "Delta++ WebAssembly Configuration Summary")
message(STATUS "=============================================================")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message(STATUS "ImGui Location: ${IMGUI_ROOT}")
message(STATUS "Output: DeltaWebUI.js + DeltaWebUI.wasm")
message(STATUS "")
message(STATUS "To build:")
message(STATUS "  ninja DeltaWebUI")
message(STATUS "")
message(STATUS "To test:")
message(STATUS "  python -m http.server 8080")
message(STATUS "  Open: http://localhost:8080/index.html")
message(STATUS "=============================================================")

# Default target
add_custom_target(default ALL DEPENDS DeltaWebUI)