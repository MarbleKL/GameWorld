#pragma once

#include "core/Types.h"
#include <vector>
#include <string>

// ============================================================
// 外观基因（AppearanceGene）
// 只用于外观生成，从GameplayGene映射而来，再加少量随机
// ============================================================

struct AppearanceGene {
    SpeciesId species_id;
    uint32_t  seed;      // 可由 GameplayGene.seed 派生

    // ========== 从 GameplayGene（共享字段）映射 + 随机扰动 ==========
    float height;        // ~ limb_length + size_scale
    float fatness;       // ~ body_mass / height
    float musculature;   // ~ base_strength 映射为视觉肌肉感

    // ========== 完全视觉用字段（不参与逻辑） ==========
    float base_tone;          // 皮肤/毛发基础颜色
    float pattern_variation;  // 花纹变化
    float scar_level;         // 伤疤程度
    
    // 发型/服装/饰品标签，可由势力/职业决定（未来扩展）
    std::vector<std::string> style_tags;
};
