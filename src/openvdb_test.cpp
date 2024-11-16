#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>

int main()
{
    // 初始化 OpenVDB
    openvdb::initialize();

    std::cout << "OpenVDB version: " << openvdb::getLibraryVersionString() << std::endl;
    // 创建一个简单的浮点 VDB 网格
    openvdb::FloatGrid::Ptr grid = openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(
        /* radius = */ 50.0,
        /* center = */ openvdb::Vec3f(0, 0, 0),
        /* voxel size = */ 1.0,
        /* width = */ 3.0);

    // 保存网格到文件
    openvdb::io::File file("my_test_grid.vdb");
    openvdb::GridPtrVec grids;
    grids.push_back(grid);
    file.write(grids);
    file.close();

    std::cout << "Successfully wrote my_test_grid.vdb" << std::endl;

    return 0;
}
