cmake_minimum_required(VERSION 3.20)

project(AIEnhancedRenderer VERSION 0.1.0 LANGUAGES C CXX)

option(AIER_ENABLE_ONNXRUNTIME "Link local ONNX Runtime from extern/onnxruntime" OFF)

# Shared dependency setup (FetchContent for glfw/glm/assimp/imgui).
include(cmake/Dependencies.cmake)

# Build main target from src/ and apply shared options there.
add_subdirectory(src)