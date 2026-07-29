#ifndef SHADER_H
#define SHADER_H

#include <glad/gl.h>

// Compila un shader individual (GL_VERTEX_SHADER o GL_FRAGMENT_SHADER).
// Retorna 0 en caso de error.
GLuint compileShader(GLenum type, const char* source);

// Enlaza vertex + fragment shader en un programa.
// Retorna 0 en caso de error.
GLuint createProgram(const char* vsSrc, const char* fsSrc);

#endif // SHADER_H
