#include "camera_control.hpp"
#include <iostream>

static double scrollYOffset = 0.0;

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    // 这里处理滚轮的滚动
    scrollYOffset = yoffset; // 保存yoffset值以供后续使用
}

Camera::Camera(GLFWwindow *window, float initialfov, glm::vec3 position, float horizontal_angle, float vertical_angle,
               float speed, float mouse_speed)
    : _position(position), _horizontal_angle(horizontal_angle), _vertical_angle(vertical_angle), _speed(speed), _mouse_speed(mouse_speed), _initial_fov(initialfov)
{
    // 设置鼠标到窗口中心
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    glfwSetCursorPos(window, width / 2, height / 2);

    glfwSetScrollCallback(window, scroll_callback);
}

void Camera::compute_matrices_from_inputs(GLFWwindow *window, glm::vec3 center)
{
    // 时间差
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    // 获取鼠标位置并计算偏移量
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    static double lastXpos = width / 2.0, lastYpos = height / 2.0;
    bool mouse_right = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    double xoffset = mouse_right ? xpos - lastXpos : 0;
    double yoffset = mouse_right ? ypos - lastYpos : 0;

    // 更新时间和位置
    lastTime = currentTime;
    lastXpos = xpos;
    lastYpos = ypos;

    // 只识别window内的鼠标移动
    if (xpos < 0 || xpos > width || ypos < 0 || ypos > height)
        return;

    // 计算方向、右向量和上向量
    glm::vec3 direction(
        cos(_vertical_angle) * sin(_horizontal_angle),
        sin(_vertical_angle),
        cos(_vertical_angle) * cos(_horizontal_angle));
    glm::vec3 right = glm::vec3(
        sin(_horizontal_angle - GLM_PI / 2.0f),
        0,
        cos(_horizontal_angle - GLM_PI / 2.0f));
    glm::vec3 up = glm::cross(right, direction);

    // 更新角度
    // _horizontal_angle -= _mouse_speed * deltaTime * float(xoffset);
    // _vertical_angle -= _mouse_speed * deltaTime * float(yoffset);           ///???感觉不对不是在世界坐标系下旋转是camera坐标系下旋转
    glm::mat4 rotation_matrix1 = glm::rotate(glm::mat4(1.0f), _mouse_speed * deltaTime * float(xoffset), up);    // 绕up轴旋转
    glm::mat4 rotation_matrix2 = glm::rotate(glm::mat4(1.0f), _mouse_speed * deltaTime * float(yoffset), right); // 绕right轴旋转
    direction = glm::vec3(rotation_matrix2 * rotation_matrix1 * glm::vec4(direction, 1.0f));
    _vertical_angle = glm::asin(direction.y);
    _vertical_angle = glm::clamp(_vertical_angle, -glm::radians(89.0f), glm::radians(89.0f)); // 限制垂直角度
    _horizontal_angle = glm::atan(direction.x, direction.z);
    right = glm::vec3(sin(_horizontal_angle - GLM_PI / 2.0f), 0, cos(_horizontal_angle - GLM_PI / 2.0f));
    up = glm::cross(right, direction);

    // 处理键盘输入
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // 相机右移，物体左移
        _position += right * deltaTime * _speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // 相机左移，物体右移
        _position -= right * deltaTime * _speed;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // 相机上移，物体下移
        _position += up * deltaTime * _speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // 相机下移，物体上移
        _position -= up * deltaTime * _speed;
    if (scrollYOffset >= 1e-6 || scrollYOffset <= -1e-6) // 滚轮滚动，相机前移后移
        _position += direction * float(scrollYOffset) * _speed * 0.3f;
    scrollYOffset = 0.0;

    // 更新投影和观察矩阵
    // ProjectionMatrix = glm::ortho(-10.0f * float(width) / height, 10.0f * float(width) / height, -10.0f, 10.0f, 0.0f, 100.0f);

    projection = glm::perspective(_initial_fov, float(width) / height, 0.5f, 300.0f);
    view = glm::lookAt(_position, _position + direction, up);
}

glm::vec3 Camera::get_pos()
{
    return _position;
}

void Camera::set_position(glm::vec3 position)
{
    _position = position;
}

glm::vec3 Camera::get_direction()
{
    return glm::vec3(
        cos(_vertical_angle) * sin(_horizontal_angle),
        sin(_vertical_angle),
        cos(_vertical_angle) * cos(_horizontal_angle));
}
