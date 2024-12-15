enable_testing()

add_executable(
    ${DPP_LIB_NAME}_TEST
    test.cpp
)
target_link_libraries(
    ${DPP_LIB_NAME}_TEST
    ${DPP_LIB_NAME}
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${DPP_LIB_NAME}_TEST)