cmake_minimum_required(VERSION 3.20)

project(AIEnhancedRenderer VERSION 0.1.0 LANGUAGES C CXX)

option(AIER_ENABLE_ONNXRUNTIME "Link local ONNX Runtime from extern/onnxruntime" OFF)
option(AIER_ENABLE_OPENCV "Enable OpenCV super-resolution integration" ON)
option(AIER_ENABLE_IMGUI "Enable Dear ImGui debug UI" ON)

include(cmake/Dependencies.cmake)
find_package(assimp CONFIG REQUIRED)
if(AIER_ENABLE_OPENCV)
    find_package(OpenCV REQUIRED COMPONENTS
    core
    imgproc
    imgcodecs
    dnn
    dnn_superres
)
    find_library(OPENCV_DNN_SUPERRES NAMES opencv_dnn_superres
        PATHS /usr/lib64 /usr/lib ${OpenCV_INSTALL_PATH}/lib ${OpenCV_DIR}/lib)
endif()

add_subdirectory(src)

if(AIER_ENABLE_OPENCV)
    enable_testing()
    add_subdirectory(tests)
endif()
