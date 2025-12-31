#pragma once

#include <cstdint>

// ============================================================
// 核心类型定义
// 为ERPE框架（Entity/Resource/Process/Effect）预留
// ============================================================

using EntityId = uint64_t;
using SpeciesId = uint32_t;
using AvatarId = uint64_t;

// 实体类型枚举
enum class EntityType {
    Creature,      // 个体生物
    Population,    // 种群统计实体（未来扩展）
    Faction,       // 势力（未来扩展）
    Location,      // 地点（未来扩展）
    // 未来可扩展: JobSlot, Quest, Shop等
};
