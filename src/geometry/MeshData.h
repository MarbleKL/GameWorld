#pragma once

#include <vector>
#include <string>
#include <cstdint>

// ============================================================
// 引擎无关的几何数据结构
// 纯数据，不包含任何渲染API类型
// ============================================================

struct Vec2 {
    float x, y;
};

struct Vec3 {
    float x, y, z;
};

struct Vec4 {
    float x, y, z, w;
};

struct Color {
    float r, g, b, a;
};

struct Mat4 {
    float m[16];
};

// 顶点权重（用于骨骼蒙皮）
struct VertexWeight {
    uint16_t bone_indices[4];
    float    weights[4];
};

// Mesh数据
struct MeshData {
    std::vector<Vec3>          positions;
    std::vector<Vec3>          normals;
    std::vector<Vec2>          uvs;
    std::vector<Color>         colors;        // 可选
    std::vector<Vec4>          tangents;      // 可选
    std::vector<VertexWeight>  skin;          // 可选（骨骼蒙皮）
    std::vector<uint32_t>      indices;
};

// 骨骼定义
struct Bone {
    std::string name;
    int         parent;    // -1 表示根骨骼
    Mat4        bind_pose;
};

// 骨架
struct Skeleton {
    std::vector<Bone> bones;
};

// Avatar完整包（骨架 + 多个Mesh）
struct AvatarBundle {
    Skeleton                skeleton;
    std::vector<MeshData>   meshes;
};
