#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <functional>

#include "model.hpp"
#include "assimp_glm_helpers.hpp"

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

struct Keyframes
{
    std::vector<KeyPosition> positions;
    std::vector<KeyRotation> rotations;
    std::vector<KeyScale> scales;
};
class Animation
{
public:
    Animation() = default;

    Animation(const std::string& animationPath, Model* model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);
        auto animation = scene->mAnimations[0];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;
        aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
        globalTransformation = globalTransformation.Inverse();
        ReadHierarchyData(m_RootNode, scene->mRootNode);
        ReadKeyframes(animation, *model);
    }

    ~Animation(){}

    inline float GetTicksPerSecond() { return m_TicksPerSecond; }
    inline float GetDuration() { return m_Duration;}
    inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
    Keyframes GetKeyframes(std::string nodeName){return m_Keyframes[nodeName];}
    std::map<std::string, Keyframes> m_Keyframes;

private:
    void ReadKeyframes(const aiAnimation* animation, Model& model)
    {
        for (unsigned int i = 0; i < animation->mNumChannels; ++i)
        {
            auto channel = animation->mChannels[i];
            std::string nodeName = channel->mNodeName.data;

            // 存储关键帧数据
            for (unsigned int j = 0; j < channel->mNumPositionKeys; ++j)
            {
                aiVector3D position = channel->mPositionKeys[j].mValue;
                float timeStamp = channel->mPositionKeys[j].mTime;
                m_Keyframes[nodeName].positions.push_back({ AssimpGLMHelpers::GetGLMVec(position), timeStamp });
            }

            for (unsigned int j = 0; j < channel->mNumRotationKeys; ++j)
            {
                aiQuaternion rotation = channel->mRotationKeys[j].mValue;
                float timeStamp = channel->mRotationKeys[j].mTime;
                m_Keyframes[nodeName].rotations.push_back({ AssimpGLMHelpers::GetGLMQuat(rotation), timeStamp });
            }

            for (unsigned int j = 0; j < channel->mNumScalingKeys; ++j)
            {
                aiVector3D scale = channel->mScalingKeys[j].mValue;
                float timeStamp = channel->mScalingKeys[j].mTime;
                m_Keyframes[nodeName].scales.push_back({ AssimpGLMHelpers::GetGLMVec(scale), timeStamp });
            }
        }
    }

    void ReadHierarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        assert(src);

        dest.name = src->mName.data;
        dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHierarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }

    float m_Duration;
    int m_TicksPerSecond;
    AssimpNodeData m_RootNode;
};