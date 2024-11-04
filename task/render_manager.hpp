#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include "shader_manager.hpp"
#include "light_manager.hpp"
#include "camera_control.hpp"
#include "scene.hpp"
#include "classic_scene.hpp"
#include <memory>
#include <GLFW/glfw3.h>

class RenderManager
{
public:
    RenderManager(int window_width, int window_height, int frames);
    ~RenderManager();

    void initialize();
    void start_rendering(bool offscreen = false);

private:
    void initialize_GLFW();
    void initialize_framebuffer();
    void update_camera();
    void render_frame(int frame_number);

    GLFWwindow *window;
    std::shared_ptr<Camera> camera;
    ShaderManager shader_manager;
    LightManager light_manager;
    std::unique_ptr<Scene> scene;

    unsigned int fbo, texture_color_buffer, rbo;
    int window_width, window_height, frames;
    bool offscreen;
};

#endif // RENDER_MANAGER_H