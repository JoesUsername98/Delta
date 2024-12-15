file(GLOB_RECURSE ${DPP_LIB_NAME}_INC_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

message("${DPP_LIB_NAME}_INC_SR from ${CMAKE_CURRENT_SOURCE_DIR} :${${DPP_LIB_NAME}_INC_SRC} ")
add_library(${DPP_LIB_NAME} STATIC ${${DPP_LIB_NAME}_INC_SRC})

target_include_directories(${DPP_LIB_NAME} PUBLIC
    "./"
    "${CMAKE_BINARY_DIR}/configured_files/include")

add_subdirectory(test)
