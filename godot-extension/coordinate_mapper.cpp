#include "coordinate_mapper.h"
#include <cmath>
#include <limits>

// Region 布局 (俯视图):
// ┌─────────┬─────────┬─────────┐
// │ Region1 │ Region2 │ Region3 │
// │ Forest  │ Plains  │ Hills   │
// │  (0,0)  │(100,0)  │(200,0)  │
// ├─────────┼─────────┼─────────┤
// │ Region4 │ Region5 │ Region6 │
// │ Swamp   │ River   │Mountain │
// │ (0,100) │(100,100)│(200,100)│
// └─────────┴─────────┴─────────┘

const std::map<uint32_t, Vector3> CoordinateMapper::REGION_CENTERS = {
    {1, Vector3(0, 0, 0)},           // Forest
    {2, Vector3(100, 0, 0)},         // Plains
    {3, Vector3(200, 0, 0)},         // Hills
    {4, Vector3(0, 0, 100)},         // Swamp
    {5, Vector3(100, 0, 100)},       // River
    {6, Vector3(200, 0, 100)}        // Mountain
};

Vector3 CoordinateMapper::to_godot_world(uint32_t region_id, float local_x, float local_z) {
    auto it = REGION_CENTERS.find(region_id);
    if (it == REGION_CENTERS.end()) {
        // 默认返回原坐标
        return Vector3(local_x, 0, local_z);
    }

    const Vector3& center = it->second;
    return Vector3(center.x + local_x, 0.0f, center.z + local_z);
}

uint32_t CoordinateMapper::find_nearest_region(const Vector3& world_pos) {
    uint32_t nearest_id = 1;
    float min_distance = std::numeric_limits<float>::max();

    for (const auto& [region_id, center] : REGION_CENTERS) {
        float dx = world_pos.x - center.x;
        float dz = world_pos.z - center.z;
        float distance = std::sqrt(dx * dx + dz * dz);

        if (distance < min_distance) {
            min_distance = distance;
            nearest_id = region_id;
        }
    }

    return nearest_id;
}

float CoordinateMapper::calculate_camera_distance(const Vector3& cam_pos, uint32_t region_id) {
    auto it = REGION_CENTERS.find(region_id);
    if (it == REGION_CENTERS.end()) {
        return 1000.0f;  // 默认极远距离 (LQ模式)
    }

    const Vector3& center = it->second;
    // 只计算 XZ 平面距离 (忽略高度)
    float dx = cam_pos.x - center.x;
    float dz = cam_pos.z - center.z;
    return std::sqrt(dx * dx + dz * dz);
}
