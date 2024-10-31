#ifndef LOAD_BMP_HPP
#define LOAD_BMP_HPP
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

GLuint load_image(const char *imagepath);

#endif