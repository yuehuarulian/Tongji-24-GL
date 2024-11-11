#include "render_manager.hpp"

const unsigned int WINDOW_HEIGHT = 1080;
const unsigned int WINDOW_WIDTH = 720;
const unsigned int FRAMES = 60;

int main()
{
    RenderManager render_manager(WINDOW_WIDTH, WINDOW_HEIGHT, FRAMES);
    render_manager.start_rendering(true); // 参数 `true` 表示离线渲染
    return 0;
}
