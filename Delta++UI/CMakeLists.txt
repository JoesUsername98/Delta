include(FetchContent)

message(STATUS "FetchContent_Declare Walnut-cmake")
FetchContent_Declare(
    Walnut-cmake
    GIT_REPOSITORY https://github.com/Mr-Precise/Walnut-cmake
    GIT_TAG v2
    )
message(STATUS "FetchContent_MakeAvailable Walnut-cmake")
FetchContent_MakeAvailable(Walnut-cmake)

if(POLICY CMP0079 )
  cmake_policy(SET CMP0079 NEW)
endif()

message(STATUS "find_package Vulkan")
find_package(Vulkan REQUIRED)

message(STATUS "Found Vulkan: ${Vulkan_LIBRARIES}")
message(STATUS "Vulkan include dirs: ${Vulkan_INCLUDE_DIRS}")
target_link_libraries(Walnut PUBLIC Vulkan::Vulkan)

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

set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "Delat++")