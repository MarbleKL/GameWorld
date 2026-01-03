#pragma once

#include <godot_cpp/variant/vector3.hpp>
#include <map>

using namespace godot;

// ============================================================
// CoordinateMapper - 坐标系映射工具
// 将C++ ECS的Region局部坐标映射到Godot世界坐标
// ============================================================

class CoordinateMapper {
public:
    // Region 布局配置 (2x3网格, 每个100x100单位)
    static const std::map<uint32_t, Vector3> REGION_CENTERS;
    static constexpr float REGION_SIZE = 100.0f;

    // Region ID + 局部坐标 → Godot 世界坐标
    static Vector3 to_godot_world(uint32_t region_id, float local_x, float local_z);

    // Godot 世界坐标 → 最近的 Region ID
    static uint32_t find_nearest_region(const Vector3& world_pos);

    // 计算相机到 Region 的距离 (XZ平面)
    static float calculate_camera_distance(const Vector3& cam_pos, uint32_t region_id);
};
