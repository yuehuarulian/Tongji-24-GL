#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <openvdb/openvdb.h>
#include <openvdb/io/File.h>
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "camera_control.hpp"
#include "skybox.hpp"

// 顶点着色器代码
const char *vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    uniform mat4 model;
    void main() {
        gl_Position = model * vec4(aPos, 1.0);
    }
)glsl";

// 片段着色器代码
const char *fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色
    }
)glsl";

// 初始化OpenGL窗口
GLFWwindow *initOpenGL()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "OpenVDB Cloud Visualization", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);
    return window;
}

// 从VDB文件中读取体积数据并返回一个点的向量
std::vector<openvdb::Vec3s> loadVDBData(const std::string &filename)
{
    openvdb::initialize();
    openvdb::io::File file(filename);
    file.open();

    openvdb::GridBase::Ptr baseGrid;
    std::vector<openvdb::Vec3s> points;

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
            if (*iter > 0.2)
            { // 根据密度筛选点
                openvdb::Vec3s coord = iter.getCoord().asVec3s();
                // 缩放坐标到 [-1, 1] 范围
                coord = (coord / 50.0f);
                points.push_back(coord);
            }
        }
    }
    file.close();
    return points;
}

// 编译着色器
GLuint compileShader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Error: Shader Compilation Failed\n"
                  << infoLog << std::endl;
    }
    return shader;
}

// 创建着色器程序
GLuint createShaderProgram()
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Error: Shader Linking Failed\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// 渲染点云
void renderPointCloud(const std::vector<openvdb::Vec3s> &points, GLuint shaderProgram)
{
    // 创建VAO和VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(openvdb::Vec3s), points.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 创建模型变换矩阵
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::scale(model, glm::vec3(0.02f)); // 缩小点云，使其适应显示窗口

    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // 绘制点云
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, points.size());

    // 清理
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
}

int main()
{
    // 初始化OpenGL
    GLFWwindow *window = initOpenGL();

    Camera camera(window);

    // 加载VDB数据
    std::cout << "Loading VDB data..." << std::endl;
    std::vector<openvdb::Vec3s> points = loadVDBData("./cloud.vdb");
    std::cout << "Loaded " << points.size() << " points" << std::endl;

    GLuint shaderProgram = createShaderProgram();

    while (!glfwWindowShouldClose(window))
    {
        // 清空屏幕
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 渲染点云
        renderPointCloud(points, shaderProgram);

        // 交换缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
