#pragma once

#include "gene/GameplayGene.h"
#include "gene/AppearanceGene.h"
#include "core/Types.h"
#include "geometry/MeshData.h"

// ============================================================
// ECS组件定义
// 使用 namespace component 命名空间，避免C_前缀
// ============================================================

namespace component {

// 逻辑基因组件
struct GameplayGene {
    ::GameplayGene gene;
};

// 外观组件（包含外观基因和生成的Avatar ID）
struct Appearance {
    ::AppearanceGene gene;
    AvatarId         avatar_id;  // 几何生成+上传后的句柄（0表示尚未生成）
};

// 数值统计组件（预留，未来由GameplayGene计算得出）
struct Stats {
    float attack_power;
    float move_speed;
    float max_health;
    // ... 更多数值
};

// 种群组件（预留，用于Population实体）
struct Population {
    SpeciesId species_id;
    uint32_t  region_id;         // 所在区域ID
    uint32_t  estimated_count;   // 估计数量
    float     birth_rate;
    float     death_rate;

    enum class Mode {
        Simulated,              // 种群级模拟（LQ区）
        DerivedFromIndividuals  // 从个体统计（HQ区）
    };
    Mode mode;

    // 统计数据（从个体或模拟得出）
    float avg_limb_length;
    float avg_body_mass;
    float avg_size_scale;

    // 统计分布（用于采样）
    float std_limb_length;
    float std_body_mass;
    float std_size_scale;
};

// 位置组件（用于Creature和Population）
struct Position {
    uint32_t region_id;
    Vec3 local_pos;  // 区域内的局部位置
};

// 生命周期组件（用于Creature）
struct Lifecycle {
    float age;           // 当前年龄（天）
    float lifespan;      // 预期寿命（天）
    float hunger;        // 饥饿度 0.0-1.0（>0.95死亡）
    float health;        // 健康度 0.0-1.0（<0.05死亡）
};

// 物种引用组件（用于Creature）
struct SpeciesRef {
    SpeciesId species_id;
};

} // namespace component
