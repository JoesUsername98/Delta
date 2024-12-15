cmake_minimum_required(VERSION 3.13)

project("Delta++" VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD          20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(DPP_LIB_NAME "Delta++")
set(EXECUTABLE_NAME "Delta++UI")
set(TEST_NAME "D++Tests")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(AddGitSubmodule)
add_subdirectory(configured)
add_subdirectory(Delta++)
add_subdirectory(Delta++UI)