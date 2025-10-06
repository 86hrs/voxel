// chunk.h
#pragma once
#include "FastNoiseLite.h"
#include "block.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include <memory>

struct Chunk {
    static constexpr int CHUNK_SIZE = 32;

    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    uint vao, vbo, vbo_type, ebo;
    std::vector<float> vertex_data;
    std::vector<unsigned int> index_data;
    std::vector<int> texture_index_data;
    glm::vec2 chunk_position;

    int chunk_verticies = 0;
    int chunk_triangles = 0;
    int chunk_trees = 0;

    std::unique_ptr<FastNoiseLite> noise;

    static constexpr float face_verticies[6][12] = {
        {0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1}, // Top (y+1)
        {0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0}, // Bottom (y)
        {0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1}, // Front (z+1)
        {0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0}, // Back (z)
        {0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1}, // Left (x)
        {1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0}  // Right (x+1)
    };

    static constexpr float face_normals[6][3] = {
        {0, 1, 0},  // Top
        {0, -1, 0}, // Bottom
        {0, 0, 1},  // Front
        {0, 0, -1}, // Back
        {-1, 0, 0}, // Left
        {1, 0, 0}   // Right
    };

    static constexpr float face_uvs[6][8] = {
        {0, 0, 1, 0, 1, 1, 0, 1}, // Top face
        {0, 0, 0, 1, 1, 1, 1, 0}, // Bottom face
        {0, 0, 0, 1, 1, 1, 1, 0}, // Front face
        {0, 0, 1, 0, 1, 1, 0, 1}, // Back face
        {0, 0, 0, 1, 1, 1, 1, 0}, // Left face
        {0, 0, 1, 0, 1, 1, 0, 1}  // Right face
    };

    enum class Biome { Plains, Forest, Desert, Ocean };

    Chunk(int x, int z);
    ~Chunk();

    void generate_terrain();
    void generate_tree(int x, int y, int z);
    void render();
    void build_mesh();
    void add_block_to_mesh(int x, int y, int z, int &index,
                           const bool occluded[6]);
    void modify_block(int x, int y, int z, Block::BlockType type);
    void upload_to_gpu();
};
