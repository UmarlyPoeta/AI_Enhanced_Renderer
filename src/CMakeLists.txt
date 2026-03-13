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

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        OpenGL::GL
        glfw
        assimp
        glm::glm
)

if(AIER_ENABLE_ONNXRUNTIME)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/extern/onnxruntime/include)
    target_link_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/extern/onnxruntime/lib)
    target_link_libraries(${PROJECT_NAME} PRIVATE onnxruntime)
    target_compile_definitions(${PROJECT_NAME} PRIVATE AIER_USE_ONNXRUNTIME=1)
endif()
