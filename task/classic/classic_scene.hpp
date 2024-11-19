#ifndef CLASSIC_SCENE_HPP
#define CLASSIC_SCENE_HPP

#include "scene.hpp"
#include "shader_manager.hpp"
#include "light_manager.hpp"
#include "renderable_model.hpp"
#include "camera_control.hpp"
#include <memory>

namespace GL_TASK
{
    class ClassicScene : public Scene
    {
    public:
        ClassicScene(ShaderManager &shader_manager, LightManager &light_manager);
        ~ClassicScene() = default;

        void updateCameraInfo(Camera *camera);
        void render() override; // 渲染函数

    private:
        void setup_scene() override; // 配置场景
        void InitShaders() override;

        void LoadModels();
        void LoadLights();
    };
}
#endif
