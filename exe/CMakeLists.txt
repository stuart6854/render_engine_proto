set(TARGET_NAME RenderEngine_Exe)
file(GLOB_RECURSE LIB_SOURCE src/**.cpp src/**.hpp)
add_executable(${TARGET_NAME} ${LIB_SOURCE})

# target_compile_definitions(${TARGET_NAME} PRIVATE)
set_target_properties(${TARGET_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(${TARGET_NAME} PRIVATE src)

target_link_libraries(${TARGET_NAME} PRIVATE spdlog glm assimp::assimp glfw RenderEngine)
