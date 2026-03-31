set(DPP_BLACK_SCHOLES_LIB_NAME "Delta++BlackScholes")

file(GLOB_RECURSE ${DPP_BLACK_SCHOLES_LIB_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${DPP_BLACK_SCHOLES_LIB_NAME} STATIC ${${DPP_BLACK_SCHOLES_LIB_NAME}_SRC})

target_include_directories(${DPP_BLACK_SCHOLES_LIB_NAME} PUBLIC
    "./inc"
    "${CMAKE_BINARY_DIR}/configured_files/include")

target_link_libraries(${DPP_BLACK_SCHOLES_LIB_NAME} PUBLIC
    ${DPP_MATH_LIB_NAME})

set_target_properties(${DPP_BLACK_SCHOLES_LIB_NAME} PROPERTIES FOLDER "Delta++/Lib")

