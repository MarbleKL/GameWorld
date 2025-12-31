#pragma once

#include "gene/GameplayGene.h"
#include "gene/AppearanceGene.h"
#include <vector>
#include <string>
#include <cstdint>

// ============================================================
// 基因映射系统
// 从逻辑基因（GameplayGene）推导外观基因（AppearanceGene）
// ============================================================

namespace systems {

class GeneSystem {
public:
    // 从GameplayGene构建AppearanceGene
    // visual_seed: 用于纯视觉随机的种子（可以从gameplay_gene.seed派生）
    static AppearanceGene BuildAppearanceFromGameplay(
        const GameplayGene& gameplay_gene,
        uint32_t visual_seed = 0
    );
    
    // 辅助函数：为AppearanceGene添加样式标签
    // 未来可以根据Faction、职业等添加
    static void ApplyStyleTags(AppearanceGene& appearance, const std::vector<std::string>& tags);
};

} // namespace systems
