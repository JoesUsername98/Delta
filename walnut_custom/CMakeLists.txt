cmake_minimum_required(VERSION 3.16)
project(walnut_custom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add GLFW from Walnut submodule
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_SOURCE_DIR}/Delta++UI/vendor/Walnut/vendor/GLFW ${CMAKE_BINARY_DIR}/walnut_glfw EXCLUDE_FROM_ALL)

# Add ImGui from Walnut submodule
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/Delta++UI/vendor/Walnut/vendor/imgui)
file(GLOB IMGUI_SOURCES 
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR} 
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PRIVATE glfw)

# Create the custom Walnut library with OpenGL backend
add_library(walnut_custom STATIC
    ApplicationOpenGL.cpp
    ImageOpenGL.cpp
)

# Include directories
target_include_directories(walnut_custom PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/Delta++UI/vendor/Walnut/Walnut/src
    ${CMAKE_SOURCE_DIR}/Delta++UI/vendor/Walnut/vendor/imgui
    ${CMAKE_SOURCE_DIR}/Delta++UI/vendor/Walnut/vendor/imgui/backends
    ${CMAKE_SOURCE_DIR}/Delta++UI/vendor/Walnut/vendor/glm
    ${CMAKE_SOURCE_DIR}/Delta++UI/vendor/Walnut/vendor/GLFW/include
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Link against required libraries
target_link_libraries(walnut_custom PUBLIC
    OpenGL::GL
    glfw
    imgui
)

# Define WALNUT_USE_OPENGL for this library
target_compile_definitions(walnut_custom PUBLIC WALNUT_USE_OPENGL)

# Windows-specific settings
if(WIN32)
    target_link_libraries(walnut_custom PRIVATE dwmapi)
    target_compile_definitions(walnut_custom PUBLIC WL_PLATFORM_WINDOWS)
endif()

# Ensure proper dependencies
add_dependencies(walnut_custom glfw imgui)