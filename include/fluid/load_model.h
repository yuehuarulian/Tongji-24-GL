#ifndef LOAD_MODEL_H
#define LOAD_MODEL_H

#include <fluid/renderer/scene.h>
#include <fluid/renderer/camera.h>

namespace fluid {

    class LoadModel {
    public:
        /**
         * @brief ���캯������ʼ���ļ�·�������ź�ƫ����
         * @param filepath ģ���ļ�·��
         * @param scale ģ�͵���������
         * @param offset ģ�͵�ƫ����
         */
        LoadModel();
        LoadModel(const std::string& filepath, double scale = 1.0, const vec3d& offset = vec3d(0.0, 0.0, 0.0));

        /**
        * @brief ������ģ�͵�mesh��
        * @param filepath ģ���ļ�·��
        * @param scale ģ�͵���������
        * @param offset ģ�͵�ƫ����
        */
        void addModel(const std::string& filepath, double scale = 1.0, const vec3d& offset = vec3d(0.0, 0.0, 0.0));

        /**
         * @brief ��ȡģ�͵� mesh
         * @return ���ص� mesh
         */
        const renderer::scene::mesh_t& get_mesh() const;

        /**
         * @brief ��ȡģ����ʼλ����Ϣ(����ƫ����)
         * @return ��ʼλ������offset
         */
        const vec3d& get_offset() const;

        /**
         * @brief ��ȡģ�ʹ�С��Ϣ(�����С)
         * @return ģ����ռ���οռ��Сsize
         */
        const vec3d& get_size() const;

        /**
         * @brief ��ӡģ����Ϣ��������������������������Ϣ
         */
        void print_mesh_info() const;

    private:
        std::string filepath_;                // �ļ�·��
        double scale_;                        // ģ����������
        vec3d offset_;                        // ģ��ƫ����
        renderer::scene::mesh_t mesh_;        // �洢���ص� mesh
        vec3d min_coords_, max_coords_, size_coords_;        // ���ڴ洢ģ�͵���С���������ֵ

        /**
         * @brief ����ģ��
         * @return ������سɹ����� true�����򷵻� false
         */
        bool load();
    };

} // namespace fluid

#endif