include(FetchContent)

FetchContent_Declare(
    Walnut-cmake
    GIT_REPOSITORY https://github.com/Mr-Precise/Walnut-cmake
    GIT_TAG v2
    )
FetchContent_MakeAvailable(Walnut-cmake)

if(POLICY CMP0079 )
  cmake_policy(SET CMP0079 NEW)
endif()

find_package(Vulkan REQUIRED)

message(STATUS "Found Vulkan: ${Vulkan_LIBRARIES}")
message(STATUS "Vulkan include dirs: ${Vulkan_INCLUDE_DIRS}")
target_link_libraries(Walnut PUBLIC Vulkan::Vulkan)

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum/
    GIT_TAG v0.9.7
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(magic_enum)

file(GLOB_RECURSE ${EXECUTABLE_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
)

add_executable(${EXECUTABLE_NAME} ${${EXECUTABLE_NAME}_SRC})
target_include_directories(${EXECUTABLE_NAME} PUBLIC 
"./inc"
"${CMAKE_BINARY_DIR}/configured_files/include"
)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${DPP_LIB_NAME} Walnut Vulkan::Vulkan glfw opengl32.lib magic_enum::magic_enum )