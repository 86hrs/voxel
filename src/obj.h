#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>

typedef struct {
    float *vertices;
    float *texCoords;
    float *normals;
    unsigned int *indices;
    int vertexCount;
    int texCoordCount;
    int normalCount;
    int indexCount;

    const char* path;
} OBJModel;

bool loadOBJ(const char *path, OBJModel *model);
void freeOBJModel(OBJModel *model);
void printOBJ(OBJModel* model);
