set(DPP_DB_LIB_NAME "Delta++DB")

file(READ "${CMAKE_CURRENT_LIST_DIR}/sql/reference_schema.sql" REFERENCE_SCHEMA_SQL_CONTENT)
configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/schema_embed.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/schema_embed.cpp"
  @ONLY
)

file(READ "${CMAKE_CURRENT_LIST_DIR}/sql/market_schema.sql" MARKET_SCHEMA_SQL_CONTENT)
configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/market_schema_embed.cpp.in"
  "${CMAKE_CURRENT_BINARY_DIR}/market_schema_embed.cpp"
  @ONLY
)

include(FetchContent)
FetchContent_Declare(
  sqlite3_amalgamation
  URL https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE # CMP0135: rebuild when archive changes
)
FetchContent_MakeAvailable(sqlite3_amalgamation)

file(GLOB_RECURSE SQLITE3_C "${sqlite3_amalgamation_SOURCE_DIR}/sqlite3.c")
if(NOT SQLITE3_C)
  message(FATAL_ERROR "sqlite3.c not found under ${sqlite3_amalgamation_SOURCE_DIR}")
endif()

add_library(sqlite3 STATIC ${SQLITE3_C})
target_include_directories(sqlite3 PUBLIC "${sqlite3_amalgamation_SOURCE_DIR}")
if(WIN32)
  target_link_libraries(sqlite3 PUBLIC bcrypt)
endif()

file(GLOB_RECURSE ${DPP_DB_LIB_NAME}_SRC
  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp
)

add_library(${DPP_DB_LIB_NAME} STATIC
  ${${DPP_DB_LIB_NAME}_SRC}
  "${CMAKE_CURRENT_BINARY_DIR}/schema_embed.cpp"
  "${CMAKE_CURRENT_BINARY_DIR}/market_schema_embed.cpp"
)

target_include_directories(${DPP_DB_LIB_NAME} PUBLIC
  "${CMAKE_CURRENT_LIST_DIR}/inc"
)

target_link_libraries(${DPP_DB_LIB_NAME} PUBLIC Delta++MarketAPI sqlite3)

file(GLOB_RECURSE ${DPP_DB_LIB_NAME}_TEST_FILES
  ${CMAKE_CURRENT_LIST_DIR}/test/*.cpp
)

add_executable(
  ${DPP_DB_LIB_NAME}_Test
  ${${DPP_DB_LIB_NAME}_TEST_FILES}
)
target_link_libraries(${DPP_DB_LIB_NAME}_Test PUBLIC ${DPP_DB_LIB_NAME})
target_link_libraries(${DPP_DB_LIB_NAME}_Test PRIVATE GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(${DPP_DB_LIB_NAME}_Test TEST_PREFIX "DeltaPP.")

set_target_properties(sqlite3 PROPERTIES FOLDER "Delta++/Lib")
set_target_properties(${DPP_DB_LIB_NAME} PROPERTIES FOLDER "Delta++/Lib")
set_target_properties(${DPP_DB_LIB_NAME}_Test PROPERTIES FOLDER "Delta++/Test")
