cmake_minimum_required(VERSION 3.13)

set(DPP_MARKETDATA_LIB_NAME "Delta++MarketData")
set(DPP_MARKETDATA_SERVICE_EXE_NAME "Delta++MarketDataService")

# ---------------------------------------------------------------------------
# Service library (only built when source files are actually present).
# ---------------------------------------------------------------------------
file(GLOB_RECURSE ${DPP_MARKETDATA_LIB_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

if (${DPP_MARKETDATA_LIB_NAME}_SRC)
    add_library(${DPP_MARKETDATA_LIB_NAME} STATIC ${${DPP_MARKETDATA_LIB_NAME}_SRC})

    target_include_directories(${DPP_MARKETDATA_LIB_NAME} PUBLIC
        "./inc"
        "${CMAKE_BINARY_DIR}/configured_files/include"
    )

    target_link_libraries(${DPP_MARKETDATA_LIB_NAME}
        PUBLIC
            Delta++DB
    )

    if (WIN32)
        target_compile_definitions(${DPP_MARKETDATA_LIB_NAME} PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()

    set_target_properties(${DPP_MARKETDATA_LIB_NAME} PROPERTIES FOLDER "Delta++/Services")

    # Service exe depends on the library; only attempt if both lib and main.cpp are present.
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/service/main.cpp")
        add_executable(${DPP_MARKETDATA_SERVICE_EXE_NAME}
            ${CMAKE_CURRENT_SOURCE_DIR}/service/main.cpp
        )
        target_link_libraries(${DPP_MARKETDATA_SERVICE_EXE_NAME}
            PRIVATE
                ${DPP_MARKETDATA_LIB_NAME}
        )
        set_target_properties(${DPP_MARKETDATA_SERVICE_EXE_NAME} PROPERTIES FOLDER "Delta++/Services")
    endif()

    # Bench exe (only if bench sources exist).
    file(GLOB_RECURSE ${DPP_MARKETDATA_LIB_NAME}_BENCH_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/bench/*.cpp
    )
    if (${DPP_MARKETDATA_LIB_NAME}_BENCH_FILES)
        add_executable(${DPP_MARKETDATA_LIB_NAME}_Bench ${${DPP_MARKETDATA_LIB_NAME}_BENCH_FILES})
        target_link_libraries(${DPP_MARKETDATA_LIB_NAME}_Bench
            PRIVATE
                ${DPP_MARKETDATA_LIB_NAME}
                benchmark::benchmark
                benchmark::benchmark_main
        )
        set_target_properties(${DPP_MARKETDATA_LIB_NAME}_Bench PROPERTIES FOLDER "Delta++/Services/Bench")
    endif()
endif()

# ---------------------------------------------------------------------------
# Test executable (standalone - includes its own classes inline). Builds even
# when the service library above is absent so the practice file can be
# exercised in isolation.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE ${DPP_MARKETDATA_LIB_NAME}_TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp
)

if (${DPP_MARKETDATA_LIB_NAME}_TEST_FILES)
    enable_testing()

    add_executable(${DPP_MARKETDATA_LIB_NAME}_Test
        ${${DPP_MARKETDATA_LIB_NAME}_TEST_FILES}
    )

    target_link_libraries(${DPP_MARKETDATA_LIB_NAME}_Test
        PRIVATE
            GTest::gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(${DPP_MARKETDATA_LIB_NAME}_Test TEST_PREFIX "DeltaPP.")

    set_target_properties(${DPP_MARKETDATA_LIB_NAME}_Test PROPERTIES FOLDER "Delta++/Services/Test")
endif()
