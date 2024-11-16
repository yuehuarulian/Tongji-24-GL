#include <openvdb/openvdb.h>
#include <openvdb/tools/ValueTransformer.h>
#include <openvdb/tools/ValueTransformer.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <iostream>
#include <random>

// 定义一个简单的 Perlin 噪声函数
float perlinNoise(float x, float y, float z)
{
    return std::sin(x * 0.1f) * std::cos(y * 0.1f) * std::sin(z * 0.1f);
}

int main()
{
    // 初始化 OpenVDB 库
    openvdb::initialize();

    // 创建一个浮点网格（用于存储云的密度）
    openvdb::FloatGrid::Ptr densityGrid = openvdb::FloatGrid::create(0.0);

    // 定义网格的体素大小
    float voxelSize = 0.5f;
    densityGrid->setTransform(openvdb::math::Transform::createLinearTransform(voxelSize));

    // 设置云的体积密度范围（例如 -50 到 50 的体素范围）
    openvdb::FloatGrid::Accessor accessor = densityGrid->getAccessor();
    for (int x = -50; x <= 50; ++x)
    {
        for (int y = -50; y <= 50; ++y)
        {
            for (int z = -50; z <= 50; ++z)
            {
                // 使用 Perlin 噪声生成密度值
                float density = perlinNoise(x, y, z);

                // 将噪声值写入网格，注意我们只保留比较高的密度值以模拟云
                if (density > 0.2)
                {
                    accessor.setValue(openvdb::Coord(x, y, z), density);
                }
            }
        }
    }

    // 设置网格的一些元数据
    densityGrid->setGridClass(openvdb::GRID_FOG_VOLUME);
    densityGrid->setName("cloud");

    // 创建一个 OpenVDB 文件，并将网格保存进去
    openvdb::io::File file("cloud.vdb");
    openvdb::GridPtrVec grids;
    grids.push_back(densityGrid);
    file.write(grids);
    file.close();

    std::cout << "Successfully wrote cloud.vdb" << std::endl;

    return 0;
}
