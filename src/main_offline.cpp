#include "render_manager.hpp"
#include "config.hpp"

int main()
{
    RenderManager render_manager(WINDOW_WIDTH, WINDOW_HEIGHT, 300);
    render_manager.start_rendering(true); // 参数 `true` 表示离线渲染
    return 0;
}
