#ifndef CAMERA_CONTROL_H
#define CAMERA_CONTROL_H

#define GLM_PI 3.1415926535897932384626433832795f

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera
{
public:
    // 投影和观察矩阵
    glm::mat4 ProjectionMatrix;
    glm::mat4 ViewMatrix;

    Camera(GLFWwindow *window, float initialfov = 45.0f, glm::vec3 position = glm::vec3(0, 0, 20), float horizontal_angle = GLM_PI, float vertical_angle = 0.f,
           float speed = 5.0f, float mouse_speed = 1.0f);

    Camera() = default;

    void computeMatricesFromInputs(GLFWwindow *window, glm::vec3 center = glm::vec3(0, 0, 0));

    glm::vec3 get_camerapos()
    {
        return _position;
    }

private:
    glm::vec3 _position;     // 摄像机初始位置
    float _horizontal_angle; // 摄像机初始水平角度 x-z坐标系 z轴正方向为0度
    float _vertical_angle;   // 摄像机初始垂直角度 y-z坐标系 z轴正方向为0度
    float _speed;
    float _mouse_speed;
    float _initial_fov;
};

#endif
