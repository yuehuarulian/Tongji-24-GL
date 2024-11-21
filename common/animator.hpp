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
    Animator(Animation *animation)
    {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;
    }

    void UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation)
        {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            CalculateNodeTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));

            glm::vec3 pathtranslate(m_CurrentTime / 500, 0.0f, m_CurrentTime / 500);
            glm::vec4 translation_R = m_KeyframeTransforms["R"][3]; // 获取第四列
            translation_R += glm::vec4(pathtranslate, 0.0f);        // 直接加上平移向量
            m_KeyframeTransforms["R"][3] = translation_R;           // 设置回去
            glm::vec4 translation_L = m_KeyframeTransforms["L"][3]; // 获取第四列
            translation_L += glm::vec4(pathtranslate, 0.0f);        // 直接加上平移向量
            m_KeyframeTransforms["L"][3] = translation_L;           // 设置回去
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
    float m_DeltaTime;

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
        //assert(0);
    }

    int GetRotationIndex(const std::vector<KeyRotation> &rotations, float animationTime)
    {
        for (int index = 0; index < rotations.size() - 1; ++index)
        {
            if (animationTime < rotations[index + 1].timeStamp)
                return index;
        }
        return 0;
        //assert(0);
    }

    int GetScaleIndex(const std::vector<KeyScale> &scales, float animationTime)
    {
        for (int index = 0; index < scales.size() - 1; ++index)
        {
            if (animationTime < scales[index + 1].timeStamp)
                return index;
        }
        return 0;
        //assert(0);
    }

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        return midWayLength / framesDiff;
    }
};