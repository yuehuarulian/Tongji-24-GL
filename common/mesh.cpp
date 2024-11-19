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
