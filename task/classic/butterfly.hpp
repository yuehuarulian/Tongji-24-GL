#ifndef BUTTERFLY_HPP
#define BUTTERFLY_HPP

#include "renderable_model.hpp"
#include "animator.hpp"
#include "animation.hpp"
#define GLM_ENABLE_EXPERIMENTAL

namespace GL_TASK
{
    class Butterfly : public RenderableModel
    {
    public:
        Butterfly(const std::string &model_path, std::vector<Mesh *> &meshes, std::vector<MeshInstance *> &meshInstances, std::vector<Texture *> &textures, std::vector<Material> &materials);

        void update() override;

    private:
        void set_model_matrix() override;

        void update_matrix();

        bool add_model(const std::string &model_path) override;

        const float scale_rand = (float)((rand() % (500 - 200)) + 200) / 100 * 2;       // 缩放
        const float rotate_rand = (float)((rand() % (450 - (-450))) + (-450)) / 10;     // 旋转角
        const float translate_rand = (float)((rand() % (500 - (-500))) + (-500)) / 100; // 模型位置偏移

        glm::mat4 keyframe_transforms_r;
        glm::mat4 keyframe_transforms_l;
        Animation danceAnimation;
        Animator animator;
        int differ = rand(); // 动画时间偏移

        int start_meshInstance_id_l;
        int end_meshInstance_id_l;
        int start_meshInstance_id_r;
        int end_meshInstance_id_r;
    };
}

#endif // BUTTERFLY_HPP
