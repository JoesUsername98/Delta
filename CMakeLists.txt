cmake_minimum_required(VERSION 3.13)

project("Delta++" VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(DPP_LIB_NAME "Delta++")
set(DPP_MATH_LIB_NAME "Delta++Math")
set(EXECUTABLE_NAME "Delta++UI")
set(TEST_NAME "D++Tests")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

# CMake tools
include(AddGitSubmodule)

# Options
if(WIN32)
  option(ENABLE_COVERAGE "Enable a code coverage build." OFF)
else()  
  option(ENABLE_COVERAGE "Enable a code coverage build." ON)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/fp:precise)
    message(STATUS "Globally applied MSVC strict FP flag using add_compile_options().")

elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -fno-fast-math
        -fno-associative-math
        -frounding-math
    )
    message(STATUS "Globally applied GNU/Clang strict FP flags using add_compile_options().")
endif()

if ( ENABLE_COVERAGE )
  include( CodeCoverage )
  append_coverage_compiler_flags()
endif ( ENABLE_COVERAGE )

add_subdirectory(configured)
add_subdirectory(Delta++Math)
add_subdirectory(Delta++)
if (WIN32)
  add_subdirectory(Delta++UI)
endif (WIN32)