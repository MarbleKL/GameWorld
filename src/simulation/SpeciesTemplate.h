#pragma once

#include "core/Types.h"
#include "gene/GameplayGene.h"
#include <string>
#include <vector>

// ============================================================
// 物种模板定义
// 定义物种的基因分布、生命周期参数、生态位
// ============================================================

struct SpeciesTemplate {
    SpeciesId id;
    std::string name;

    // 种群动力学参数
    float base_birth_rate;       // 基础出生率（每天百分比）
    float base_death_rate;       // 基础死亡率（每天百分比）
    float food_requirement;      // 每个个体的食物需求

    // 捕食关系
    std::vector<SpeciesId> prey_species;  // 捕食的物种列表
    float hunt_efficiency;                // 捕猎效率（每天每个捕食者杀死猎物的比例）

    // 基因分布参数（用于生成个体）
    float limb_length_mean, limb_length_std;
    float body_mass_mean, body_mass_std;
    float size_scale_mean, size_scale_std;
    float base_strength_mean, base_strength_std;
    float base_agility_mean, base_agility_std;
    float base_endurance_mean, base_endurance_std;
    float base_intellect_mean, base_intellect_std;

    // 生命周期参数
    float maturity_age;          // 成熟年龄（天）
    float average_lifespan;      // 平均寿命（天）

    // 环境适应性
    float optimal_temperature;   // 最适温度
    float temperature_tolerance; // 温度耐受范围
};

// 预定义物种模板
namespace species_templates {

// 物种1：兔子（猎物，快速繁殖）
inline SpeciesTemplate rabbit() {
    SpeciesTemplate s;
    s.id = 1;
    s.name = "Rabbit";

    // 种群参数
    s.base_birth_rate = 0.30f;       // 30%每天
    s.base_death_rate = 0.10f;       // 10%每天
    s.food_requirement = 1.0f;

    // 不捕食其他物种
    s.prey_species = {};
    s.hunt_efficiency = 0.0f;

    // 基因分布（小体型）
    s.limb_length_mean = 0.5f; s.limb_length_std = 0.1f;
    s.body_mass_mean = 0.3f; s.body_mass_std = 0.05f;
    s.size_scale_mean = 0.8f; s.size_scale_std = 0.1f;
    s.base_strength_mean = 3.0f; s.base_strength_std = 0.5f;
    s.base_agility_mean = 7.0f; s.base_agility_std = 1.0f;
    s.base_endurance_mean = 5.0f; s.base_endurance_std = 1.0f;
    s.base_intellect_mean = 2.0f; s.base_intellect_std = 0.5f;

    // 生命周期
    s.maturity_age = 5.0f;           // 5天成熟
    s.average_lifespan = 50.0f;      // 50天寿命

    // 环境适应
    s.optimal_temperature = 20.0f;
    s.temperature_tolerance = 15.0f;

    return s;
}

// 物种2：狼（捕食者，慢速繁殖）
inline SpeciesTemplate wolf() {
    SpeciesTemplate s;
    s.id = 2;
    s.name = "Wolf";

    // 种群参数
    s.base_birth_rate = 0.05f;       // 5%每天
    s.base_death_rate = 0.03f;       // 3%每天
    s.food_requirement = 5.0f;       // 需要更多食物

    // 捕食兔子
    s.prey_species = {1};            // 捕食物种ID=1（兔子）
    s.hunt_efficiency = 0.02f;       // 每天每只狼杀死2%的猎物

    // 基因分布（大体型）
    s.limb_length_mean = 1.2f; s.limb_length_std = 0.2f;
    s.body_mass_mean = 1.5f; s.body_mass_std = 0.3f;
    s.size_scale_mean = 1.5f; s.size_scale_std = 0.2f;
    s.base_strength_mean = 8.0f; s.base_strength_std = 1.5f;
    s.base_agility_mean = 6.0f; s.base_agility_std = 1.0f;
    s.base_endurance_mean = 7.0f; s.base_endurance_std = 1.0f;
    s.base_intellect_mean = 5.0f; s.base_intellect_std = 1.0f;

    // 生命周期
    s.maturity_age = 20.0f;          // 20天成熟
    s.average_lifespan = 150.0f;     // 150天寿命

    // 环境适应
    s.optimal_temperature = 15.0f;
    s.temperature_tolerance = 20.0f;

    return s;
}

// 物种3：熊（杂食，可选）
inline SpeciesTemplate bear() {
    SpeciesTemplate s;
    s.id = 3;
    s.name = "Bear";

    // 种群参数
    s.base_birth_rate = 0.08f;       // 8%每天
    s.base_death_rate = 0.04f;       // 4%每天
    s.food_requirement = 3.0f;

    // 可以捕食兔子和狼
    s.prey_species = {1, 2};
    s.hunt_efficiency = 0.015f;      // 1.5%效率

    // 基因分布（最大体型）
    s.limb_length_mean = 1.5f; s.limb_length_std = 0.3f;
    s.body_mass_mean = 2.0f; s.body_mass_std = 0.5f;
    s.size_scale_mean = 2.0f; s.size_scale_std = 0.3f;
    s.base_strength_mean = 10.0f; s.base_strength_std = 2.0f;
    s.base_agility_mean = 4.0f; s.base_agility_std = 1.0f;
    s.base_endurance_mean = 9.0f; s.base_endurance_std = 1.5f;
    s.base_intellect_mean = 6.0f; s.base_intellect_std = 1.0f;

    // 生命周期
    s.maturity_age = 30.0f;          // 30天成熟
    s.average_lifespan = 200.0f;     // 200天寿命

    // 环境适应
    s.optimal_temperature = 10.0f;
    s.temperature_tolerance = 25.0f;

    return s;
}

} // namespace species_templates
