#ifndef RENDER_MANAGER_H
#define RENDER_MANAGER_H

#include "shader_manager.hpp"
#include "light_manager.hpp"
#include "camera_control.hpp"
#include "scene.hpp"
#include "classic_scene.hpp"
#include <memory>
#include <GLFW/glfw3.h>
#include "skybox.hpp"

class RenderManager
{
public:
    RenderManager(int window_width, int window_height, int frames);
    ~RenderManager();

    void initialize();
    void start_rendering(bool offscreen = false);

private:
    void initialize_GLFW();
    // void initialize_framebuffer();
    void update_camera();
    void update_camera(int current_frame);
    void update_camera(glm::mat4 transform);
    void render_frame(int frame_number);

    GLFWwindow *window;
    Camera camera;
    ShaderManager shader_manager;
    LightManager light_manager;
    std::unique_ptr<Scene> scene;
    std::unique_ptr<Skybox> skybox;

    // unsigned int msaa_fbo, msaa_texture, msaa_rbo;
    // unsigned int resolve_fbo, resolve_texture;
    int window_width, window_height, frames;
    bool offscreen;
    bool dirty = false;
    std::vector<glm::mat4> camera_transforms;
};

#endif // RENDER_MANAGER_H
