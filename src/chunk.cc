// chunk.cc
#include "glad.h"
#include "chunk.h"

Chunk::Chunk(int x, int z) {
    this->chunk_position.x = x;
    this->chunk_position.y = z;
    this->noise = std::make_unique<FastNoiseLite>();
    this->noise->SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2S);

    glGenVertexArrays(1, &this->vao);
    glGenBuffers(1, &this->vbo);
    glGenBuffers(1, &this->vbo_type);
    glGenBuffers(1, &this->ebo);

    this->generate_terrain();
    this->build_mesh();
    this->upload_to_gpu();
}
Chunk::~Chunk() {
    glDeleteVertexArrays(1, &this->vao);
    glDeleteBuffers(1, &this->vbo);
    glDeleteBuffers(1, &this->vbo_type);
    glDeleteBuffers(1, &this->ebo);
}
void Chunk::generate_terrain() {
    this->noise->SetFrequency(0.01f);

    float biome_noise = this->noise->GetNoise(this->chunk_position.x * 5.0f,
                                              this->chunk_position.y * 5.0f);
    Biome biome = Biome::Plains;
    if (biome_noise > 0.3f) {
        biome = Biome::Desert;
    }

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            float worldX = this->chunk_position.x * CHUNK_SIZE + x;
            float worldZ = this->chunk_position.y * CHUNK_SIZE + z;

            float noiseValue = this->noise->GetNoise(worldX, worldZ);
            float temp = (noiseValue + 1.0f) * ((float)CHUNK_SIZE / 4);
            int sectionHeight = (int)temp;
            sectionHeight = glm::clamp(sectionHeight, 0, CHUNK_SIZE - 1);

            for (int y = 0; y < CHUNK_SIZE; y++) {
                if (y <= sectionHeight) {
                    switch (biome) {
                    case Biome::Plains:
                        this->blocks[x][y][z].type = Block::BlockType::Dirt;
                        this->blocks[x][y][z].top =
                            Block::BlockTexture::GRASS_TOP;
                        this->blocks[x][y][z].bottom =
                            Block::BlockTexture::GRASS_BOTTOM;
                        this->blocks[x][y][z].side =
                            Block::BlockTexture::GRASS_SIDE;

                        if (y == sectionHeight)
                            this->blocks[x][y][z].type =
                                Block::BlockType::Grass;
                        break;
                    case Biome::Desert:
                        this->blocks[x][y][z].type = Block::BlockType::Sand;
                        this->blocks[x][y][z].top = Block::BlockTexture::SAND;
                        this->blocks[x][y][z].bottom =
                            Block::BlockTexture::SAND;
                        this->blocks[x][y][z].side = Block::BlockTexture::SAND;
                        break;

                    default:
                        break;
                    }
                } else {
                    this->blocks[x][y][z].type = Block::BlockType::Air;
                }
            }
        }
    }

    if (biome == Biome::Desert)
        return;

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            float worldX = this->chunk_position.x * CHUNK_SIZE + x;
            float worldZ = this->chunk_position.y * CHUNK_SIZE + z;

            int y = CHUNK_SIZE - 1;
            while (y >= 0 &&
                   this->blocks[x][y][z].type == Block::BlockType::Air) {
                y--;
            }

            if (y >= 0 &&
                this->blocks[x][y][z].type == Block::BlockType::Grass) {
                this->blocks[x][y][z].type = Block::BlockType::Wood;
                float treeValue =
                    this->noise->GetNoise(worldX * 10, worldZ * 10);
                if (x >= 2 and x < CHUNK_SIZE - 2 and z >= 2 and
                    z < CHUNK_SIZE - 2) {
                    if (treeValue > 0.93f and y < CHUNK_SIZE - 6) {
                        generate_tree(x, y + 1, z);
                        this->chunk_trees += 1;
                    }
                }
            }
        }
    }
}
void Chunk::generate_tree(int x, int y, int z) {
    int requiredSpace = 5;
    if (x < requiredSpace || x >= CHUNK_SIZE - requiredSpace ||
        z < requiredSpace || z >= CHUNK_SIZE - requiredSpace) {
        return;
    }

    int treeHeight = 5 + (rand() % 2);

    for (int dy = 0; dy < treeHeight && y + dy < CHUNK_SIZE; dy++) {
        this->blocks[x][y + dy][z].type = Block::BlockType::Wood;
        this->blocks[x][y + dy][z].top = Block::BlockTexture::WOOD_TOP;
        this->blocks[x][y + dy][z].bottom = Block::BlockTexture::WOOD_TOP;
        this->blocks[x][y + dy][z].side = Block::BlockTexture::WOOD;
    }

    int leafStartY = y + treeHeight - 3;
    int leafEndY = y + treeHeight + 1;

    for (int ly = leafStartY; ly <= leafEndY && ly < CHUNK_SIZE; ly++) {
        int radius = (ly == leafEndY) ? 1 : 2;

        for (int lx = x - radius; lx <= x + radius; lx++) {
            for (int lz = z - radius; lz <= z + radius; lz++) {
                if (radius == 2 && (abs(lx - x) == 2 && abs(lz - z) == 2)) {
                    continue;
                }
                this->blocks[lx][ly][lz].type = Block::BlockType::Leaf;
                this->blocks[lx][ly][lz].top = Block::BlockTexture::LEAF;
                this->blocks[lx][ly][lz].bottom = Block::BlockTexture::LEAF;
                this->blocks[lx][ly][lz].side = Block::BlockTexture::LEAF;
            }
        }
    }
}
void Chunk::render() {
    glBindVertexArray(this->vao);
    glDrawElements(GL_TRIANGLES, this->index_data.size(), GL_UNSIGNED_INT, 0);
}
void Chunk::build_mesh() {
    this->vertex_data.clear();
    this->index_data.clear();
    this->texture_index_data.clear();

    vertex_data.reserve(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6 * 8);
    texture_index_data.reserve(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 6);

    int idx = 0;
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                if (this->blocks[x][y][z].type == Block::BlockType::Air)
                    continue;

                bool occluded[6] = {false, false, false, false, false, false};

                if (x > 0 &&
                    this->blocks[x - 1][y][z].type != Block::BlockType::Air)
                    occluded[4] = true;
                if (x < CHUNK_SIZE - 1 &&
                    this->blocks[x + 1][y][z].type != Block::BlockType::Air)
                    occluded[5] = true;
                if (y > 0 &&
                    this->blocks[x][y - 1][z].type != Block::BlockType::Air)
                    occluded[1] = true;
                if (y < CHUNK_SIZE - 1 &&
                    this->blocks[x][y + 1][z].type != Block::BlockType::Air)
                    occluded[0] = true;
                if (z > 0 &&
                    this->blocks[x][y][z - 1].type != Block::BlockType::Air)
                    occluded[3] = true;
                if (z < CHUNK_SIZE - 1 &&
                    this->blocks[x][y][z + 1].type != Block::BlockType::Air)
                    occluded[2] = true;

                if (occluded[0] && occluded[1] && occluded[2] && occluded[3] &&
                    occluded[4] && occluded[5]) {
                    continue;
                }
                add_block_to_mesh(x, y, z, idx, occluded);
            }
        }
    }
}

void Chunk::add_block_to_mesh(int x, int y, int z, int &index,
                              const bool occluded[6]) {
    for (int face = 0; face < 6; face++) {
        if (occluded[face])
            continue;

        int texIndex;

        switch (face) {
        case 0:
            texIndex = this->blocks[x][y][z].top;
            break;
        case 1:
            texIndex = this->blocks[x][y][z].bottom;
            break;
        default:
            texIndex = this->blocks[x][y][z].side;
            break;
        }

        for (int vertex = 0; vertex < 4; ++vertex) {
            int i = vertex * 3;
            int ti = vertex * 2;

            float vx = this->face_verticies[face][i] + x;
            float vy = this->face_verticies[face][i + 1] + y;
            float vz = this->face_verticies[face][i + 2] + z;

            // Position (3 floats)
            this->vertex_data.push_back(vx);
            this->vertex_data.push_back(vy);
            this->vertex_data.push_back(vz);

            // Texture coordinates (2 floats)
            this->vertex_data.push_back(this->face_uvs[face][ti]);
            this->vertex_data.push_back(this->face_uvs[face][ti + 1]);

            // Normal (3 floats)
            this->vertex_data.push_back(this->face_normals[face][0]);
            this->vertex_data.push_back(this->face_normals[face][1]);
            this->vertex_data.push_back(this->face_normals[face][2]);

            // Texture Index (1 int)
            this->texture_index_data.push_back(texIndex);
        }

        this->index_data.insert(
            this->index_data.end(),
            {(unsigned int)(index), (unsigned int)(index + 1),
             (unsigned int)(index + 2), (unsigned int)(index),
             (unsigned int)(index + 2), (unsigned int)(index + 3)});

        index += 4;
        this->chunk_verticies += 4;
        this->chunk_triangles += 2;
    }
}

void Chunk::modify_block(int x, int y, int z, Block::BlockType type) {
    if (x < 0 || x >= CHUNK_SIZE || y < 0 || y >= CHUNK_SIZE || z < 0 ||
        z >= CHUNK_SIZE)
        return;

    Block &block = this->blocks[x][y][z];

    // Remove
    if (type == Block::BlockType::Air && block.type != Block::BlockType::Air) {
        block.type = Block::BlockType::Air;
    }

    // Add
    if (type == Block::BlockType::Dirt) {
        block.type = Block::BlockType::Dirt;
        block.top = Block::BlockTexture::GRASS_TOP;
        block.bottom = Block::BlockTexture::GRASS_BOTTOM;
        block.side = Block::BlockTexture::GRASS_SIDE;
    }
    this->build_mesh();
    this->upload_to_gpu();
}

void Chunk::upload_to_gpu() {
    glBindVertexArray(this->vao);
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
    glBufferData(GL_ARRAY_BUFFER, this->vertex_data.size() * sizeof(float),
                 this->vertex_data.data(), GL_DYNAMIC_DRAW);

    // Vertex positions (attribute 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)0);

    // Texture coordinates (attribute 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(3 * sizeof(float)));

    // Normal vectors (attribute 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                          (void *)(5 * sizeof(float)));

    // Type data (attribute 3)
    glBindBuffer(GL_ARRAY_BUFFER, this->vbo_type);
    glBufferData(GL_ARRAY_BUFFER, this->texture_index_data.size() * sizeof(int),
                 this->texture_index_data.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_INT, 0, (void *)0);

    // Element indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 this->index_data.size() * sizeof(unsigned int),
                 this->index_data.data(), GL_DYNAMIC_DRAW);
}
