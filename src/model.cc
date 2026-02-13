// model.cc
#include "assimp/material.h"
#include "stb_image.h"
#include "model.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <cassert>

Model::Model(const std::string &filename) {
    this->filename = filename;
    this->load_scene();
    this->build_meshes();
}

void Model::load_scene() {
    const aiScene *scene = importer.ReadFile(
        filename, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices |
                      aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
                      aiProcess_FlipUVs | aiProcess_LimitBoneWeights |
                      aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
        !scene->mRootNode) {
        std::cerr << "ASSIMP ERROR: " << importer.GetErrorString() << "\n";
        this->scene = nullptr;
        return;
    }

    this->scene = const_cast<aiScene *>(scene);
}

void Model::load_embedded_texture(const aiTexture *texture, GLuint &texID) {
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned char *image = nullptr;
    int width = 0, height = 0, channels = 0;

    if (texture->mHeight == 0 && texture->pcData) {
        image = stbi_load_from_memory(
            reinterpret_cast<unsigned char *>(texture->pcData), texture->mWidth,
            &width, &height, &channels, 0);
    } else if (texture->mHeight > 0 && texture->pcData) {
        // Uncompressed RGBA texture
        width = texture->mWidth;
        height = texture->mHeight;
        channels = 4;
        image = new unsigned char[width * height * 4];
        for (int i = 0; i < width * height; ++i) {
            aiTexel texel = reinterpret_cast<aiTexel *>(texture->pcData)[i];
            image[i * 4 + 0] = texel.r;
            image[i * 4 + 1] = texel.g;
            image[i * 4 + 2] = texel.b;
            image[i * 4 + 3] = texel.a;
        }
    }

    if (!image) {
        std::cerr << "Failed to load embedded texture\n";
        glBindTexture(GL_TEXTURE_2D, 0);
        return;
    }

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    GLenum internalFormat = (channels == 4) ? GL_RGBA8 : GL_RGB8;

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format,
                 GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (texture->mHeight > 0)
        delete[] image; 
    else
        stbi_image_free(image);

    glBindTexture(GL_TEXTURE_2D, 0);
}

void Model::build_meshes() {
    if (!scene)
        return;
    meshes.clear();

    const aiVector3D aiZero(0, 0, 0);

    for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
        aiMesh *mesh = scene->mMeshes[m];
        aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

        Mesh newMesh;
        newMesh.vertices.reserve(mesh->mNumVertices);
        newMesh.indices.reserve(mesh->mNumFaces * 3);

        // Vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
            const aiVector3D *pos = &mesh->mVertices[i];
            const aiVector3D *normal = &mesh->mNormals[i];
            const aiVector3D *texCoord = mesh->HasTextureCoords(0)
                                             ? &mesh->mTextureCoords[0][i]
                                             : &aiZero;

            Vertex v;
            v.position = glm::vec3(pos->x, pos->y, pos->z);
            v.normal = glm::vec3(normal->x, normal->y, normal->z);
            v.texCoords = glm::vec2(texCoord->x, texCoord->y);

            newMesh.vertices.push_back(v);
        }

        unsigned int baseVertex = 0;
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
            const aiFace &face = mesh->mFaces[i];
            assert(face.mNumIndices == 3);
            for (unsigned int j = 0; j < face.mNumIndices; ++j) {
                newMesh.indices.push_back(face.mIndices[j] + baseVertex);
            }
        }

        if (material->GetTextureCount(aiTextureType_BASE_COLOR) > 0) {
            aiString str;
            material->GetTexture(aiTextureType_BASE_COLOR, 0, &str);
            const aiTexture *tex = scene->GetEmbeddedTexture(str.C_Str());
            if (tex) {
                load_embedded_texture(tex, newMesh.diffuseTexture);
            }
        }

        glGenVertexArrays(1, &newMesh.vao);
        glGenBuffers(1, &newMesh.vbo);
        glGenBuffers(1, &newMesh.ebo);

        glBindVertexArray(newMesh.vao);
        glBindBuffer(GL_ARRAY_BUFFER, newMesh.vbo);
        glBufferData(GL_ARRAY_BUFFER, newMesh.vertices.size() * sizeof(Vertex),
                     newMesh.vertices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void *)offsetof(Vertex, position));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void *)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                              (void *)offsetof(Vertex, normal));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, newMesh.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     newMesh.indices.size() * sizeof(uint32_t),
                     newMesh.indices.data(), GL_STATIC_DRAW);
        glBindVertexArray(0);

        meshes.push_back(newMesh);
    }

    std::cout << "Loaded " << meshes.size() << " meshes.\n";
}

void Model::render() {
    GLint prevTex;
    glGetIntegerv(GL_ACTIVE_TEXTURE, &prevTex);
    for (auto &mesh : meshes) {
        if (mesh.diffuseTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mesh.diffuseTexture);
        }

        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    }

    glBindTexture(GL_TEXTURE_2D, prevTex);
    glBindVertexArray(0);
}

