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

void checkOpenGLError(const std::string &message)
{
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (err)
        {
        case GL_INVALID_ENUM:
            error = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "GL_OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        default:
            error = "Unknown Error";
            break;
        }
        std::cerr << "OpenGL Error [" << error << "]: " << message << std::endl;
    }
}

#endif