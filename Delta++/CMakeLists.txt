set(LIBRARY_SOURCES
    "dllmain.cpp"
    "pch.cpp"
    "triangularmatrix.cpp"
    "trimatrixbuilder.cpp")
set(LIBRARY_HEADERS
    "framework.h"
    "pch.h"
    "triangularmatrix.h"
    "trimatrixbuilder.h")

add_library(${DPP_LIB_NAME} STATIC
    ${LIBRARY_SOURCES}
    ${LIBRARY_HEADERS})
target_include_directories(${DPP_LIB_NAME} PUBLIC
    "./"
    "${CMAKE_BINARY_DIR}/configured_files/include")
