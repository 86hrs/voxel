// engine.hpp
#pragma once
#include "camera.h"
#include "chunker.h"
#include "glad.h"
#include "hud.h"
#include "model.h"
#include "shader.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <unordered_map>

struct Engine {
  private:
    bool isRunning;
    GLFWwindow *window;
    int width;
    int height;
    const char *title;
    const char *vendor;
    const char *renderer;

    bool wireframe = false;

    std::unique_ptr<ChunkManager> chunker;
    std::unique_ptr<Shader> shader;
    std::unique_ptr<Shader> hud_shader;
    std::unique_ptr<Shader> obj_shader;
    std::unique_ptr<Hud> hud;
    std::unordered_map<Block::BlockTexture, unsigned int> textures;

    Model *diablo;

    void input();
    void update();
    void render();
    void clean();

    void render_imgui();

    void setup_opengl();
    void setup_imgui();
    void setup_objects();
    void setup_shaders();
    void load_textures(const std::string &path,
                       Block::BlockTexture textureType);
    void load_scene(const std::string &filename);

    std::unique_ptr<Camera> camera;

    bool firstMouse = true;
    float lastX = 800.0f / 2.0;
    float lastY = 600.0 / 2.0;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;
    float previousSecond = glfwGetTime();
    int frameCount = 0;
    int fps = 0;
    bool b_vsync = false;

    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);
    glm::mat4 model = glm::mat4(1.0f);

    static void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
        Engine *instance = (Engine *)(glfwGetWindowUserPointer(window));

        if (instance)
            instance->handle_mouse_callback(xpos, ypos);
    }
    static void scroll_callback(GLFWwindow *window, double xoffset,
                                double yoffset) {
        Engine *instance = (Engine *)(glfwGetWindowUserPointer(window));
        if (instance)
            instance->handle_scroll_callback(xoffset, yoffset);
    }

    void handle_mouse_callback(double xpos, double ypos);
    void handle_scroll_callback(double xoffset, double yoffset);

  public:
    Engine(int windowWidth = 800, int windowHeight = 600,
           const char *windowTitle = "Learning...") {
        this->isRunning = false;
        this->window = nullptr;
        this->width = windowWidth;
        this->height = windowHeight;
        this->title = windowTitle;
    }

    ~Engine();

    void init();
    void run();
    bool running() const { return isRunning; }
};
