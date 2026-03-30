set(DPP_DATASYNC_EXE_NAME "Delta++DataSync")

set(${DPP_DATASYNC_EXE_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/datasync_args.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/datasync_app.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/job_factory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/option_contract_sync.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/yield_curve_sync.cpp
)

add_executable(${DPP_DATASYNC_EXE_NAME} ${${DPP_DATASYNC_EXE_NAME}_SRC})

target_include_directories(${DPP_DATASYNC_EXE_NAME} PUBLIC
    "./inc"
    "${CMAKE_BINARY_DIR}/configured_files/include"
)

target_link_libraries(${DPP_DATASYNC_EXE_NAME} PUBLIC
    Delta++MarketAPI
    Delta++DB
)

set_target_properties(${DPP_DATASYNC_EXE_NAME} PROPERTIES FOLDER "Delta++")

