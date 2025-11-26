cmake_minimum_required(VERSION 3.10)
project(client LANGUAGES C CXX)

# Set C++ standard (change to 20 if you need)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR/build})

# Include directory
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    include/glad/glad.c
)

# Create executable
add_executable(scuffed_minecraft ${SOURCES})

# Link libraries
target_link_libraries(scuffed_minecraft
    m          # math library
    GL         # OpenGL
    glfw       # GLFW
)

