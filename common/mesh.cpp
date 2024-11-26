#include <mesh.hpp>

Mesh::Mesh(vector<Vertex> vertices, vector<unsigned int> indices, const Material &material)
    : vertices(vertices),
      indices(indices),
      material(material)
{
    bvh = new BVH(2.0);
}

void Mesh::BuildBVH()
{
    //
    // 对一个Mesh节点构建BVH节点
    //
    int numTris = indices.size() / 3;
    printf("Triangle Nums: #%d\n", numTris);
    std::vector<AABB> bounds(numTris);

    // 遍历所有的三角形 -- 为其创建包围盒
    for (int i = 0; i < numTris; i++)
    {
        const glm::vec3 v1 = vertices[indices[i * 3 + 0]].Position;
        const glm::vec3 v2 = vertices[indices[i * 3 + 1]].Position;
        const glm::vec3 v3 = vertices[indices[i * 3 + 2]].Position;

        bounds[i].grow(v1);
        bounds[i].grow(v2);
        bounds[i].grow(v3);
    }
    bvh->Build(&bounds[0], numTris);
}

void Mesh::ProcessVertices(std::vector<glm::vec4> &verticesUVX, std::vector<glm::vec4> &normalsUVY)
{
    //
    // 将节点数据转换成可以传送给Shader的类型
    //
    for (const auto &vertex : vertices)
    {
        // 组合位置和纹理坐标 u/s，存入 verticesUVX
        verticesUVX.emplace_back(vertex.Position.x, vertex.Position.y, vertex.Position.z, vertex.TexCoords.x);

        // 组合法线和纹理坐标 v/t，存入 normalsUVY
        normalsUVY.emplace_back(vertex.Normal.x, vertex.Normal.y, vertex.Normal.z, vertex.TexCoords.y);
    }
}

// unsigned int Mesh::createDefaultTexture()
// {
//     unsigned int textureID;
//     glGenTextures(1, &textureID);
//     glBindTexture(GL_TEXTURE_2D, textureID);
//     // 创建一个 1x1 的白色像素数据
//     unsigned char whitePixel[3] = {0, 0, 0};
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
//     // 设置纹理参数
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//     return textureID;
// }

void Mesh::updateMesh()
{
    needsUpdate = true;
}

// // render the mesh
// void Mesh::Draw(Shader &shader)
// {
//     // 如果需要更新，先更新网格
//     if (needsUpdate)
//     {
//         update();
//         needsUpdate = false;
//     }
//     // 确保所有 PBR 纹理都绑定到固定的纹理单元位置
//     unsigned int defaultTextureID = createDefaultTexture(); // 初始化时调用一次
//     unsigned int textureUnit = 0;
//     for (const auto &type : {"texture_diffuse", "texture_specular", "texture_normal",
//                              "texture_height", "texture_metallic", "texture_roughness", "texture_ao", "texture_opacity"})
//     {
//         bool textureFound = false;
//         for (unsigned int i = 0; i < textures.size(); i++)
//         {
//             if (textures[i].type == type)
//             {
//                 // 激活指定的纹理单元并绑定
//                 glActiveTexture(GL_TEXTURE0 + textureUnit);
//                 glBindTexture(GL_TEXTURE_2D, textures[i].id);
//                 // 将对应 uniform 设置为这个纹理单元
//                 glUniform1i(glGetUniformLocation(shader.ID, (std::string(type) + "1").c_str()), textureUnit);
//                 // printf("name: %s, number: %d\n", (std::string(type) + "1").c_str(), textureUnit);
//                 textureFound = true;
//                 break; // 该类型找到就可以跳出循环
//             }
//         }
//         if (!textureFound)
//         {
//             glActiveTexture(GL_TEXTURE0 + textureUnit);
//             glBindTexture(GL_TEXTURE_2D, defaultTextureID); // 使用默认的白色纹理
//         }
//         textureUnit++;
//     }
//     // draw mesh
//     glBindVertexArray(VAO);
//     glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()), GL_UNSIGNED_INT, 0);
//     glBindVertexArray(0);
//     // always good practice to set everything back to defaults once configured.
//     glActiveTexture(GL_TEXTURE0);
// }

// void Mesh::setupMesh()
// {
//     // create buffers/arrays
//     glGenVertexArrays(1, &VAO);
//     glGenBuffers(1, &VBO);
//     glGenBuffers(1, &EBO);
//     glBindVertexArray(VAO);
//     // load data into vertex buffers
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     // A great thing about structs is that their memory layout is sequential for all its items.
//     // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
//     // again translates to 3/2 floats which translates to a byte array.
//     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
//     // set the vertex attribute pointers
//     glEnableVertexAttribArray(0); // 位置
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
//     glEnableVertexAttribArray(1); // 法线
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
//     glEnableVertexAttribArray(2); // 纹理坐标
//     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
//     glEnableVertexAttribArray(3); // 切线
//     glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
//     glEnableVertexAttribArray(4); // 副切线
//     glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));
//     glEnableVertexAttribArray(5); // 骨骼ID
//     glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, m_BoneIDs));
//     glEnableVertexAttribArray(6); // 骨骼权重
//     glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, m_Weights));
//     glBindVertexArray(0);
// }

// void Mesh::update()
// {
//     // 绑定VAO
//     glBindVertexArray(VAO);

//     // 更新顶点缓冲数据
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
//     // 更新索引缓冲数据
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
//     // 更新顶点属性指针设置
//     glEnableVertexAttribArray(0); // 位置
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
//     glEnableVertexAttribArray(1); // 法线
//     glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Normal));
//     glEnableVertexAttribArray(2); // 纹理坐标
//     glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, TexCoords));
//     glEnableVertexAttribArray(3); // 切线
//     glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Tangent));
//     glEnableVertexAttribArray(4); // 副切线
//     glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, Bitangent));
//     glEnableVertexAttribArray(5); // 骨骼ID
//     glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void *)offsetof(Vertex, m_BoneIDs));
//     glEnableVertexAttribArray(6); // 骨骼权重
//     glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)offsetof(Vertex, m_Weights));
//     // 解绑VAO
//     glBindVertexArray(0);
// }
