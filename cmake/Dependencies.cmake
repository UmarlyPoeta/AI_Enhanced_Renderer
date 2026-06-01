# Dependencies: prefer vendored copies in extern/, fall back to FetchContent.

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# ---------------- GLFW ----------------
find_package(glfw3 3.3 QUIET)
if(glfw3_FOUND)
    if(NOT TARGET glfw AND TARGET glfw3)
        add_library(glfw ALIAS glfw3)
    endif()
else()
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.9
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glfw)
endif()

# ---------------- GLM ----------------
if(EXISTS "${PROJECT_SOURCE_DIR}/extern/glm/glm/glm.hpp")
    if(NOT TARGET glm::glm)
        add_library(glm INTERFACE)
        target_include_directories(glm INTERFACE "${PROJECT_SOURCE_DIR}/extern/glm")
        add_library(glm::glm ALIAS glm)
    endif()
else()
    include(FetchContent)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 0.9.9.8
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(glm)
endif()

# ---------------- ImGui ----------------
if(EXISTS "${PROJECT_SOURCE_DIR}/extern/imgui/imgui.h")
    set(IMGUI_DIR "${PROJECT_SOURCE_DIR}/extern/imgui")
else()
    include(FetchContent)
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.90.1
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(imgui)
    set(IMGUI_DIR "${imgui_SOURCE_DIR}")
endif()

add_library(imgui_lib STATIC
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_lib PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)

if(TARGET glfw)
    target_link_libraries(imgui_lib PUBLIC glfw)
elseif(TARGET glfw3)
    target_link_libraries(imgui_lib PUBLIC glfw3)
elseif(TARGET GLFW::GLFW)
    target_link_libraries(imgui_lib PUBLIC GLFW::GLFW)
endif()
