cmake_minimum_required(VERSION 3.20)
project(AIEnhancedRenderer)

set(CMAKE_CXX_STANDARD 20)

# Dependencies
include(cmake/Dependencies.cmake)

# Sources
file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(AIEnhancedRenderer ${SRC_FILES}
    extern/glad/src/glad.c
)

# Include dirs
target_include_directories(AIEnhancedRenderer PRIVATE
    src
    extern
    extern/glad/include
    extern/stb
    extern/onnxruntime/include
)

# Link libs
target_link_libraries(AIEnhancedRenderer
    glfw
    assimp
)

# Linux OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(AIEnhancedRenderer OpenGL::GL)

# ONNX
target_link_directories(AIEnhancedRenderer PRIVATE
    extern/onnxruntime/lib
)

target_link_libraries(AIEnhancedRenderer onnxruntime)