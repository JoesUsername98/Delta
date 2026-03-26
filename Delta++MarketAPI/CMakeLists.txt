set(DPP_MARKET_API_LIB_NAME "Delta++MarketAPI")

file(GLOB_RECURSE ${DPP_MARKET_API_LIB_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE ${DPP_MARKET_API_LIB_NAME}_TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp
)

add_library(${DPP_MARKET_API_LIB_NAME} STATIC ${${DPP_MARKET_API_LIB_NAME}_SRC})

target_include_directories(${DPP_MARKET_API_LIB_NAME} PUBLIC
    "./inc"
    "${CMAKE_BINARY_DIR}/configured_files/include")

target_link_libraries(${DPP_MARKET_API_LIB_NAME} PUBLIC libcurl_static)

enable_testing()

add_executable(
    ${DPP_MARKET_API_LIB_NAME}_Test
    ${${DPP_MARKET_API_LIB_NAME}_TEST_FILES}
)
target_link_libraries(${DPP_MARKET_API_LIB_NAME}_Test PUBLIC ${DPP_MARKET_API_LIB_NAME})
target_link_libraries(${DPP_MARKET_API_LIB_NAME}_Test PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${DPP_MARKET_API_LIB_NAME}_Test TEST_PREFIX "DeltaPP.")

set_target_properties(${DPP_MARKET_API_LIB_NAME} PROPERTIES FOLDER "Delta++/Lib")
set_target_properties(${DPP_MARKET_API_LIB_NAME}_Test PROPERTIES FOLDER "Delta++/Test")
