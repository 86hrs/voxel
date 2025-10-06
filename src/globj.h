#pragma once
#include "glad.h"
#include "obj.h"
#include <GLFW/glfw3.h>

typedef struct {
  GLuint vao;
  GLuint vbo;
  GLuint ebo;
  GLuint texCoordVBO;
  GLuint normalVBO;
  int indexCount;
} GLObject;

void setupGLObject(OBJModel *objModel, GLObject *glModel);
void renderGLObject(GLObject *model); 
void freeGLObject(GLObject *model);
