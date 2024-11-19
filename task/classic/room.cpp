#include "room.hpp"
#include "glm/gtx/transform.hpp"
#include "error.hpp"

GL_TASK::Room::Room(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma)
    : RenderableModel(model_path, std::move(shader), gamma)
{
    calculateBoundingBox();
}

void GL_TASK::Room::draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos)
{
    checkGLError("room draw");

    shader->use();
    shader->setMat4("projection", projection);
    shader->setMat4("view", view);
    shader->setMat4("model", model_matrix);
    shader->setVec3("camPos", camera_pos);

    model.Draw(*shader);
    // checkOpenGLError("After model draw");
}

void GL_TASK::Room::calculateBoundingBox()
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile("source/model/room/overall.obj", aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->HasMeshes())
    {
        throw std::runtime_error("Failed to load model for calculating bounding box.");
    }

    // 遍历模型中的每一个网格
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh *mesh = scene->mMeshes[i];
        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            aiVector3D vertex = mesh->mVertices[j];
            glm::vec3 pos = glm::vec3(vertex.x, vertex.y, vertex.z);

            // 更新 min 和 max 边界
            min_bound = glm::min(min_bound, pos);
            max_bound = glm::max(max_bound, pos);
        }
    }
}

void GL_TASK::Room::getBoundingBox(glm::vec3 &min, glm::vec3 &max)
{
    min = min_bound;
    max = max_bound;
}