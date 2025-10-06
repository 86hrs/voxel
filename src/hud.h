#pragma once
#include "glad.h"

struct Hud {
    float verticies[12] = {
        -0.01f, 0.0f,    0.0f, // Left point
        0.01f,  0.0f,    0.0f, // Right point

        0.0f,   -0.015f, 0.0f, // Bottom point
        0.0f,   0.015f,  0.0f  // Top point
    };
    unsigned int indices[4] = {0, 1, 2, 3};
    unsigned int vao, vbo, ebo;

    Hud() { setup(); }

    ~Hud() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }
    void setup() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verticies), verticies,
                     GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
                     GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                              (void *)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
    void render() {
        glLineWidth(3.0f);
        glBindVertexArray(vao);
        glDrawElements(GL_LINES, 4, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
    void cleanup() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
    }
};
