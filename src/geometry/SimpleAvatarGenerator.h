#pragma once

#include "geometry/IAvatarGenerator.h"
#include "geometry/MeshData.h"

// ============================================================
// 简单Avatar生成器
// 使用基础几何体（立方体/球体）组合生成角色模型
// ============================================================

namespace geometry {

class SimpleAvatarGenerator : public IAvatarGenerator {
public:
    AvatarBundle generate(const AppearanceGene& gene) override;

private:
    // 生成身体（躯干立方体，受fatness影响宽度和厚度）
    MeshData GenerateBody(const AppearanceGene& gene);

    // 生成头部（球体，受musculature略微影响大小）
    MeshData GenerateHead(const AppearanceGene& gene);

    // 生成四肢（4个圆柱体：2条手臂 + 2条腿）
    MeshData GenerateLimbs(const AppearanceGene& gene);

    // 辅助函数：生成单个肢体（圆柱体）
    void GenerateLimb(MeshData& mesh, Vec3 start, Vec3 end, float thickness, int vertex_offset);

    // 辅助函数：生成眼球（球体）
    void GenerateEye(MeshData& mesh, Vec3 center, float radius, int vertex_offset);

    // 辅助函数：生成耳朵（C形几何体）
    void GenerateEar(MeshData& mesh, Vec3 position, float size, bool is_left, int vertex_offset);
};

} // namespace geometry
