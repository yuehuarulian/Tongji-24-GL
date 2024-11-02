#include "render_manager.hpp"

int main()
{
    RenderManager render_manager(2160, 1440, 120);
    render_manager.start_rendering(true); // 参数 `true` 表示离线渲染
    return 0;
}
