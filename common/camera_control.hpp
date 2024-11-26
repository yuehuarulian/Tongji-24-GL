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
    Camera() = default;

    Camera(GLFWwindow *window, float initialfov = GLM_PI / 4, glm::vec3 position = glm::vec3(0, 0, 20), float horizontal_angle = GLM_PI, float vertical_angle = 0.f,
           float speed = 5.0f, float mouse_speed = 1.0f);

    void compute_matrices_from_inputs(GLFWwindow *window,  bool &userInteracted, glm::vec3 center = glm::vec3(0, 0, 0));

    // gets函数
    glm::vec3 get_pos() { return _position; }
    float get_fov() { return _initial_fov; }
    glm::vec3 get_direction() { return _direction; }
    glm::vec3 get_right() { return _right; }
    glm::vec3 get_up() { return _up; }
    float get_horizontal_angle() { return _horizontal_angle; }
    float get_vertical_angle() { return _vertical_angle; }
    // sets函数
    void set_position(glm::vec3 position) { this->_position = position; }
    void set_fov(float fov) { this->_initial_fov = fov; }

    // 投影和观察矩阵
    glm::mat4 projection;
    glm::mat4 view;

private:
    void calculate_view_and_projection_matrix();
    float _width, _height;
    glm::vec3 _position;     // 摄像机初始位置
    float _horizontal_angle; // 摄像机初始水平角度 x-z坐标系 z轴正方向为0度
    float _vertical_angle;   // 摄像机初始垂直角度 y-z坐标系 z轴正方向为0度
    float _speed;
    float _mouse_speed;
    float _initial_fov;
    glm::vec3 _direction;
    glm::vec3 _right;
    glm::vec3 _up;
};

#endif
