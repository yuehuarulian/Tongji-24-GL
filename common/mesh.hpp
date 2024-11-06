#ifndef MESH_H
#define MESH_H

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "shader.hpp"

#include <string>
#include <vector>
#include <map>

using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Vertex
{
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    // bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    // weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture
{
    unsigned int id;
    string type; // 例如: "texture_diffuse", "texture_specular", "texture_normal", "texture_metallic"
    string path;
};

class Mesh
{
public:
    // mesh Data
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;
    unsigned int VAO;

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
        : vertices(vertices), indices(indices), textures(textures)
    {
        setupMesh();
    }

    unsigned int createDefaultTexture()
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // 创建一个 1x1 的白色像素数据
        unsigned char whitePixel[3] = {255, 255, 255}; // 白色或其他中性色
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        return textureID;
    }
    // render the mesh
    void Draw(Shader &shader)
    {
        // 确保所有 PBR 纹理都绑定到固定的纹理单元位置
        unsigned int defaultTextureID = createDefaultTexture(); // 初始化时调用一次
        unsigned int textureUnit = 0;

        for (const auto &type : {"texture_diffuse", "texture_specular", "texture_normal",
                                 "texture_height", "texture_metallic", "texture_roughness", "texture_ao"})
        {
            bool textureFound = false;

            for (unsigned int i = 0; i < textures.size(); i++)
            {
                if (textures[i].type == type)
                {
                    // 激活指定的纹理单元并绑定
                    glActiveTexture(GL_TEXTURE0 + textureUnit);
                    glBindTexture(GL_TEXTURE_2D, textures[i].id);

                    // 将对应 uniform 设置为这个纹理单元
                    glUniform1i(glGetUniformLocation(shader.ID, (std::string(type) + "1").c_str()), textureUnit);
                    // printf("name: %s, number: %d\n", (std::string(type) + "1").c_str(), textureUnit);

                    textureFound = true;
                    break; // 该类型找到就可以跳出循环
                }
            }

            if (!textureFound)
            {
                glActiveTexture(GL_TEXTURE0 + textureUnit);
                glBindTexture(GL_TEXTURE_2D, defaultTextureID); // 使用默认的白色纹理
            }

            textureUnit++;
        }

        // draw mesh
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // render data
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh()
    {
        // create buffers/arrays
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        // load data into vertex buffers
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        glEnableVertexAttribArray(0); // 位置
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

        glEnableVertexAttribArray(1); // 法线
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));

        glEnableVertexAttribArray(2); // 纹理坐标
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));

        glEnableVertexAttribArray(3); // 切线
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));

        glEnableVertexAttribArray(4); // 副切线
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));

        glEnableVertexAttribArray(5); // 骨骼ID
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, m_BoneIDs));

        glEnableVertexAttribArray(6); // 骨骼权重
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, m_Weights));

        glBindVertexArray(0);
    }
};
#endif
