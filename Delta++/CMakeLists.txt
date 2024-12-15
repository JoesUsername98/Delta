file(GLOB_RECURSE ${DPP_LIB_NAME}_INC_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${DPP_LIB_NAME} STATIC ${${DPP_LIB_NAME}_INC_SRC})

target_include_directories(${DPP_LIB_NAME} PUBLIC
    "./"
    "${CMAKE_BINARY_DIR}/configured_files/include")

add_subdirectory(test)
