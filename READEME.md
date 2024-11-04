## 相机操作：
- A：左移
- S：下移
- D：右移
- W：上移
- 摁住鼠标右键拖拽：转换视角
- 滚轮：前后移动

万向节死锁现象只是简单的限制了一下角度可能有点奇怪那里

## 代码规范
- 所有的 class 名称大写并使用驼峰命名法
- 所有函数，变量，文件名，请不要使用驼峰，下划线分割！！！
- task文件夹下所有具体类请用命名空间GL_TASK
- 类里面的函数之间需要空一格，变量不需要
- 参考如下

```C++
class Scene
{
public:
    Scene(ShaderManager &shader_manager, LightManager light_manager) : shader_manager(shader_manager), light_manager(light_manager) {}

    virtual ~Scene() = default;

    virtual void render(const glm::mat4 &projection, const glm::mat4 &view, glm::vec3 &camera_pos) = 0;

protected:
    virtual void setup_scene() = 0;

    ShaderManager &shader_manager;
    LightManager light_manager;
    std::vector<std::shared_ptr<RenderableModel>> models;
};
```