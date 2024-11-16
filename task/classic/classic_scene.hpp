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

        void setCamera(Camera* camera);
        void render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos) override;

    private:
        glm::mat4 room_model_matrix = glm::mat4(1.0f);

        void setup_scene() override;
        void InitShaders() override;

        void LoadModels();
        void LoadLights();
    };
}
#endif
