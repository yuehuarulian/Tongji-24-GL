#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "camera_control.hpp"
#include "light_manager.hpp"
#include "shader_manager.hpp"
#include <GLFW/glfw3.h>

class GUIManager
{
public:
    GUIManager(GLFWwindow *window, Camera &camera, LightManager &light_manager, ShaderManager &shader_manager);
    ~GUIManager();

    void render(); // 渲染 ImGui 控件
    void update(); // 更新各个参数

private:
    Camera &camera;
    LightManager &light_manager;
    ShaderManager &shader_manager;

    float fov;             // 视角参数
    float light_intensity; // 光照强度参数
};

#endif
