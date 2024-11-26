// #ifndef BUTTERFLY_HPP
// #define BUTTERFLY_HPP

// #include "shader.hpp"
// #include "renderable_model.hpp"
// #include "animator.hpp"
// #include "animation.hpp"
// #define GLM_ENABLE_EXPERIMENTAL
// #include "glm/glm.hpp"

// namespace GL_TASK
// {
//     class Butterfly : public RenderableModel
//     {
//     public:
//         Butterfly(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma = false);

//         void draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos) override;

//         void set_model_matrix(const glm::mat4 &model) { model_matrix = model; }

//     private:
//         Animation danceAnimation;
//         Animator animator;
//         glm::mat4 model_matrix = glm::mat4(1.0f);
//         float deltaTime = 0.0f;
//         float lastFrame = 0.0f;
//         int differ=rand();
//     };
// }

// #endif // BUTTERFLY_HPP
