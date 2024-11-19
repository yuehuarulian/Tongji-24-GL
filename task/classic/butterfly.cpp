#include "butterfly.hpp"
#include "glm/gtx/transform.hpp"
#include "error.hpp"

GL_TASK::Butterfly::Butterfly(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma)
    : RenderableModel(model_path, std::move(shader), gamma) ,danceAnimation(model_path, &model), animator(&danceAnimation){}

void GL_TASK::Butterfly::draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos)
{
    float currentFrame = glfwGetTime()+differ;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    // 更新动画
    animator.UpdateAnimation(deltaTime);

    // 已确认有时间更新
    //std::cout << "Updating animation with deltaTime: " << deltaTime << std::endl;

    // std::cout << "Model matrix: " << glm::to_string(model_matrix) << std::endl;

    // std::cout << "transforms size:" << transforms.size() << std::endl;
    // auto transforms = animator.GetFinalBoneMatrices();
    // for (int i = 0; i < transforms.size(); ++i)
    //     shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

    //std::map<std::string, glm::mat4> keyframeTransforms = animator.m_KeyframeTransforms;
    glm::mat4 keyframeTransforms = animator.m_KeyframeTransforms["R"];
    //glm::mat4 current;

    // 传递关键帧变换矩阵给顶点着色器
    /*for (const auto &transform : keyframeTransforms)
    {

        std::cout << "index: " << transform.first << std::endl; //
        std::cout << "keyframe: " << glm::to_string(transform.second) << std::endl; //
        //std::string uniformName = "keyframeTransforms[" + transform.first + "]";
        shader->use();
        //shader->setMat4(uniformName, transform.second);
        shader->setMat4("keyframeTransforms", transform.second);
        current=transform.second;

    }
    std::cout << "keyframeinshader: " << glm::to_string(current) << std::endl; //*/

    shader->use();
    //shader->setMat4("keyframeTransforms", keyframeTransforms["Circle"]);
    shader->setMat4("keyframeTransforms", keyframeTransforms);
    //std::cout << "keyframeinshader: " << glm::to_string(keyframeTransforms) << std::endl;
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", model_matrix);
    shader->setVec3("camPos", camera_pos);
    model.Draw(*shader);
    keyframeTransforms = animator.m_KeyframeTransforms["L"];
    shader->setMat4("keyframeTransforms", keyframeTransforms);

    model.Draw(*shader);
    // checkOpenGLError("After model draw");
}