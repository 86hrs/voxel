// engine.cc
#include <iostream>
#include "engine.h"
#include "camera.h"
#include "glm/ext/matrix_transform.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "model.h"
#include "shader.hpp"
#include "textures.h"
#include <cassert>

void Engine::handle_mouse_callback(double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    this->camera->ProcessMouseMovement(xoffset, yoffset);
}
void Engine::handle_scroll_callback(double xoffset, double yoffset) {
    (void)xoffset;
    this->camera->ProcessMouseScroll((float)yoffset);
}
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    (void)window;
    glViewport(0, 0, width, height);
}

void debug_message_callback(GLenum source, GLenum type, GLuint id,
                            GLenum severity, GLsizei length,
                            const GLchar *message, const void *userParam) {
    (void)length;
    (void)userParam;

    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::cout << "---------------\n";
    std::cout << "Debug message (" << id << "): " << message << "\n";

    switch (source) {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "Source: Other";
        break;
    }
    std::cout << "\n";

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behavior";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behavior";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other";
        break;
    }
    std::cout << "\n";

    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << "\n\n";
}
void Engine::run() {
    while (!glfwWindowShouldClose(this->window) && this->running()) {
        this->input();
        this->update();
        this->render();
    }
}
void Engine::init() {
    this->setup_opengl();
    this->setup_imgui();
    this->setup_shaders();
    this->setup_objects();
    this->isRunning = true;
}
void Engine::setup_opengl() {
    // GLFW
    assert(glfwInit() && "GLFW3 did not initialize");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    this->window =
        glfwCreateWindow(this->width, this->height, this->title, NULL, NULL);
    assert(this->window && "Window did not create");
    glfwSetWindowUserPointer(this->window, this);

    glfwMakeContextCurrent(this->window);

    glfwSwapInterval(0);

    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(this->window, framebuffer_size_callback);
    glfwSetCursorPosCallback(this->window, this->mouse_callback);
    glfwSetScrollCallback(this->window, this->scroll_callback);

    // GLAD
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) &&
           "Failed to initialize GLAD\n");

    int fb_w, fb_h;
    glfwGetFramebufferSize(window, &fb_w, &fb_h);
    glViewport(0, 0, fb_w, fb_h);

    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_BACK);

    this->vendor = (const char *)glGetString(GL_VENDOR);
    this->renderer = (const char *)glGetString(GL_RENDERER);

    GLint flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
        std::cout << "Debug context active!\n";
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(debug_message_callback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE,
                              GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
                              GL_TRUE);
    } else {
        std::cout << "Debug context NOT active!\n";
    }
}
void Engine::setup_imgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410 core");
}
void Engine::setup_shaders() {
    this->shader = std::make_unique<Shader>("vertex.glsl", "fragment.glsl");
    this->hud_shader =
        std::make_unique<Shader>("hud_vertex.glsl", "hud_fragment.glsl");
    this->obj_shader =
        std::make_unique<Shader>("obj_vertex.glsl", "obj_fragment.glsl");
}
void Engine::setup_objects() {
    this->diablo = new Model("doom.glb");
    this->load_textures("grass_block_top.png", Block::BlockTexture::GRASS_TOP);
    this->load_textures("dirt.png", Block::BlockTexture::GRASS_BOTTOM);
    this->load_textures("spruce_log.png", Block::BlockTexture::WOOD);
    this->load_textures("oak_leaves.png", Block::BlockTexture::LEAF);
    this->load_textures("sand.png", Block::BlockTexture::SAND);
    this->load_textures("spruce_log_top.png", Block::BlockTexture::WOOD_TOP);
    this->load_textures("grass_block_side.png",
                        Block::BlockTexture::GRASS_SIDE);
    this->shader->use();

    for (const auto &[textureType, textureID] : this->textures) {
        int textureUnit = (int)(textureType);
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, textureID);
        std::string uniformName =
            "textures[" + std::to_string(textureUnit) + "]";

        this->shader->set_int(uniformName, textureUnit);
    }
    Shader::stop();

    this->chunker = std::make_unique<ChunkManager>(shader.get());
    this->camera = std::make_unique<Camera>(glm::vec3(0.0f, 15.0f, 0.0f));
    this->hud = std::make_unique<Hud>();
}
void Engine::load_textures(const std::string &path,
                           Block::BlockTexture textureID) {
    uint texture = load_textures_from_file(path);
    this->textures[textureID] = texture;
}
void Engine::render() {
    glClearColor(119.0f / 255.0f, 168.0f / 255.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    this->model = glm::identity<glm::mat4>();
    this->shader->use();
    this->shader->set_mat4("view", this->view);
    this->shader->set_mat4("model", this->model);
    this->shader->set_mat4("projection", this->projection);
    this->shader->set_float("time", (float)glfwGetTime());
    // glFrontFace(GL_CW);
    // this->chunker->render();

    this->hud_shader->use();
    this->hud->render();

    this->model = glm::translate(model, glm::vec3{0.0, 20.0, -5.0});
    this->model = glm::scale(model, glm::vec3{0.05, 0.05, 0.05});
    this->obj_shader->use();
    this->obj_shader->set_mat4("model", this->model);
    this->obj_shader->set_mat4("view", this->view);
    this->obj_shader->set_mat4("projection", this->projection);
    this->obj_shader->set_float("time", (float)glfwGetTime());
    this->obj_shader->set_int("diffuseMap", 0);
    // glFrontFace(GL_CCW);
    this->diablo->render();
    this->render_imgui();
    glfwSwapBuffers(this->window);
}
void Engine::render_imgui() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Menu");

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    ImGui::Text("Renderer: %s", this->renderer);
    ImGui::Text("Vendor: %s", this->vendor);
    ImGui::Text("FPS: %d", this->fps);
    ImGui::Text("Frame time: %f", ((float)1 / this->fps) * 1000.0f);
    ImGui::Checkbox("Wireframe", &this->wireframe);
    ImGui::SameLine();
    ImGui::Checkbox("V-Sync", &this->b_vsync);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    ImGui::InputFloat("Fov", &this->camera->Zoom, 5, 5);
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    ImGui::InputInt("Render Distance", &this->chunker->render_distance, 1, 20);
    ImGui::Text("Loaded chunks: %lu", this->chunker->chunks.size());
    ImGui::Text("Trees: %d", this->chunker->total_trees);
    ImGui::Text("Verticies: %d", this->chunker->total_verticies);
    ImGui::Text("Triangles: %d", this->chunker->total_triangles);
    ImGui::Text("Camera Position:");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "X:%.2f Y:%.2f Z:%.2f",
                       this->camera->Position.x, this->camera->Position.y,
                       this->camera->Position.z);
    ImGui::End();

    ImGui::Render();
    auto ImGuiImplOpenGL3RenderDrawData = ImGui_ImplOpenGL3_RenderDrawData;
    ImGuiImplOpenGL3RenderDrawData(ImGui::GetDrawData());
}
static bool prevLeftMousePressed = false;
static bool prevRightMousePressed = false;

void Engine::input() {
    this->isRunning = glfwGetKey(this->window, GLFW_KEY_ESCAPE) == GLFW_PRESS
                          ? false
                          : this->running();

    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        this->camera->ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        this->camera->ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        this->camera->ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        this->camera->ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        this->camera->ProcessKeyboard(DOWN, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        this->camera->ProcessKeyboard(UP, deltaTime);

    bool leftMousePressed =
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    bool rightMousePressed =
        glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;

    if ((!prevLeftMousePressed && leftMousePressed) or
        (!prevRightMousePressed && rightMousePressed)) {
        glm::vec3 rayStart = this->camera->Position;
        glm::vec3 rayDirection = glm::normalize(this->camera->Front);

        for (float step = 1; step < 20.0f; step += 0.5f) {
            glm::vec3 raytrace = rayStart + rayDirection * step;

            int chunkX = floor(raytrace.x / Chunk::CHUNK_SIZE);
            int chunkZ = floor(raytrace.z / Chunk::CHUNK_SIZE);

            int blockX = floor(raytrace.x) - chunkX * Chunk::CHUNK_SIZE;
            int blockY = floor(raytrace.y);
            int blockZ = floor(raytrace.z) - chunkZ * Chunk::CHUNK_SIZE;

            if (blockX < 0 || blockX >= Chunk::CHUNK_SIZE || blockY < 0 ||
                blockY >= Chunk::CHUNK_SIZE || blockZ < 0 ||
                blockZ >= Chunk::CHUNK_SIZE) {
                continue;
            }

            auto chunkKey = this->chunker->get_chunk_key(chunkX, chunkZ);
            if (this->chunker->chunks.count(chunkKey)) {
                auto &chunk = this->chunker->chunks[chunkKey];
                auto &block = chunk->blocks[blockX][blockY][blockZ];

                // Remove
                if (!prevLeftMousePressed and leftMousePressed and
                    block.type != Block::BlockType::Air) {
                    chunk->modify_block(blockX, blockY, blockZ,
                                        Block::BlockType::Air);
                    break;
                }

                // Add
                if (!prevRightMousePressed and rightMousePressed and step > 2) {
                    // +1 to place on block above
                    chunk->modify_block(blockX, blockY, blockZ,
                                        Block::BlockType::Dirt);
                    break;
                }
            }
        }
    }
    prevLeftMousePressed = leftMousePressed;
    prevRightMousePressed = rightMousePressed;
}
void Engine::update() {
    glfwPollEvents();

    float currentFrame = glfwGetTime();
    this->deltaTime = currentFrame - this->lastFrame;
    this->lastFrame = currentFrame;

    float elapsed_seconds = currentFrame - previousSecond;
    if (elapsed_seconds > 1.0f) {
        previousSecond = currentFrame;
        this->fps = (int)(frameCount / elapsed_seconds);

        frameCount = 0;
    }
    frameCount++;

    this->chunker->update(camera->Position);

    if (this->wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (this->b_vsync)
        glfwSwapInterval(1);
    else
        glfwSwapInterval(0);

    float farP = 1.141 * this->chunker->render_distance * Chunk::CHUNK_SIZE;

    this->projection =
        glm::perspective(glm::radians(this->camera->Zoom),
                         (float)this->width / (float)this->height, 0.1f, farP);

    this->view = this->camera->get_view_matrix();
}
void Engine::clean() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}
Engine::~Engine() { clean(); }
