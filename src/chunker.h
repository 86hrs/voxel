// chunker.h
#pragma once
#include "glad.h"
#include "shader.hpp"
#include "chunk.h"
#include <glm/fwd.hpp>
#include <GLFW/glfw3.h>
#include <memory>
#include <string>
#include <unordered_map>

struct ChunkManager {
    std::unordered_map<std::string, std::unique_ptr<Chunk>> chunks;
    Shader *shader;
    int render_distance = 12;

    int total_verticies = 0;
    int total_triangles = 0;
    int total_trees = 0;

    int cameraChunkX;
    int cameraChunkZ;

    ChunkManager(Shader *shader) {
        this->shader = shader;
        chunks.reserve(render_distance * render_distance);
    };
    ~ChunkManager() = default;

    void update(const glm::vec3 &cameraPosition) {
        unload_chunks();
        this->render_distance = glm::clamp(render_distance, 5, 20);
        this->cameraChunkX =
            (int)(std::floor(cameraPosition.x / Chunk::CHUNK_SIZE));
        this->cameraChunkZ =
            (int)(std::floor(cameraPosition.z / Chunk::CHUNK_SIZE));

        for (int x = this->cameraChunkX - render_distance;
             x <= this->cameraChunkX + render_distance; x++) {
            for (int z = cameraChunkZ - render_distance;
                 z <= cameraChunkZ + render_distance; z++) {

                std::string key = get_chunk_key(x, z);
                if (chunks.find(key) == chunks.end())
                    load_chunk(x, z);
            }
        }
    }
    void render() {
        this->shader->use();
        for (const auto &[key, chunk] : this->chunks) {
            glm::mat4 model = glm::translate(
                glm::mat4(1.0f),
                glm::vec3(chunk->chunk_position.x * Chunk::CHUNK_SIZE, 0,
                          chunk->chunk_position.y * Chunk::CHUNK_SIZE));

            this->shader->set_mat4("model", model);
            chunk->render();
        };
    }
    void load_chunk(int x, int z) {
        std::string key = this->get_chunk_key(x, z);
        this->chunks[key] = std::make_unique<Chunk>(x, z);
        auto &chunk = this->chunks[key];

        this->total_triangles += chunk->chunk_triangles;
        this->total_verticies += chunk->chunk_verticies;
        this->total_trees += chunk->chunk_trees;
    }
    void unload_chunks() {
        for (auto it = chunks.begin(); it != chunks.end();) {
            size_t delim_pos = it->first.find(':');
            int chunkX = std::stoi(it->first.substr(0, delim_pos));
            int chunkZ = std::stoi(it->first.substr(delim_pos + 1));

            auto &chunk = it->second;

            bool render_distance_condition =
                std::abs(chunkX - cameraChunkX) > render_distance or
                std::abs(chunkZ - cameraChunkZ) > render_distance;

            if (render_distance_condition) {
                this->total_triangles -= chunk->chunk_triangles;
                this->total_verticies -= chunk->chunk_verticies;
                this->total_trees -= chunk->chunk_trees;
                it = this->chunks.erase(it);
            } else {
                it++;
            }
        }
    }
    std::string get_chunk_key(int x, int z) const {
        return std::to_string(x) + ":" + std::to_string(z);
    }
};
