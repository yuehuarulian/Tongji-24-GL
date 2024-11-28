#ifndef BUTTERFLY_HPP
#define BUTTERFLY_HPP

#include "shader.hpp"
#include "renderable_model.hpp"
#include "animator.hpp"
#include "animation.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"

namespace GL_TASK
{
    class Butterfly
    {
    public:
        Butterfly(const std::string &model_path);

        bool add_model(const std::string &modelfilePath, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials);

        void update();

        void set_model_matrix(glm::mat4 &model)
        {
            model = glm::scale(model, glm::vec3(1.f, 1.f, 1.f) * scale_rand);                   // 缩放
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));      // 正向
            model = glm::rotate(model, glm::radians(rotate_rand), glm::vec3(0.0f, 1.0f, 0.0f)); // 旋转
            model = glm::translate(model, glm::vec3(translate_rand, 0.0f, translate_rand));     // 平移
            model_matrix = model;
        }
        glm::mat4 keyframe_transforms_r;
        glm::mat4 keyframe_transforms_l;

    private:
        void update_matrix();

        glm::mat4 model_matrix = glm::mat4(1.0f);
        Animation danceAnimation;
        Animator animator;
        float deltaTime = 0.0f;
        float lastFrame = 0.0f;
        int differ = rand();                                                      // 动画时间偏移
        float scale_rand = (float)((rand() % (500 - 200)) + 200) / 100;           // 缩放
        float rotate_rand = (float)((rand() % (900 - (-900))) + (-900)) / 10;     // 旋转角
        float translate_rand = (float)((rand() % (400) - (-400)) + (-400)) / 100; // 模型位置偏移
    };
}

#endif // BUTTERFLY_HPP
