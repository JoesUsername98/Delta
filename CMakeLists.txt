cmake_minimum_required(VERSION 3.13...3.22)

project("Delta++" VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# Lets VS Code/Cursor C++ IntelliSense (and clangd) use real compile flags for Go to Definition / includes.
# With Visual Studio generators this is supported in CMake 3.20+ (VS 16+).
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(DPP_LIB_NAME "Delta++")
set(DPP_MATH_LIB_NAME "Delta++Math")
set(EXECUTABLE_NAME "Delta++UI")
set(TEST_NAME "D++Tests")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Enable testing at the top level
enable_testing()

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark/
  GIT_TAG v1.9.1
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(benchmark)

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY https://github.com/Neargye/magic_enum/
    GIT_TAG v0.9.7
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(magic_enum)

FetchContent_Declare(
  json
  URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
FetchContent_MakeAvailable(json)

set(CURL_ENABLE_SSL ON CACHE BOOL "" FORCE)
# Schannel is Windows-only; Linux/WSL need OpenSSL (or another TLS backend).
if(WIN32)
  set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
else()
  set(CURL_USE_SCHANNEL OFF CACHE BOOL "" FORCE)
  set(CURL_USE_OPENSSL ON CACHE BOOL "" FORCE)
endif()
option(BUILD_CURL_EXE "Build curl executable" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_11_1
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(curl)

# CMake tools
include(AddGitSubmodule)

# Options
if(WIN32)
  option(ENABLE_COVERAGE "Enable a code coverage build." OFF)
else()  
  option(ENABLE_COVERAGE "Enable a code coverage build." ON)
endif ()

if ( ENABLE_COVERAGE )
  include( CodeCoverage )
  append_coverage_compiler_flags()
endif ( ENABLE_COVERAGE )

add_subdirectory(configured)
add_subdirectory(Delta++Math)
add_subdirectory(Delta++Solver)
add_subdirectory(Delta++MarketAPI)
add_subdirectory(Delta++DB)
add_subdirectory(Delta++Market)
add_subdirectory(Delta++)
if (WIN32)
  add_subdirectory(walnut_custom)
  add_subdirectory(Delta++UI)
endif (WIN32)

# Copy compilation database to repo root so C_Cpp.default.compileCommands in .vscode/settings.json resolves.
if(CMAKE_EXPORT_COMPILE_COMMANDS AND EXISTS "${CMAKE_BINARY_DIR}/compile_commands.json")
  execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/compile_commands.json"
    "${CMAKE_SOURCE_DIR}/compile_commands.json")
endif()