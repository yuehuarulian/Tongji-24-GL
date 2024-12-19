#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>

#include "animation.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

class Animator
{
public:
    Animator(Animation *animation, float translate_rand, float rotate_rand)
    {
        m_CurrentTime = (float)(rand() % 10000);
        m_CurrentAnimation = animation;
        path_x = (float)((rand() % (20 - (-20))) + (-20)) / 10;
        path_y = (float)((rand() % (20 - (-20))) + (-20)) / 10; //-2.0-2.0
        path_z = (float)((rand() % (200))) / 10;                // 0-20.0

        // x方向的控制
        float x = path_z;
        if (translate_rand - path_z * sin(glm::radians(rotate_rand)) > 5.0f)
            x = (translate_rand - 5.0f) / sin(glm::radians(rotate_rand));
        if (translate_rand - path_z * sin(glm::radians(rotate_rand)) < -5.0f)
            x = (translate_rand + 5.0f) / sin(glm::radians(rotate_rand));

        // z方向控制
        float z = x;
        if ((translate_rand + x) > 5.0f)
            z = 5.0 - translate_rand;
        if ((translate_rand + x) < -25.0f)
            z = -25.0 - translate_rand;

        path_z = z;
    }

    void UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation)
        {
            float duration = m_CurrentAnimation->GetDuration();
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, duration);

            CalculateNodeTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));

            // 使用正弦函数而非线性插值计算路径变换
            float t = m_CurrentTime / duration;
            t = sin(t * glm::pi<float>());
            m_CurrentTime_Cout = t * duration / 10000;                                                                     // 0~1左右
            glm::mat4 pathtranslate = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, m_CurrentTime_Cout * path_z)); // 世界系中向右平移1单位

            m_KeyframeTransforms["R"] = pathtranslate * m_KeyframeTransforms["R"];
            m_KeyframeTransforms["L"] = pathtranslate * m_KeyframeTransforms["L"];
        }
    }

    void CalculateNodeTransform(const AssimpNodeData *node, glm::mat4 parentTransform)
    {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        auto keyframes = m_CurrentAnimation->GetKeyframes(nodeName);
        if (keyframes.positions.size() > 0 || keyframes.rotations.size() > 0 || keyframes.scales.size() > 0)
        {
            glm::vec3 position = InterpolatePosition(keyframes.positions, m_CurrentTime);
            glm::quat rotation = InterpolateRotation(keyframes.rotations, m_CurrentTime);
            glm::vec3 scale = InterpolateScale(keyframes.scales, m_CurrentTime);
            // glm::vec3 position = InterpolatePosition(keyframes.positions, m_CurrentTime_Cout);
            // glm::quat rotation = InterpolateRotation(keyframes.rotations, m_CurrentTime_Cout);
            // glm::vec3 scale = InterpolateScale(keyframes.scales, m_CurrentTime_Cout);

            nodeTransform = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale);
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;
        m_KeyframeTransforms[nodeName] = globalTransformation;
        for (int i = 0; i < node->childrenCount; i++)
            CalculateNodeTransform(&node->children[i], globalTransformation);
    }

    std::map<std::string, glm::mat4> m_KeyframeTransforms;

private:
    Animation *m_CurrentAnimation;
    float m_CurrentTime;
    float m_CurrentTime_Cout;
    float m_DeltaTime;
    float path_x;
    float path_y;
    float path_z;

    glm::vec3 InterpolatePosition(const std::vector<KeyPosition> &positions, float animationTime)
    {
        if (positions.size() == 1)
            return positions[0].position;

        int p0Index = GetPositionIndex(positions, animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(positions[p0Index].timeStamp, positions[p1Index].timeStamp, animationTime);
        return glm::mix(positions[p0Index].position, positions[p1Index].position, scaleFactor);
    }

    glm::quat InterpolateRotation(const std::vector<KeyRotation> &rotations, float animationTime)
    {
        if (rotations.size() == 1)
            return rotations[0].orientation;

        int p0Index = GetRotationIndex(rotations, animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(rotations[p0Index].timeStamp, rotations[p1Index].timeStamp, animationTime);
        return glm::slerp(rotations[p0Index].orientation, rotations[p1Index].orientation, scaleFactor);
    }

    glm::vec3 InterpolateScale(const std::vector<KeyScale> &scales, float animationTime)
    {
        if (scales.size() == 1)
            return scales[0].scale;

        int p0Index = GetScaleIndex(scales, animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(scales[p0Index].timeStamp, scales[p1Index].timeStamp, animationTime);
        return glm::mix(scales[p0Index].scale, scales[p1Index].scale, scaleFactor);
    }

    int GetPositionIndex(const std::vector<KeyPosition> &positions, float animationTime)
    {
        for (int index = 0; index < positions.size() - 1; ++index)
        {
            if (animationTime < positions[index + 1].timeStamp)
                return index;
        }
        return 0;
        // assert(0);
    }

    int GetRotationIndex(const std::vector<KeyRotation> &rotations, float animationTime)
    {
        for (int index = 0; index < rotations.size() - 1; ++index)
        {
            if (animationTime < rotations[index + 1].timeStamp)
                return index;
        }
        return 0;
        // assert(0);
    }

    int GetScaleIndex(const std::vector<KeyScale> &scales, float animationTime)
    {
        for (int index = 0; index < scales.size() - 1; ++index)
        {
            if (animationTime < scales[index + 1].timeStamp)
                return index;
        }
        return 0;
        // assert(0);
    }

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        return midWayLength / framesDiff;
    }
};