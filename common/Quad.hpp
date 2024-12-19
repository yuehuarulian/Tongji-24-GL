#pragma once

#include <glad/glad.h>
#include <shader.hpp>

class Quad
{
public:
    Quad();
    void Draw(Shader *);

private:
    GLuint vao;
    GLuint vbo;
};