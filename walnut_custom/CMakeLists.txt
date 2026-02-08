cmake_minimum_required(VERSION 3.13...3.22)
project(walnut_custom)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use FetchContent for GLFW
include(FetchContent)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# Use FetchContent for ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

# Create ImGui library with OpenGL backend
set(IMGUI_DIR ${imgui_SOURCE_DIR})
add_library(imgui STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC 
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
target_link_libraries(imgui PRIVATE glfw)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Add GLAD for OpenGL loading\nFetchContent_Declare(\n    glad\n    GIT_REPOSITORY https://github.com/Dav1dde/glad.git\n    GIT_TAG v0.1.36\n    GIT_SHALLOW TRUE\n)\nFetchContent_MakeAvailable(glad)\n\n# Add GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

# Create the custom Walnut library with OpenGL backend
add_library(walnut_custom STATIC
    ApplicationOpenGL.cpp
    ImageOpenGL.cpp
)

# Include directories
target_include_directories(walnut_custom PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${glm_SOURCE_DIR}
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