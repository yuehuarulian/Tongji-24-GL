// PointCloud.hpp
#ifndef POINT_CLOUD_HPP
#define POINT_CLOUD_HPP

#include <random>
#include <vector>
#include <string>
#include <algorithm>
#include <openvdb/openvdb.h>
#include <openvdb/io/File.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "shader.hpp"

const float MIN_DENSITY = 0.0f;
const float RANDOM_OFFSET = 0.0f;

class PointCloud
{
public:
    PointCloud(const std::string &vdb_file, std::shared_ptr<Shader> shader);

    ~PointCloud();

    void draw(const glm::mat4 &projection = glm::mat4(1.0f), const glm::mat4 &view = glm::mat4(1.0f),
              const glm::vec3 &viewPos = glm::vec3(0.0f), const glm::vec3 &fogColor = glm::vec3(0.0f), float fogDensity = 0.0f, bool sort_points = false);

    void set_model_matrix(glm::mat4 &model)
    {
        this->model = model;
    }

private:
    std::shared_ptr<Shader> shader;
    std::vector<glm::vec3> points;
    glm::mat4 model = glm::mat4(1.0f);
    GLuint VAO, VBO;

    void load_vdb_data(const std::string &filename);
    void setup_opengl();
    void sort_points_by_depth(const glm::vec3 &viewPos);
};

#endif // POINT_CLOUD_HPP