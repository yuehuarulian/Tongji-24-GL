#pragma once

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
//#include <bone.h>
#include <functional>
#include <animdata.h>
//#include <model_animation.h>
#include "model.hpp"
#include "assimp_glm_helpers.h"

struct AssimpNodeData
{
	glm::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

/*class Animation
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
		ReadMissingBones(animation, *model);
	}

	~Animation()
	{
	}

	Bone* FindBone(const std::string& name)
	{
		auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
			[&](const Bone& Bone)
			{
				return Bone.GetBoneName() == name;
			}
		);
		if (iter == m_Bones.end()) return nullptr;
		else return &(*iter);
	}

	
	inline float GetTicksPerSecond() { return m_TicksPerSecond; }
	inline float GetDuration() { return m_Duration;}
	inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
	inline const std::map<std::string,BoneInfo>& GetBoneIDMap() 
	{ 
		return m_BoneInfoMap;
	}

private:
	void ReadMissingBones(const aiAnimation* animation, Model& model)
	{
		int size = animation->mNumChannels;

		auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
		int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

		//reading channels(bones engaged in an animation and their keyframes)
		for (int i = 0; i < size; i++)
		{
			auto channel = animation->mChannels[i];
			std::string boneName = channel->mNodeName.data;

			if (boneInfoMap.find(boneName) == boneInfoMap.end())
			{
				boneInfoMap[boneName].id = boneCount;
				boneCount++;
			}
			m_Bones.push_back(Bone(channel->mNodeName.data,
				boneInfoMap[channel->mNodeName.data].id, channel));
		}

		m_BoneInfoMap = boneInfoMap;
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
	std::vector<Bone> m_Bones;
	AssimpNodeData m_RootNode;
	std::map<std::string, BoneInfo> m_BoneInfoMap;
};*/
/*class Animation
{
public:
    Animation(const aiAnimation *animation) : animation(animation) {}

    void update(float time)
    {
        // 计算当前时间对应的动画帧
        float ticksPerSecond = animation->mTicksPerSecond != 0 ? animation->mTicksPerSecond : 25.0f;
        float timeInTicks = time * ticksPerSecond;
        float animationTime = fmod(timeInTicks, animation->mDuration);

        // 更新骨骼变换矩阵
        readNodeHeirarchy(animationTime, animation->mRootNode, glm::mat4(1.0f));
    }

private:
    const aiAnimation *animation;
    std::map<std::string, BoneInfo> boneInfoMap;

    void readNodeHeirarchy(float animationTime, const aiNode *node, const glm::mat4 &parentTransform)
    {
        std::string nodeName = node->mName.data;
        const aiAnimation *animation = this->animation;

        glm::mat4 nodeTransform = convertMatrix(node->mTransformation);

        const aiNodeAnim *nodeAnim = findNodeAnim(animation, nodeName);

        if (nodeAnim)
        {
            // 计算当前帧的变换矩阵
            glm::vec3 scaling = calcInterpolatedScaling(animationTime, nodeAnim);
            glm::quat rotation = calcInterpolatedRotation(animationTime, nodeAnim);
            glm::vec3 translation = calcInterpolatedPosition(animationTime, nodeAnim);

            nodeTransform = glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scaling);
        }

        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int boneIndex = boneInfoMap[nodeName].id;
            boneInfoMap[nodeName].finalTransformation = globalTransformation * boneInfoMap[nodeName].offsetMatrix;
        }

        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            readNodeHeirarchy(animationTime, node->mChildren[i], globalTransformation);
        }
    }

    const aiNodeAnim *findNodeAnim(const aiAnimation *animation, const std::string nodeName)
    {
        for (unsigned int i = 0; i < animation->mNumChannels; i++)
        {
            const aiNodeAnim *nodeAnim = animation->mChannels[i];
            if (std::string(nodeAnim->mNodeName.data) == nodeName)
            {
                return nodeAnim;
            }
        }
        return nullptr;
    }

    glm::vec3 calcInterpolatedScaling(float animationTime, const aiNodeAnim *nodeAnim)
    {
        if (nodeAnim->mNumScalingKeys == 1)
        {
            return convertVector(nodeAnim->mScalingKeys[0].mValue);
        }

        unsigned int scalingIndex = findScaling(animationTime, nodeAnim);
        unsigned int nextScalingIndex = (scalingIndex + 1);
        assert(nextScalingIndex < nodeAnim->mNumScalingKeys);
        float deltaTime = nodeAnim->mScalingKeys[nextScalingIndex].mTime - nodeAnim->mScalingKeys[scalingIndex].mTime;
        float factor = (animationTime - nodeAnim->mScalingKeys[scalingIndex].mTime) / deltaTime;
        assert(factor >= 0.0f && factor <= 1.0f);
        const aiVector3D &start = nodeAnim->mScalingKeys[scalingIndex].mValue;
        const aiVector3D &end = nodeAnim->mScalingKeys[nextScalingIndex].mValue;
        aiVector3D delta = end - start;
        return convertVector(start + factor * delta);
    }

    glm::quat calcInterpolatedRotation(float animationTime, const aiNodeAnim *nodeAnim)
    {
        if (nodeAnim->mNumRotationKeys == 1)
        {
            return convertQuaternion(nodeAnim->mRotationKeys[0].mValue);
        }

        unsigned int rotationIndex = findRotation(animationTime, nodeAnim);
        unsigned int nextRotationIndex = (rotationIndex + 1);
        assert(nextRotationIndex < nodeAnim->mNumRotationKeys);
        float deltaTime = nodeAnim->mRotationKeys[nextRotationIndex].mTime - nodeAnim->mRotationKeys[rotationIndex].mTime;
        float factor = (animationTime - nodeAnim->mRotationKeys[rotationIndex].mTime) / deltaTime;
        assert(factor >= 0.0f && factor <= 1.0f);
        const aiQuaternion &startRotationQ = nodeAnim->mRotationKeys[rotationIndex].mValue;
        const aiQuaternion &endRotationQ = nodeAnim->mRotationKeys[nextRotationIndex].mValue;
        aiQuaternion interpolatedQ;
        aiQuaternion::Interpolate(interpolatedQ, startRotationQ, endRotationQ, factor);
        interpolatedQ = interpolatedQ.Normalize();
        return convertQuaternion(interpolatedQ);
    }

    glm::vec3 calcInterpolatedPosition(float animationTime, const aiNodeAnim *nodeAnim)
    {
        if (nodeAnim->mNumPositionKeys == 1)
        {
            return convertVector(nodeAnim->mPositionKeys[0].mValue);
        }

        unsigned int positionIndex = findPosition(animationTime, nodeAnim);
        unsigned int nextPositionIndex = (positionIndex + 1);
        assert(nextPositionIndex < nodeAnim->mNumPositionKeys);
        float deltaTime = nodeAnim->mPositionKeys[nextPositionIndex].mTime - nodeAnim->mPositionKeys[positionIndex].mTime;
        float factor = (animationTime - nodeAnim->mPositionKeys[positionIndex].mTime) / deltaTime;
        assert(factor >= 0.0f && factor <= 1.0f);
        const aiVector3D &start = nodeAnim->mPositionKeys[positionIndex].mValue;
        const aiVector3D &end = nodeAnim->mPositionKeys[nextPositionIndex].mValue;
        aiVector3D delta = end - start;
        return convertVector(start + factor * delta);
    }

    unsigned int findScaling(float animationTime, const aiNodeAnim *nodeAnim)
    {
        for (unsigned int i = 0; i < nodeAnim->mNumScalingKeys - 1; i++)
        {
            if (animationTime < nodeAnim->mScalingKeys[i + 1].mTime)
            {
                return i;
            }
        }
        assert(0);
        return 0;
    }

    unsigned int findRotation(float animationTime, const aiNodeAnim *nodeAnim)
    {
        for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++)
        {
            if (animationTime < nodeAnim->mRotationKeys[i + 1].mTime)
            {
                return i;
            }
        }
        assert(0);
        return 0;
    }

    unsigned int findPosition(float animationTime, const aiNodeAnim *nodeAnim)
    {
        for (unsigned int i = 0; i < nodeAnim->mNumPositionKeys - 1; i++)
        {
            if (animationTime < nodeAnim->mPositionKeys[i + 1].mTime)
            {
                return i;
            }
        }
        assert(0);
        return 0;
    }

    glm::mat4 convertMatrix(const aiMatrix4x4 &matrix)
    {
        return glm::mat4(
            matrix.a1, matrix.b1, matrix.c1, matrix.d1,
            matrix.a2, matrix.b2, matrix.c2, matrix.d2,
            matrix.a3, matrix.b3, matrix.c3, matrix.d3,
            matrix.a4, matrix.b4, matrix.c4, matrix.d4);
    }

    glm::vec3 convertVector(const aiVector3D &vector)
    {
        return glm::vec3(vector.x, vector.y, vector.z);
    }

    glm::quat convertQuaternion(const aiQuaternion &quaternion)
    {
        return glm::quat(quaternion.w, quaternion.x, quaternion.y, quaternion.z);
    }
};
*/
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

    ~Animation()
    {
    }

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
    //std::map<std::string, Keyframes> m_Keyframes;
};