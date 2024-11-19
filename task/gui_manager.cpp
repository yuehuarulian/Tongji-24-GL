#include "gui_manager.hpp"

GUIManager::GUIManager(GLFWwindow *window, Camera &camera, LightManager &lightManager)
    : camera(camera), lightManager(lightManager), fov(90.0f), lightIntensity(1.0f)
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
    ImGui::Begin("Debug");

    if (ImGui::CollapsingHeader("Camera Controls", ImGuiTreeNodeFlags_DefaultOpen))
    {
        // FOV 滑动条控件
        if (ImGui::SliderFloat("FOV", &fov, 30.0f, 120.0f))
        {
            camera.set_fov(fov / 180.0f * glm::pi<float>()); // 更新摄像机的视场角
        }
        // 摄像机位置滑动条控件
        static glm::vec3 cameraPosition = camera.get_pos(); // 获取摄像机初始位置
        // x 坐标滑动条
        if (ImGui::SliderFloat("Camera Position X", &cameraPosition.x, -1000.0f, 1000.0f))
        {
            camera.set_position(cameraPosition); // 更新摄像机的位置
        }
        // y 坐标滑动条
        if (ImGui::SliderFloat("Camera Position Y", &cameraPosition.y, -1000.0f, 1000.0f))
        {
            camera.set_position(cameraPosition); // 更新摄像机的位置
        }
        // z 坐标滑动条
        if (ImGui::SliderFloat("Camera Position Z", &cameraPosition.z, -1000.0f, 1000.0f))
        {
            camera.set_position(cameraPosition); // 更新摄像机的位置
        }
    }

    // 光照强度滑动条控件
    if (ImGui::SliderFloat("Light Intensity", &lightIntensity, 0.0f, 10.0f))
    {
        // lightManager.setIntensity(lightIntensity);
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
