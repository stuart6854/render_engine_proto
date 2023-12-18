set(TARGET_NAME RenderEngine)
file(GLOB_RECURSE LIB_HEADERS include/**.hpp)
file(GLOB_RECURSE LIB_SOURCE src/**.cpp)
add_library(${TARGET_NAME} ${LIB_HEADERS} ${LIB_SOURCE})

# target_compile_definitions(${TARGET_NAME} PRIVATE)
set_target_properties(${TARGET_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(${TARGET_NAME} PRIVATE include src)
target_include_directories(${TARGET_NAME} SYSTEM PUBLIC include)

target_link_libraries(${TARGET_NAME} PRIVATE fmt PUBLIC VkMana)