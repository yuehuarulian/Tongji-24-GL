#include <random>
#include <vector>
#include <string>
#include <algorithm> // 用于排序
#include <openvdb/openvdb.h>
#include <openvdb/io/File.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "shader.hpp"

const float MIN_DENSITY = 0.3f;
const float RANDOM_OFFSET = 0.0f;
class PointCloud
{
public:
    PointCloud(const std::string &vdbFile)
    {
        loadVDBData(vdbFile);
        setupOpenGL();
    }

    ~PointCloud()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    void render(const glm::mat4 &view, const glm::mat4 &projection, const glm::vec3 &viewPos, std::shared_ptr<Shader> shader, const glm::vec3 &fogColor, float fogDensity)
    {
        // 在渲染之前进行深度排序
        // sortPointsByDepth(viewPos);

        // 重新更新点云数据到 GPU
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

        shader->use();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-30.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.15f)); // 缩小点云，使其适应显示窗口
        shader->setMat4("model", model);
        shader->setMat4("view", view);
        shader->setMat4("projection", projection);

        // 设置雾相关的
        shader->setVec3("viewPos", viewPos);
        shader->setVec3("fogColor", fogColor);
        shader->setFloat("fogDensity", fogDensity);

        // 绑定 VAO 并绘制点云
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, points.size());
    }

private:
    std::vector<glm::vec3> points;
    GLuint VAO, VBO;

    void loadVDBData(const std::string &filename)
    {
        openvdb::initialize();
        openvdb::io::File file(filename);
        file.open();

        openvdb::GridBase::Ptr baseGrid;

        // 用于生成随机扰动的随机数引擎
        std::default_random_engine generator;
        std::uniform_real_distribution<float> distribution(RANDOM_OFFSET, RANDOM_OFFSET); // 随机偏移范围

        for (openvdb::io::File::NameIterator nameIter = file.beginName(); nameIter != file.endName(); ++nameIter)
        {
            baseGrid = file.readGrid(nameIter.gridName());
            openvdb::FloatGrid::Ptr floatGrid = openvdb::gridPtrCast<openvdb::FloatGrid>(baseGrid);

            if (!floatGrid)
            {
                continue;
            }

            for (openvdb::FloatGrid::ValueOnCIter iter = floatGrid->cbeginValueOn(); iter.test(); ++iter)
            {
                if (iter.getValue() < MIN_DENSITY) // 过滤掉密度较小的点
                    continue;
                openvdb::Coord coord = iter.getCoord();

                // 添加随机扰动，使点的分布更自然
                float offsetX = distribution(generator);
                float offsetY = distribution(generator);
                float offsetZ = distribution(generator);

                points.emplace_back(coord.x() + offsetX, coord.y() + offsetY, coord.z() + offsetZ);
            }
        }
        file.close();
    }

    void setupOpenGL()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(glm::vec3), points.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void sortPointsByDepth(const glm::vec3 &viewPos)
    {
        // 使用 lambda 表达式对点进行排序，依据是点到相机位置的距离
        std::sort(points.begin(), points.end(), [&viewPos](const glm::vec3 &a, const glm::vec3 &b)
                  {
                      float distA = glm::length(a - viewPos);
                      float distB = glm::length(b - viewPos);
                      return distA > distB; // 按距离从远到近排序
                  });
    }
};
