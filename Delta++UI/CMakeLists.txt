if(POLICY CMP0079 )
  cmake_policy(SET CMP0079 NEW)
endif()


file(GLOB_RECURSE ${EXECUTABLE_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
)

add_executable(${EXECUTABLE_NAME} ${${EXECUTABLE_NAME}_SRC})

# Add include directories 
target_include_directories(${EXECUTABLE_NAME} PUBLIC 
"./inc"
"${CMAKE_BINARY_DIR}/configured_files/include"
"../walnut_custom"
"vendor/Walnut/Walnut/src"
"vendor/Walnut/vendor/imgui"
"vendor/Walnut/vendor/imgui/backends"
"vendor/Walnut/vendor/glm"
)

target_link_libraries(${EXECUTABLE_NAME} PUBLIC ${DPP_LIB_NAME} walnut_custom magic_enum::magic_enum)

set_target_properties(${EXECUTABLE_NAME} PROPERTIES FOLDER "Delat++")