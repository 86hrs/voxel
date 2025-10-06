#include "obj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

bool loadOBJ(const char *path, OBJModel *model) {
    // Safely initialize model to zero
    memset(model, 0, sizeof(OBJModel));
    model->path = path;

    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << path << std::endl;
        return 0;
    }

    // Temporary storage
    std::vector<float> tempVertices;
    std::vector<float> tempTexCoords;
    std::vector<float> tempNormals;
    std::vector<unsigned int> tempVertexIndices;
    std::vector<unsigned int> tempTexCoordIndices;
    std::vector<unsigned int> tempNormalIndices;

    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace and skip empty/comments
        line.erase(line.find_last_not_of(" \t") + 1);
        if (line.empty() || line[0] == '#')
            continue;

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") { // Vertex position
            float x, y, z, w = 1.0f;
            if (iss >> x >> y >> z) {
                iss >> w; // Optional w component
                tempVertices.insert(tempVertices.end(), {x, y, z, w});
            }
        } else if (prefix == "vt") { // Texture coordinate
            float u, v, w = 0.0f;
            if (iss >> u >> v) {
                iss >> w; // Optional w component
                tempTexCoords.insert(tempTexCoords.end(), {u, v, w});
            }
        } else if (prefix == "vn") { // Normal
            float x, y, z;
            if (iss >> x >> y >> z) {
                tempNormals.insert(tempNormals.end(), {x, y, z});
            }
        } else if (prefix ==
                   "f") { // Face (supports quads and n-gons by triangulating)
            std::vector<std::string> faceVertices;
            std::string vertex;
            while (iss >> vertex) {
                faceVertices.push_back(vertex);
            }

            // Triangulate the face (convert quads/polygons to triangles)
            if (faceVertices.size() >= 3) {
                // Create a triangle fan for polygons with more than 3 vertices
                for (size_t i = 1; i < faceVertices.size() - 1; i++) {
                    // Process each triangle in the fan
                    std::string v1 = faceVertices[0];
                    std::string v2 = faceVertices[i];
                    std::string v3 = faceVertices[i + 1];

                    // Process each vertex of the triangle
                    for (const auto &v : {v1, v2, v3}) {
                        unsigned int vi = 0, vti = 0, vni = 0;
                        int matches = 0;

                        // Handle all possible formats:
                        // v/vt/vn, v//vn, v/vt, or just v
                        if (sscanf(v.c_str(), "%u/%u/%u", &vi, &vti, &vni) ==
                            3) {
                            matches = 3;
                        } else if (sscanf(v.c_str(), "%u//%u", &vi, &vni) ==
                                   2) {
                            matches = 2;
                        } else if (sscanf(v.c_str(), "%u/%u", &vi, &vti) == 2) {
                            matches = 1;
                        } else if (sscanf(v.c_str(), "%u", &vi) == 1) {
                            matches = 0;
                        }

                        // OBJ indices are 1-based
                        if (matches >= 0) {
                            tempVertexIndices.push_back(vi - 1);
                            if (matches >= 1 && vti > 0)
                                tempTexCoordIndices.push_back(vti - 1);
                            if (matches >= 2 && vni > 0)
                                tempNormalIndices.push_back(vni - 1);
                        }
                    }
                }
            }
        }
    }

    // Calculate required sizes
    size_t vertexCount = tempVertexIndices.size();
    bool hasTexCoords = !tempTexCoordIndices.empty() && !tempTexCoords.empty();
    bool hasNormals = !tempNormalIndices.empty() && !tempNormals.empty();

    // Allocate memory
    model->vertices = new float[vertexCount * 3];
    if (hasTexCoords)
        model->texCoords = new float[vertexCount * 2];
    if (hasNormals)
        model->normals = new float[vertexCount * 3];
    model->indices = new unsigned int[vertexCount];

    // Expand the data
    for (size_t i = 0; i < vertexCount; i++) {
        // Vertices
        unsigned int vi = tempVertexIndices[i];
        model->vertices[i * 3] = tempVertices[vi * 4];
        model->vertices[i * 3 + 1] = tempVertices[vi * 4 + 1];
        model->vertices[i * 3 + 2] = tempVertices[vi * 4 + 2];

        // Texture coordinates (if available)
        if (hasTexCoords) {
            unsigned int ti =
                tempTexCoordIndices.empty() ? vi : tempTexCoordIndices[i];
            model->texCoords[i * 2] = tempTexCoords[ti * 3];
            model->texCoords[i * 2 + 1] = tempTexCoords[ti * 3 + 1];
        }

        // Normals (if available)
        if (hasNormals) {
            unsigned int ni =
                tempNormalIndices.empty() ? vi : tempNormalIndices[i];
            model->normals[i * 3] = tempNormals[ni * 3];
            model->normals[i * 3 + 1] = tempNormals[ni * 3 + 1];
            model->normals[i * 3 + 2] = tempNormals[ni * 3 + 2];
        }

        // Indices (just sequential in this flattened representation)
        model->indices[i] = static_cast<unsigned int>(i);
    }

    // Set counts
    model->vertexCount = static_cast<int>(vertexCount);
    model->indexCount =
        static_cast<int>(vertexCount); // All vertices are indexed
    model->texCoordCount = hasTexCoords ? static_cast<int>(vertexCount) : 0;
    model->normalCount = hasNormals ? static_cast<int>(vertexCount) : 0;
    model->path = path;

    return true;
}

void freeOBJModel(OBJModel *model) {
    if (model) {
        delete[] model->vertices;
        delete[] model->texCoords;
        delete[] model->normals;
        delete[] model->indices;
        memset(model, 0, sizeof(OBJModel));
    }
}

void printOBJ(OBJModel *model) {
    std::cout << "Loaded OBJ model  :        " << model->path << "\n";
    std::cout << "Verticies         :        " << model->vertexCount << "\n";
    std::cout << "Indicies          :        " << model->indexCount << "\n";
    std::cout << "TexCoords         :        " << model->texCoordCount << "\n";
    std::cout << "Normals           :        " << model->normalCount << "\n";
}
