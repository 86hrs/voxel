cmake_minimum_required(VERSION 3.5)
project(main)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(assimp REQUIRED)

# ImGui setup
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp src/*.c src/*.h src/*.cc)

add_executable(${PROJECT_NAME} ${SRC_FILES} ${IMGUI_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    src
    /opt/homebrew/Cellar/glm/1.0.3/include/
    /opt/homebrew/Cellar/glfw/3.4/include/
    /opt/homebrew/Cellar/assimp/6.0.4_1/include/
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${FREETYPE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    ${FREETYPE_LIBRARIES}
    glfw
    assimp::assimp
)

set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_options(${PROJECT_NAME} PRIVATE
    -O3
    -ffast-math
    -march=native
    -flto
    -Wall
    -Wextra
    -fno-rtti             # If you don’t use dynamic_cast or typeid
)

# Shader and resource files setup
file(GLOB BLOCK_TEXTURES "${CMAKE_SOURCE_DIR}/src/block/*.png")
set(SHADER_FILES
    ${CMAKE_SOURCE_DIR}/src/vertex.glsl
    ${CMAKE_SOURCE_DIR}/src/fragment.glsl
    ${CMAKE_SOURCE_DIR}/src/hud_fragment.glsl
    ${CMAKE_SOURCE_DIR}/src/hud_vertex.glsl
    ${CMAKE_SOURCE_DIR}/src/obj_vertex.glsl
    ${CMAKE_SOURCE_DIR}/src/obj_fragment.glsl
    ${CMAKE_SOURCE_DIR}/src/diablo.obj
    ${CMAKE_SOURCE_DIR}/src/doom.glb
    ${CMAKE_SOURCE_DIR}/src/930.glb
    ${BLOCK_TEXTURES}
)

# Custom target for copying resources
add_custom_target(copy_resources ALL
    COMMENT "Copying shaders and resources to output directory"
)

foreach(RESOURCE_FILE ${SHADER_FILES})
    get_filename_component(RESOURCE_FILENAME ${RESOURCE_FILE} NAME)
    add_custom_command(
        TARGET copy_resources
        COMMAND ${CMAKE_COMMAND} -E copy
        ${RESOURCE_FILE}
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${RESOURCE_FILENAME}
    )
endforeach()

add_dependencies(${PROJECT_NAME} copy_resources)
