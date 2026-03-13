# Shared compiler and target settings for local modules.
function(aier_apply_project_options target_name)
    target_compile_features(${target_name} PRIVATE cxx_std_20)

    if(MSVC)
        target_compile_options(${target_name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:/W4 /permissive->)
    else()
        target_compile_options(${target_name} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -Wpedantic>)
    endif()

    target_include_directories(${target_name}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/src
            ${PROJECT_SOURCE_DIR}/include
            ${PROJECT_SOURCE_DIR}/extern/glad/include
            ${PROJECT_SOURCE_DIR}/extern
    )
endfunction()
