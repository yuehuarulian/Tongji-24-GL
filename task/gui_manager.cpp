#include "gui_manager.hpp"

GUIManager::GUIManager(GLFWwindow *window, Camera &camera, LightManager &light_manager, ShaderManager &shader_manager)
    : camera(camera), light_manager(light_manager), shader_manager(shader_manager), fov(90.0f), light_intensity(30000.0f)
{
    // 初始化 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    // 初始化 ImGui 后端（GLFW 和 OpenGL3）
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

GUIManager::~GUIManager()
{
    // 清理 ImGui 资源
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUIManager::render()
{
    // 启动新的 ImGui 帧
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 创建一个窗口，用于设置参数
    ImGui::Begin("Camera and Light Controls");

    // FOV 滑动条控件
    if (ImGui::SliderFloat("FOV", &fov, 30.0f, 120.0f))
    {
        camera.set_fov(fov / 180.0f * glm::pi<float>());
    }

    // 光照强度滑动条控件
    if (ImGui::SliderFloat("Light Intensity", &light_intensity, 1.0f, 100000.0f))
    {
        light_manager.set_intensity(light_intensity, shader_manager.get_shader("room_shader"));
    }

    ImGui::End();

    // 渲染 ImGui 绘制数据
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIManager::update()
{
    // 可以在这里更新其他逻辑，例如定时更新或其他状态的更新
}
