add_executable(sandbox main.cpp)

if (EMSCRIPTEN)
    set_target_properties(sandbox PROPERTIES SUFFIX ".html")

    target_link_libraries(sandbox PRIVATE
        krnl
        emdawnwebgpu_cpp
        webgpu_glfw
    )

    target_link_options(sandbox PRIVATE
        "-sASYNCIFY=1"
        "-sUSE_GLFW=3"
    )

else()
    target_link_libraries(sandbox PRIVATE
        krnl
        webgpu_dawn
        webgpu_glfw
        glfw
    )
endif()
