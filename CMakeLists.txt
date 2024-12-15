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


include(AddGitSubmodule)
add_subdirectory(configured)
add_subdirectory(Delta++)
add_subdirectory(Delta++UI)