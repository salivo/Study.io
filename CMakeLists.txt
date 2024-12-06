project(emtest)

set(CMAKE_C_COMPILER emcc)
set(CMAKE_CXX_COMPILER em++)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -s USE_GLFW=3 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -s USE_GLFW=3 -s ASYNCIFY -s ALLOW_MEMORY_GROWTH=1")

# Add your executable
add_executable(client src/main.cc)
target_include_directories(client PRIVATE ${CMAKE_SOURCE_DIR}/lib/raylib/src)

# Import the pre-built raylib library
add_library(raylib STATIC IMPORTED)
set_target_properties(raylib PROPERTIES
    IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/raylib/src/libraylib.a"
)

# Link raylib to your target
target_link_libraries(client raylib)
