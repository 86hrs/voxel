// model.h
#pragma once
#include <string>
#include <vector>
#include "glad.h"
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLuint diffuseTexture = 0;
};

class Model {
  public:
    Model(const std::string &filename);
    ~Model() = default;

    void render();
    std::string filename;

    Assimp::Importer importer;
    aiScene *scene = nullptr;

    std::vector<Mesh> meshes;

    void load_scene();
    void build_meshes();
    void load_embedded_texture(const aiTexture *texture, GLuint &texID);
};

