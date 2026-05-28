if(NOT AIER_ENABLE_OPENCV)
    return()
endif()

find_package(OpenGL REQUIRED)

add_executable(test_frame_capture
    test_frame_capture.cpp
    ${PROJECT_SOURCE_DIR}/src/rendering/FrameCapture.cpp
    ${PROJECT_SOURCE_DIR}/extern/glad/src/glad.c
)

aier_apply_project_options(test_frame_capture)

target_include_directories(test_frame_capture PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/extern/glad/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(test_frame_capture PRIVATE OpenGL::GL ${OpenCV_LIBS})
target_compile_definitions(test_frame_capture PRIVATE AIER_HAS_OPENCV=1)

if(TARGET glfw)
    target_link_libraries(test_frame_capture PRIVATE glfw)
elseif(TARGET glfw3)
    target_link_libraries(test_frame_capture PRIVATE glfw3)
elseif(TARGET GLFW::GLFW)
    target_link_libraries(test_frame_capture PRIVATE GLFW::GLFW)
endif()

add_test(NAME FrameCaptureConversion COMMAND test_frame_capture)
