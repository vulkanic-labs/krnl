add_executable(examples main.cpp)

if (EMSCRIPTEN)
    set_target_properties(examples PROPERTIES SUFFIX ".html")

    target_link_libraries(examples PRIVATE
        krnl
        emdawnwebgpu_cpp
        webgpu_glfw
    )

    target_link_options(examples PRIVATE
        "-sASYNCIFY=1"
        "-sUSE_GLFW=3"
    )

else()
    target_link_libraries(examples PRIVATE
        krnl
        webgpu_dawn
        webgpu_glfw
        glfw
    )
endif()
