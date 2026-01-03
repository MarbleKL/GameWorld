#pragma once

#include "core/Types.h"
#include <variant>
#include <string>

// ============================================================
// Effect 定义 - 世界状态变化的原子记录
// 所有Process产生Effect，由EffectRecorder收集后统一应用
// ============================================================

namespace effect {

// Entity创建
struct EntityCreated {
    EntityId entity_id;
    EntityType type;
    std::string description;  // 可选描述
};

// Entity销毁
struct EntityDestroyed {
    EntityId entity_id;
    std::string reason;  // 销毁原因
};

// Resource变化（泛用）
struct ResourceChanged {
    EntityId entity_id;
    std::string resource_name;  // 如 "health", "hunger", "estimated_count"
    float old_value;
    float new_value;
};

// 迁移事件
struct Migration {
    EntityId entity_id;      // Population或Creature
    uint32_t from_region;
    uint32_t to_region;
    float migrant_count;     // Population迁移数量，Creature为1.0
};

// 死亡事件
struct Death {
    EntityId entity_id;
    std::string cause;       // "starvation", "old_age", "predation"
};

// 繁殖事件
struct Reproduction {
    EntityId parent_id;
    EntityId child_id;
    SpeciesId species_id;
};

// 组件添加（用于日志）
struct ComponentAdded {
    EntityId entity_id;
    std::string component_type;
};

// Effect变体
using Effect = std::variant<
    EntityCreated,
    EntityDestroyed,
    ResourceChanged,
    Migration,
    Death,
    Reproduction,
    ComponentAdded
>;

} // namespace effect
