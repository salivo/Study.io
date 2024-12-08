project(emtest)

set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER em++)

set(CMAKE_EXECUTABLE_SUFFIX ".html")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -s USE_GLFW=3 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS=['_main','_update_position','_delete_player'] -s EXPORTED_RUNTIME_METHODS=ccall,cwrap")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -s USE_GLFW=3 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1 -s EXPORTED_FUNCTIONS=['_main','_update_position','_delete_player','_set_my_uuid'] -s EXPORTED_RUNTIME_METHODS=ccall,cwrap")

add_executable(client src/main.cpp src/player.cpp)
include_directories(${PROJECT_SOURCE_DIR}/src/include)


target_include_directories(client PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/raylib/src
    ${CMAKE_SOURCE_DIR}/lib/websocketpp  # Ensure WebSocket++ headers are included
)

# Copy resources to the build directory
file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR})

# Preload the copied resources folder
target_link_options(client PUBLIC --preload-file resources)


add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/raylib/src/libraylib.a"
)

target_link_libraries(client raylib)
