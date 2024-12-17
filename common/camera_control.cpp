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
    _width = width;
    _height = height;
    _width = width;
    _height = height;
    glfwSetCursorPos(window, width / 2, height / 2);
    glfwSetScrollCallback(window, scroll_callback);
    calculate_view_and_projection_matrix();
}

void Camera::calculate_view_and_projection_matrix()
{
    // 计算方向、右向量和上向量
    _direction = glm::vec3(
        cos(_vertical_angle) * sin(_horizontal_angle),
        sin(_vertical_angle),
        cos(_vertical_angle) * cos(_horizontal_angle));
    _right = glm::vec3(
        sin(_horizontal_angle - GLM_PI / 2.0f),
        0,
        cos(_horizontal_angle - GLM_PI / 2.0f));
    _up = glm::cross(_right, _direction);

    // 更新投影和观察矩阵
    // ProjectionMatrix = glm::ortho(-10.0f * float(width) / height, 10.0f * float(width) / height, -10.0f, 10.0f, 0.0f, 100.0f);
    projection = glm::perspective(_initial_fov, float(_width) / _height, 0.5f, 1000.0f);
    view = glm::lookAt(_position, _position + _direction, _up);
}

void Camera::compute_matrices_from_inputs(GLFWwindow *window, bool &userInteracted, glm::vec3 center)
{
    userInteracted = false; // 初始状态为无操作

    // 时间差
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    // 获取鼠标位置并计算偏移量
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    _width = width;
    _height = height;
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

    if (xoffset != 0 || yoffset != 0)
        userInteracted = true;

    // 更新角度
    // _horizontal_angle -= _mouse_speed * deltaTime * float(xoffset);
    // _vertical_angle -= _mouse_speed * deltaTime * float(yoffset);           ///???感觉不对不是在世界坐标系下旋转是camera坐标系下旋转
    glm::mat4 rotation_matrix1 = glm::rotate(glm::mat4(1.0f), _mouse_speed * deltaTime * float(xoffset), _up);    // 绕up轴旋转
    glm::mat4 rotation_matrix2 = glm::rotate(glm::mat4(1.0f), _mouse_speed * deltaTime * float(yoffset), _right); // 绕right轴旋转
    _direction = glm::vec3(rotation_matrix2 * rotation_matrix1 * glm::vec4(_direction, 1.0f));
    _vertical_angle = glm::asin(_direction.y);
    _vertical_angle = glm::clamp(_vertical_angle, -glm::radians(89.0f), glm::radians(89.0f)); // 限制垂直角度
    _horizontal_angle = glm::atan(_direction.x, _direction.z);
    _right = glm::vec3(sin(_horizontal_angle - GLM_PI / 2.0f), 0, cos(_horizontal_angle - GLM_PI / 2.0f));
    _up = glm::cross(_right, _direction);

    // 处理键盘输入
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) // 相机右移，物体左移
    {
        _position += _right * deltaTime * _speed;
        userInteracted = true;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) // 相机左移，物体右移
    {
        _position -= _right * deltaTime * _speed;
        userInteracted = true;
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // 相机上移，物体下移
    {
        _position += _up * deltaTime * _speed;
        userInteracted = true;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) // 相机下移，物体上移
    {
        _position -= _up * deltaTime * _speed;
        userInteracted = true;
    }
    if (scrollYOffset >= 1e-6 || scrollYOffset <= -1e-6) // 滚轮滚动，相机前移后移
    {
        _position += _direction * float(scrollYOffset) * _speed * 0.3f;
        userInteracted = true;
    }
    scrollYOffset = 0.0;

    calculate_view_and_projection_matrix();
}

glm::vec3 Camera::get_pos()
{
    return _position;
}

void Camera::set_position(glm::vec3 position)
{
    _position = position;
    calculate_view_and_projection_matrix();
}

glm::vec3 Camera::get_direction() { return _direction; }

void Camera::set_direction(glm::vec3 direction)
{
    _direction = glm::normalize(direction); // 确保方向向量是单位向量

    // 更新右向量和上向量
    _right = glm::normalize(glm::cross(_direction, glm::vec3(0.0f, 1.0f, 0.0f))); // 基于世界空间中的 y 轴计算右向量
    _up = glm::normalize(glm::cross(_right, _direction));                         // 基于右向量和方向向量计算上向量

    // 计算水平角度和垂直角度
    _horizontal_angle = glm::atan(_direction.x, _direction.z);
    _vertical_angle = glm::asin(_direction.y);
    std::cout << "horizontal_angle: " << _horizontal_angle << " vertical_angle: " << _vertical_angle << std::endl;

    // 更新视图矩阵
    calculate_view_and_projection_matrix();
}

void Camera::set_direction(glm::mat3 rotation_matrix)
{
    // 从旋转矩阵中提取方向向量
    _direction = glm::vec3(rotation_matrix * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
    _right = glm::vec3(rotation_matrix * glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    _up = glm::vec3(rotation_matrix * glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
    // _up = glm::normalize(glm::cross(_right, _direction));

    // 计算水平角度和垂直角度
    _horizontal_angle = glm::atan(_direction.x, _direction.z);
    _vertical_angle = glm::asin(_direction.y);
    std::cout << "horizontal_angle: " << _horizontal_angle << " vertical_angle: " << _vertical_angle << std::endl;

    // 更新视图矩阵
    calculate_view_and_projection_matrix();
}