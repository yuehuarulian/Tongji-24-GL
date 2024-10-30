#ifndef ERROR_HPP
#define ERROR_HPP
#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
void checkGLError(const char *functionName)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error in " << functionName << ": " << err << std::endl;
    }
}
#endif