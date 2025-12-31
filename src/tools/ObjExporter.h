#pragma once

#include "geometry/MeshData.h"
#include <string>
#include <vector>

// ============================================================
// OBJ 格式导出器
// 将 MeshData 导出为标准的 Wavefront OBJ 格式
// 同时生成 MTL 材质文件以支持顶点颜色
// ============================================================

class ObjExporter {
public:
    // 导出单个 Avatar（多个 Mesh 部件）
    // base_filename: 不带扩展名的基础文件名，如 "avatar_1"
    // 将生成 avatar_1.obj 和 avatar_1.mtl
    static bool ExportAvatar(
        const std::string& base_filename,
        const std::vector<MeshData>& meshes,
        const std::string& output_dir = "./output"
    );

private:
    // 计算 Mesh 的平均颜色
    static Color CalculateAverageColor(const MeshData& mesh);
};
