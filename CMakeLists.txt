cmake_minimum_required(VERSION 3.5)
project(main)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)

# Compilation flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -Wall -Wextra ")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -O3 -ffast-math")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)

# Find OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# GLFW Configuration - Only build if not already built
if(NOT TARGET glfw)
    # Download and build GLFW only once
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE  # Only fetch the latest commit for this tag
    )
    
    # Set these BEFORE FetchContent_MakeAvailable
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(glfw)
    
    # Mark GLFW as excluded from 'all' target
    set_target_properties(glfw PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

# Find Freetype
find_package(Freetype REQUIRED)

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

# Gather source files
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS src/*.cpp src/*.c src/*.h src/*.cc)

# Create executable
add_executable(${PROJECT_NAME} ${SRC_FILES} ${IMGUI_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    src
    /opt/homebrew/Cellar/glm/1.0.1/include/
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${FREETYPE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    glfw
    ${FREETYPE_LIBRARIES}
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
