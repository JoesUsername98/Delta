file(GLOB_RECURSE ${DPP_LIB_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_library(${DPP_LIB_NAME} STATIC ${${DPP_LIB_NAME}_SRC})

target_include_directories(${DPP_LIB_NAME} PUBLIC
    "./inc"
    "${CMAKE_BINARY_DIR}/configured_files/include")

enable_testing()

add_executable(
    ${DPP_LIB_NAME}_TEST
    test/test.cpp
)
target_link_libraries(
    ${DPP_LIB_NAME}_TEST
    ${DPP_LIB_NAME}
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${DPP_LIB_NAME}_TEST)
