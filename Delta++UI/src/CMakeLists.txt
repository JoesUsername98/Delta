

include(FetchContent)

FetchContent_Declare(
    Walnut-cmake
    GIT_REPOSITORY https://github.com/Mr-Precise/Walnut-cmake
    GIT_TAG v2
    # GIT_SHALLOW TRUE
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

if(MINGW OR WIN32)
    message(STATUS "WalnutApp Windows build")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
endif(MINGW OR WIN32)

set(EXE_SOURCES 
"BinomialPricerUI.cpp"
"binomialpricerstate.cpp"
)
set(EXE_INCLUDES 
"magic_enum.h"
"binomialpricerstate.h"
"./")

file(GLOB_RECURSE WALNUT_INCLUDES
    ${Walnut_SOURCE_DIR}/src/**.h
)
file(GLOB_RECURSE WALNUT_SOURCES
    ${Walnut_SOURCE_DIR}/src/**.h
)
add_executable(${EXECUTABLE_NAME} 
${EXE_SOURCES}
${WALNUT_SOURCES}
)

target_include_directories(${EXECUTABLE_NAME} PUBLIC 
${EXE_INCLUDES}
${WALNUT_INCLUDES})
target_link_libraries(
  ${EXECUTABLE_NAME} PUBLIC 
  ${DPP_LIB_NAME}
  Walnut
  #Vulkan::Vulkan
  #glfw
    #nlohmann_json::nlohmann_json
    #fmt::fmt
    #spdlog::spdlog
    #cxxopts::cxxopts
    )
