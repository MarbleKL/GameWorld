#pragma once

#include "core/Types.h"

// ============================================================
// 逻辑基因（GameplayGene）
// 参与数值计算、物理行为、AI，是基因的权威来源
// ============================================================

struct GameplayGene {
    SpeciesId species_id;
    uint32_t  seed;          // 个体基因随机源

    // ========== [shared] 逻辑+视觉都会用的字段 ==========
    float limb_length;       // 四肢长度 → 攻击距离/步幅 + 外观身高/手脚比例
    float body_mass;         // 体重 → 惯性/撞击 + 外观胖瘦感
    float size_scale;        // 整体尺度 → 碰撞体/体型 + 模型缩放

    // ========== [logic-only] 只用于数值/AI，不直接映射到几何 ==========
    float base_strength;     // 力量：攻击力、负重
    float base_agility;      // 敏捷：移动速度、闪避
    float base_endurance;    // 体力：耐久、精力
    float base_intellect;    // 智力：技能、魔法、策略

    // 物种特有的遗传开关
    bool  has_wings;         // 会不会飞
    bool  has_horn;          // 有没有角（撞击攻击）
};
