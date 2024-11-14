#include <mesh.hpp>

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
    : vertices(vertices), indices(indices), textures(textures)
{
    setupMesh();
    std::cout << "#00 Build BVHTree begin..." << std::endl;
    bvh = new BVH(2.0);
    BuildBVH();
    std::cout << "#-1 Build BVHTree Success!" << std::endl;
}

void Mesh::BuildBVH()
{
    int numTris = indices.size() / 3;
    std::vector<AABB> bounds(numTris);
    for (int i = 0; i < numTris; i++)
    {
        const glm::vec3 v1 = vertices[i * 3 + 0].Position;
        const glm::vec3 v2 = vertices[i * 3 + 1].Position;
        const glm::vec3 v3 = vertices[i * 3 + 2].Position;

        bounds[i].grow(v1);
        bounds[i].grow(v2);
        bounds[i].grow(v3);
    }

    std::cout << "#01 Build BVHTree..." << std::endl;
    bvh->Build(&bounds[0], numTris);
    bvh->PrintStatistics(std::cout);
}

unsigned int Mesh::createDefaultTexture()
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

void Mesh::Draw(Shader &shader)
{
    // 确保所有 PBR 纹理都绑定到固定的纹理单元位置
    unsigned int defaultTextureID = createDefaultTexture();
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

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::setupMesh()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

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