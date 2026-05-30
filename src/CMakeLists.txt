find_package(OpenGL REQUIRED)

include(${PROJECT_SOURCE_DIR}/cmake/ProjectOptions.cmake)

file(GLOB_RECURSE AIER_SRC CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)

add_executable(${PROJECT_NAME}
    ${AIER_SRC}
    ${PROJECT_SOURCE_DIR}/extern/glad/src/glad.c
)

aier_apply_project_options(${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME} PRIVATE
    AIER_PROJECT_ROOT="${PROJECT_SOURCE_DIR}"
)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
    OpenGL::GL
    glm::glm
    assimp::assimp
)

if(TARGET glfw)
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw)
elseif(TARGET glfw3)
    target_link_libraries(${PROJECT_NAME} PRIVATE glfw3)
elseif(TARGET GLFW::GLFW)
    target_link_libraries(${PROJECT_NAME} PRIVATE GLFW::GLFW)
endif()

if(AIER_ENABLE_IMGUI)
    target_link_libraries(${PROJECT_NAME} PRIVATE imgui_lib)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AIER_HAS_IMGUI=1)
endif()

if(AIER_ENABLE_OPENCV)
    target_include_directories(${PROJECT_NAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${OpenCV_LIBS})
    target_compile_definitions(${PROJECT_NAME} PRIVATE AIER_HAS_OPENCV=1)
    if(OPENCV_DNN_SUPERRES)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENCV_DNN_SUPERRES})
    endif()
endif()

if(AIER_ENABLE_ONNXRUNTIME)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/extern/onnxruntime/include)
    target_link_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/extern/onnxruntime/lib)
    target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AIER_USE_ONNXRUNTIME=1)
endif()
