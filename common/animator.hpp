#pragma once

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <animation.h>
#include <bone.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

/*class Animator
{
public:
    Animator(Animation *animation)
    {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;

        m_FinalBoneMatrices.reserve(100);

        for (int i = 0; i < 100; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    void UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation)
        {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    void PlayAnimation(Animation *pAnimation)
    {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
    }

    void CalculateBoneTransform(const AssimpNodeData *node, glm::mat4 parentTransform)
    {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        Bone *Bone = m_CurrentAnimation->FindBone(nodeName);

        if (Bone)
        {
            Bone->Update(m_CurrentTime);
            nodeTransform = Bone->GetLocalTransform();
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].id;
        //std::cout << "index" << index << std::endl;
            glm::mat4 offset = boneInfoMap[nodeName].offset;
        //std::cout << "offset" << glm::to_string(offset) << std::endl;
            m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }

    std::vector<glm::mat4> GetFinalBoneMatrices()
    {
        return m_FinalBoneMatrices;
    }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;
    Animation *m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;
};*/
class Animator
{
public:
    Animator(Animation *animation)
    {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;

        m_FinalBoneMatrices.reserve(100);

        for (int i = 0; i < 100; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    void UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation)
        {
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;

            m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
            //std::cout<<m_CurrentTime<<std::endl;
            // std::cout<<m_CurrentAnimation->GetDuration()<<std::endl;
            CalculateNodeTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
            // glm::vec3 pathtranslate((2.0f + 1.0f * cos(m_CurrentTime * 2.0 * M_PI)) * cos(2.0f + 1.0f * cos(m_CurrentTime * 2.0 * M_PI)),
            //                         (2.0f + 1.0f * cos(2.0f + 1.0f * cos(m_CurrentTime * 2.0 * M_PI))) * sin(2.0f + 1.0f * cos(m_CurrentTime * 2.0 * M_PI)),
            //                         1.0 * sin(2.0f + 1.0f * cos(m_CurrentTime * 2.0 * M_PI)));
            //glm::vec3 pathtranslate(m_CurrentTime,m_CurrentTime,m_CurrentTime);

            glm::vec3 pathtranslate(m_CurrentTime/50,0.0f,m_CurrentTime/50);
            //std::cout << "keyframeinanimator: " << glm::to_string(m_KeyframeTransforms["Circle"]) << std::endl;
            //m_KeyframeTransforms["Circle"] = glm::translate(m_KeyframeTransforms["Circle"], pathtranslate);
            //std::cout << "keyframeinanimator: " << glm::to_string(m_KeyframeTransforms["Circle"]) << std::endl;
        }
    }

    void PlayAnimation(Animation *pAnimation)
    {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
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
        //std::cout << nodeName<<"&keyframeinprocess: " << glm::to_string(globalTransformation) << std::endl; //
        m_KeyframeTransforms[nodeName] = globalTransformation;

        // m_CurrentAnimation->m_Keyframes[nodeName] = globalTransformation;

        // std::cout<<node->childrenCount<<std::endl;//

        for (int i = 0; i < node->childrenCount; i++)
            CalculateNodeTransform(&node->children[i], globalTransformation);
    }

    std::vector<glm::mat4> GetFinalBoneMatrices()
    {
        return m_FinalBoneMatrices;
    }
    std::map<std::string, glm::mat4> m_KeyframeTransforms;

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;

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
        assert(0);
    }

    int GetRotationIndex(const std::vector<KeyRotation> &rotations, float animationTime)
    {
        for (int index = 0; index < rotations.size() - 1; ++index)
        {
            if (animationTime < rotations[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }

    int GetScaleIndex(const std::vector<KeyScale> &scales, float animationTime)
    {
        for (int index = 0; index < scales.size() - 1; ++index)
        {
            if (animationTime < scales[index + 1].timeStamp)
                return index;
        }
        assert(0);
    }

    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        return midWayLength / framesDiff;
    }
};