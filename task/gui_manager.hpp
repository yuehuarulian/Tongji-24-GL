#ifndef GUI_MANAGER_HPP
#define GUI_MANAGER_HPP

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "camera_control.hpp"
#include "light_manager.hpp"
#include <GLFW/glfw3.h>

class GUIManager
{
public:
    GUIManager(GLFWwindow *window, Camera &camera, LightManager &lightManager);
    ~GUIManager();

    void render(); // 渲染 ImGui 控件
    void update(); // 更新各个参数

private:
    Camera &camera;
    LightManager &lightManager;

    float fov;            // 视角参数
    float lightIntensity; // 光照强度参数
};

#endif
