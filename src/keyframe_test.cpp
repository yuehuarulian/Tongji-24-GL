// 实现基于关键帧的动画逻辑
// 1. 解析 DAE 文件
// 2. 存储关键帧数据
// 3. 插值关键帧以实现平滑动画
// 4. 在 OpenGL 中渲染动画

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct Keyframe
{
    float time;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

struct Animation
{
    std::vector<Keyframe> keyframes;
};

Animation loadAnimationFromDAE(const std::string &filePath)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs);
    Animation animation;

    if (!scene || !scene->mAnimations)
    {
        throw std::runtime_error("无法加载动画数据");
    }

    aiAnimation *aiAnim = scene->mAnimations[0];
    for (unsigned int i = 0; i < aiAnim->mChannels[0]->mNumPositionKeys; ++i)
    {
        Keyframe keyframe;
        keyframe.time = aiAnim->mChannels[0]->mPositionKeys[i].mTime;
        keyframe.position = glm::vec3(
            aiAnim->mChannels[0]->mPositionKeys[i].mValue.x,
            aiAnim->mChannels[0]->mPositionKeys[i].mValue.y,
            aiAnim->mChannels[0]->mPositionKeys[i].mValue.z);

        keyframe.rotation = glm::quat(
            aiAnim->mChannels[0]->mRotationKeys[i].mValue.w,
            aiAnim->mChannels[0]->mRotationKeys[i].mValue.x,
            aiAnim->mChannels[0]->mRotationKeys[i].mValue.y,
            aiAnim->mChannels[0]->mRotationKeys[i].mValue.z);

        keyframe.scale = glm::vec3(
            aiAnim->mChannels[0]->mScalingKeys[i].mValue.x,
            aiAnim->mChannels[0]->mScalingKeys[i].mValue.y,
            aiAnim->mChannels[0]->mScalingKeys[i].mValue.z);

        animation.keyframes.push_back(keyframe);
    }
    return animation;
}

glm::mat4 interpolateKeyframes(const Keyframe &kf1, const Keyframe &kf2, float t)
{
    float factor = (t - kf1.time) / (kf2.time - kf1.time);
    glm::vec3 interpolatedPosition = glm::mix(kf1.position, kf2.position, factor);
    glm::quat interpolatedRotation = glm::slerp(kf1.rotation, kf2.rotation, factor);
    glm::vec3 interpolatedScale = glm::mix(kf1.scale, kf2.scale, factor);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, interpolatedPosition);
    model *= glm::mat4_cast(interpolatedRotation);
    model = glm::scale(model, interpolatedScale);
    return model;
}

void update(float time, const Animation &animation, glm::mat4 &modelMatrix)
{
    // 找到合适的关键帧
    for (size_t i = 0; i < animation.keyframes.size() - 1; ++i)
    {
        if (time >= animation.keyframes[i].time && time < animation.keyframes[i + 1].time)
        {
            modelMatrix = interpolateKeyframes(animation.keyframes[i], animation.keyframes[i + 1], time);
            break;
        }
    }
}

void renderModel(const glm::mat4 &modelMatrix)
{
    // 使用 OpenGL 渲染
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    // 绘制静态模型的逻辑
}

int main()
{
    // 初始化 OpenGL 和窗口
    glfwInit();
    GLFWwindow *window = glfwCreateWindow(800, 600, "Keyframe Animation", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glewInit();

    // 加载模型和动画
    Animation animation = loadAnimationFromDAE("path/to/your/model.dae");

    float currentTime = 0.0f;
    glm::mat4 modelMatrix;

    while (!glfwWindowShouldClose(window))
    {
        // 更新动画时间
        currentTime += 0.01f; // 更新为合适的时间增量

        // 更新模型矩阵
        update(currentTime, animation, modelMatrix);

        // 渲染模型
        renderModel(modelMatrix);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
