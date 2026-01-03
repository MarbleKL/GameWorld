#pragma once

#include "core/Types.h"

// ============================================================
// ECS Entity 定义
// Entity本质是ID，类型由组件决定
// ============================================================

namespace ecs {

// Entity只是一个ID的包装，类型信息存储在Registry中
struct Entity {
    EntityId id;
    EntityType type;

    Entity() : id(0), type(EntityType::Creature) {}
    Entity(EntityId id, EntityType type) : id(id), type(type) {}

    bool operator==(const Entity& other) const { return id == other.id; }
    bool operator!=(const Entity& other) const { return id != other.id; }
    bool operator<(const Entity& other) const { return id < other.id; }

    explicit operator bool() const { return id != 0; }
};

} // namespace ecs
