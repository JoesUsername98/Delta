file(GLOB_RECURSE ${DPP_LIB_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# Exclude problematic files for WebAssembly builds
if(EMSCRIPTEN)
    list(REMOVE_ITEM ${DPP_LIB_NAME}_SRC 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/monte_carlo_exercise.cpp"
    )
    message(STATUS "Excluded Monte Carlo files for WebAssembly build")
endif()

file(GLOB_RECURSE ${DPP_LIB_NAME}_TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp
)

file(GLOB_RECURSE ${DPP_LIB_NAME}_BENCH_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/bench/*.cpp
)

message("Benchmark files" ${${DPP_LIB_NAME}_BENCH_FILES})

add_library(${DPP_LIB_NAME} STATIC ${${DPP_LIB_NAME}_SRC})
target_link_libraries(${DPP_LIB_NAME} ${DPP_MATH_LIB_NAME} )
target_link_libraries(${DPP_LIB_NAME} magic_enum::magic_enum )



target_include_directories(${DPP_LIB_NAME} PUBLIC
    "./inc"
    "${CMAKE_BINARY_DIR}/configured_files/include")

enable_testing()

add_executable(
    ${DPP_LIB_NAME}_Test
    ${${DPP_LIB_NAME}_TEST_FILES}
)

target_link_libraries(${DPP_LIB_NAME}_Test PUBLIC ${DPP_LIB_NAME})
target_link_libraries(${DPP_LIB_NAME}_Test PRIVATE GTest::gtest_main)

add_executable(
    ${DPP_LIB_NAME}_Bench
    ${${DPP_LIB_NAME}_BENCH_FILES}
)

set(BENCHMARK_ENABLE_TESTING off)

target_link_libraries(
    ${DPP_LIB_NAME}_Bench 
    PRIVATE 
        ${DPP_LIB_NAME} 
        benchmark::benchmark
        benchmark::benchmark_main
)

include(GoogleTest)
gtest_discover_tests(${DPP_LIB_NAME}_Test)

if(ENABLE_COVERAGE)
    set(COVERAGE_MAIN "coverage")
     set(COVERAGE_EXCLUDES 
         #"${CMAKE_BINARY_DIR}/dirs_to_exclude/*"
         "${CMAKE_BINARY_DIR}/_deps/google*"
         "${CMAKE_BINARY_DIR}/_deps/benchmark*"
         "/usr/include/*" )
    set(COVERAGE_EXTRA_FLAGS)
    set(COVERAGE_DEPENDENCIES ${DPP_LIB_NAME}_Test)

    setup_target_for_coverage_gcovr_html(
        NAME 
        ${COVERAGE_MAIN} 
        EXECUTABLE 
        ${COVERAGE_DEPENDENCIES} 
        BASE_DIRECTORY 
        ${CMAKE_SOURCE_DIR}
    )
endif( ENABLE_COVERAGE )

set_target_properties(${DPP_LIB_NAME} PROPERTIES FOLDER "Delat++/Lib")
set_target_properties(${DPP_LIB_NAME}_Test  PROPERTIES FOLDER "Delat++/Test")
set_target_properties(${DPP_LIB_NAME}_Bench PROPERTIES FOLDER "Delat++/Bench")
