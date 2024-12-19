#ifndef LOAD_MODEL_H
#define LOAD_MODEL_H

#include <fluid/renderer/scene.h>
#include <fluid/renderer/camera.h>

namespace fluid {

    class LoadModel {
    public:
        /**
         * @brief 构造函数，初始化文件路径、缩放和偏移量
         * @param filepath 模型文件路径
         * @param scale 模型的缩放因子
         * @param offset 模型的偏移量
         */
        LoadModel();
        LoadModel(const std::string& filepath, double scale = 1.0, const vec3d& offset = vec3d(0.0, 0.0, 0.0));

        /**
        * @brief 加载新模型到mesh中
        * @param filepath 模型文件路径
        * @param scale 模型的缩放因子
        * @param offset 模型的偏移量
        */
        void addModel(const std::string& filepath, double scale = 1.0, const vec3d& offset = vec3d(0.0, 0.0, 0.0));

        /**
         * @brief 获取模型的 mesh
         * @return 加载的 mesh
         */
        const renderer::scene::mesh_t& get_mesh() const;

        /**
         * @brief 获取模型起始位置信息(网格偏移量)
         * @return 起始位置坐标offset
         */
        const vec3d& get_offset() const;

        /**
         * @brief 获取模型大小信息(网格大小)
         * @return 模型所占方形空间大小size
         */
        const vec3d& get_size() const;

        /**
         * @brief 打印模型信息，包括顶点数量和其他基本信息
         */
        void print_mesh_info() const;

    private:
        std::string filepath_;                // 文件路径
        double scale_;                        // 模型缩放因子
        vec3d offset_;                        // 模型偏移量
        renderer::scene::mesh_t mesh_;        // 存储加载的 mesh
        vec3d min_coords_, max_coords_, size_coords_;        // 用于存储模型的最小和最大坐标值

        /**
         * @brief 加载模型
         * @return 如果加载成功返回 true，否则返回 false
         */
        bool load();
    };

} // namespace fluid

#endif