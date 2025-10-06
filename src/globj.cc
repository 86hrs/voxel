#include "glad.h"
#include "obj.h"
#include "globj.h"
#include <cstring>
#include <GLFW/glfw3.h>

void setupGLObject(OBJModel *objModel, GLObject *glModel) {
    glGenVertexArrays(1, &glModel->vao);
    glGenBuffers(1, &glModel->vbo);
    glGenBuffers(1, &glModel->ebo);

    if (objModel->texCoordCount > 0)
        glGenBuffers(1, &glModel->texCoordVBO);
    if (objModel->normalCount > 0)
        glGenBuffers(1, &glModel->normalVBO);

    glBindVertexArray(glModel->vao);

    // Vertex positions
    glBindBuffer(GL_ARRAY_BUFFER, glModel->vbo);
    glBufferData(GL_ARRAY_BUFFER, objModel->vertexCount * 3 * sizeof(float),
                 objModel->vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);

    // Texture coordinates
    if (objModel->texCoordCount > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, glModel->texCoordVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     objModel->texCoordCount * 2 * sizeof(float),
                     objModel->texCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float),
                              (void *)(0 * sizeof(float)));
    }

    // Normals
    if (objModel->normalCount > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, glModel->normalVBO);
        glBufferData(GL_ARRAY_BUFFER, objModel->normalCount * 3 * sizeof(float),
                     objModel->normals, GL_STATIC_DRAW);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void *)(0 * sizeof(float)));
    }

    // Reverse the winding
    if (objModel->indices) {
        for (unsigned int i = 0; i < (unsigned int)objModel->indexCount; i += 3) {
            unsigned int temp = objModel->indices[i + 1];
            objModel->indices[i + 1] = objModel->indices[i + 2];
            objModel->indices[i + 2] = temp;
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glModel->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 objModel->indexCount * sizeof(unsigned int), objModel->indices,
                 GL_STATIC_DRAW);

    glBindVertexArray(0);
    glModel->indexCount = objModel->indexCount;
}

void renderGLObject(GLObject *model) {
    glBindVertexArray(model->vao);
    glDrawElements(GL_TRIANGLES, model->indexCount, GL_UNSIGNED_INT,0);
    glBindVertexArray(0);
}

void freeGLObject(GLObject *model) {
    glDeleteVertexArrays(1, &model->vao);
    glDeleteBuffers(1, &model->vbo);
    glDeleteBuffers(1, &model->ebo);

    if (model->texCoordVBO)
        glDeleteBuffers(1, &model->texCoordVBO);
    if (model->normalVBO)
        glDeleteBuffers(1, &model->normalVBO);

    memset(model, 0, sizeof(GLObject));
}
