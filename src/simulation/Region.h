#pragma once

#include "core/Types.h"
#include <string>
#include <vector>

// ============================================================
// Region - 空间区域定义
// 用于种群统计和HQ/LQ模式切换
// ============================================================

struct Region {
    uint32_t id;
    std::string name;

    // 环境参数
    float food_capacity;     // 最大食物容量（影响承载力）
    float current_food;      // 当前食物量（自然恢复）
    float temperature;       // 温度（影响不同物种的生存率）

    // HQ/LQ模式管理
    enum class Mode {
        LQ,  // 低精度：种群统计模拟
        HQ   // 高精度：个体模拟
    };
    Mode mode;
    Mode target_mode;  // 目标模式（ConversionSystem会将mode转换到target_mode）

    // 连接关系
    std::vector<uint32_t> neighbors;  // 相邻Region ID（用于迁移）

    // 构造函数
    Region()
        : id(0), name(""), food_capacity(1000.0f), current_food(1000.0f),
          temperature(20.0f), mode(Mode::LQ), target_mode(Mode::LQ) {}

    Region(uint32_t id, const std::string& name, float food_cap, float temp)
        : id(id), name(name), food_capacity(food_cap), current_food(food_cap),
          temperature(temp), mode(Mode::LQ), target_mode(Mode::LQ) {}
};
