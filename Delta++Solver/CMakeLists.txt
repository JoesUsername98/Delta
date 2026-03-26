set(DPP_SOLVER_LIB_NAME "Delta++Solver")

file(GLOB_RECURSE ${DPP_SOLVER_LIB_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE ${DPP_SOLVER_LIB_NAME}_TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp
)

add_library(${DPP_SOLVER_LIB_NAME} STATIC ${${DPP_SOLVER_LIB_NAME}_SRC})

target_include_directories(${DPP_SOLVER_LIB_NAME} PUBLIC
    "./inc"
    "${CMAKE_BINARY_DIR}/configured_files/include")

target_link_libraries(${DPP_SOLVER_LIB_NAME} PUBLIC ${DPP_MATH_LIB_NAME})

enable_testing()

add_executable(
    ${DPP_SOLVER_LIB_NAME}_Test
    ${${DPP_SOLVER_LIB_NAME}_TEST_FILES}
)
target_link_libraries(${DPP_SOLVER_LIB_NAME}_Test PUBLIC ${DPP_SOLVER_LIB_NAME})
target_link_libraries(${DPP_SOLVER_LIB_NAME}_Test PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${DPP_SOLVER_LIB_NAME}_Test TEST_PREFIX "DeltaPP.")

set_target_properties(${DPP_SOLVER_LIB_NAME} PROPERTIES FOLDER "Delta++/Lib")
set_target_properties(${DPP_SOLVER_LIB_NAME}_Test PROPERTIES FOLDER "Delta++/Test")
