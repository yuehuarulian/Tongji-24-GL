#include <GLFW/glfw3.h>
#include <btBulletDynamicsCommon.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <cmath>

struct Boat
{
    btRigidBody *rigidBody;
};

Boat boat;
btDiscreteDynamicsWorld *dynamicsWorld;
const aiScene *boatModel;
Assimp::Importer importer;

void initBullet()
{
    // Bullet initialization
    btBroadphaseInterface *broadphase = new btDbvtBroadphase();
    btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));

    // Create ground
    btCollisionShape *groundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);
    btDefaultMotionState *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));
    btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, groundShape, btVector3(0, 0, 0));
    btRigidBody *groundRigidBody = new btRigidBody(groundRigidBodyCI);
    dynamicsWorld->addRigidBody(groundRigidBody);

    // Create boat
    btCollisionShape *boatShape = new btBoxShape(btVector3(0.5f, 0.5f, 0.5f));
    btDefaultMotionState *boatMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 5, 0)));
    btScalar mass = 1.0f;
    btVector3 boatInertia(0, 0, 0);
    boatShape->calculateLocalInertia(mass, boatInertia);
    btRigidBody::btRigidBodyConstructionInfo boatRigidBodyCI(mass, boatMotionState, boatShape, boatInertia);
    boat.rigidBody = new btRigidBody(boatRigidBodyCI);
    dynamicsWorld->addRigidBody(boat.rigidBody);
}

void loadBoatModel(const std::string &path)
{
    boatModel = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!boatModel || boatModel->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !boatModel->mRootNode)
    {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
}

void updatePhysics(float deltaTime)
{
    dynamicsWorld->stepSimulation(deltaTime, 10);
}

void renderModel(const aiScene *scene, const aiNode *node)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        glBegin(GL_TRIANGLES);
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
            {
                unsigned int index = face.mIndices[k];
                aiVector3D vertex = mesh->mVertices[index];
                glVertex3f(vertex.x, vertex.y, vertex.z);
            }
        }
        glEnd();
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        renderModel(scene, node->mChildren[i]);
    }
}

void renderBoat()
{
    btTransform trans;
    boat.rigidBody->getMotionState()->getWorldTransform(trans);
    float mat[16];
    trans.getOpenGLMatrix(mat);

    glPushMatrix();
    glMultMatrixf(mat);
    glScalef(0.1f, 0.1f, 0.1f); // Scale down the model to make sure it fits in the view
    if (boatModel)
    {
        renderModel(boatModel, boatModel->mRootNode);
    }
    glPopMatrix();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Adjust camera to better view the boat

    // Render boat
    renderBoat();

    glfwSwapBuffers(glfwGetCurrentContext());
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow *window = glfwCreateWindow(800, 600, "Boat Simulation with Bullet Physics", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glEnable(GL_DEPTH_TEST);

    initBullet();
    loadBoatModel("path/to/your/boat.obj"); // Replace with the actual path to your OBJ file

    while (!glfwWindowShouldClose(window))
    {
        updatePhysics(0.016f); // Simulate ~60FPS
        display();
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
