#ifndef BOAT_HPP
#define BOAT_HPP
#include "shader.hpp"
#include "renderable_model.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/glm.hpp"
#include <btBulletDynamicsCommon.h>
namespace GL_TASK
{
    class Boat : public RenderableModel
    {
    public:
        Boat(const std::string &model_path, std::shared_ptr<Shader> shader, bool gamma = false)
            : RenderableModel(model_path, std::move(shader), gamma), rigidBody(nullptr) {}

        void draw(const glm::mat4 &projection, const glm::mat4 &view, const glm::vec3 &camera_pos) override
        {
            if (rigidBody)
            {
                btTransform trans;
                rigidBody->getMotionState()->getWorldTransform(trans);
                float mat[16];
                trans.getOpenGLMatrix(mat);
                glm::mat4 M = glm::mat4(mat[0], mat[1], mat[2], mat[3],
                                        mat[4], mat[5], mat[6], mat[7],
                                        mat[8], mat[9], mat[10], mat[11],
                                        mat[12], mat[13], mat[14], mat[15]);

                shader->use();
                shader->setMat4("projection", projection);
                shader->setMat4("view", view);
                shader->setMat4("model", model_matrix);
                shader->setVec3("camPos", camera_pos);

                model.Draw(*shader);
            }
        }

        void setRigidBody(btRigidBody *body)
        {
            rigidBody = body;
        }

        btRigidBody *getRigidBody() const
        {
            return rigidBody;
        }

        void set_model_matrix(const glm::mat4 &model)
        {
            model_matrix = model;
        }

        glm::mat4 get_model_matrix() const
        {
            return model_matrix;
        }

    private:
        btRigidBody *rigidBody;
        glm::mat4 model_matrix = glm::mat4(1.0f);
    };
}
#endif // BOAT_HPP