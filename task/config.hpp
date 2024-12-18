#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <iostream>
#include <vector>
#include <string>

const double D2R = glm::pi<double>() / 180.0; // 角度转弧度
const double R2D = 180.0 / glm::pi<double>(); // 弧度转角度
const unsigned int WINDOW_HEIGHT = 720;
const unsigned int WINDOW_WIDTH = 1080;
const unsigned int FRAMES = 300;
const unsigned int SAMPLES_PER_FRAME = 30; // 每帧采样的数量
const bool CAMERA_ANIMATION = false;       // 是否开启相机动画
const int START_FRAME = 0;                 // 开始帧数
const std::vector<std::string> faces{
    "source/skybox/sky/right.jpg",
    "source/skybox/sky/left.jpg",
    "source/skybox/sky/top.jpg",
    "source/skybox/sky/bottom.jpg",
    "source/skybox/sky/front.jpg",
    "source/skybox/sky/back.jpg"};