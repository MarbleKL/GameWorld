#include "systems/GeneSystem.h"
#include <cmath>
#include <random>

namespace systems {

AppearanceGene GeneSystem::BuildAppearanceFromGameplay(
    const GameplayGene& gameplay_gene,
    uint32_t visual_seed
) {
    AppearanceGene appearance;
    
    appearance.species_id = gameplay_gene.species_id;
    
    // 如果没有指定visual_seed，从gameplay_gene.seed派生
    appearance.seed = (visual_seed != 0) ? visual_seed : (gameplay_gene.seed + 1000);
    
    // ========== 从逻辑基因的[shared]字段映射到外观参数 ==========
    
    // 身高 = limb_length的主要贡献 + size_scale的辅助
    appearance.height = gameplay_gene.limb_length * 0.5f + gameplay_gene.size_scale * 0.5f;
    
    // 胖瘦感 = body_mass / height（避免除零）
    float safe_height = std::max(appearance.height, 0.1f);
    appearance.fatness = gameplay_gene.body_mass / safe_height;
    
    // 肌肉感 = 力量的视觉映射（简单线性映射）
    appearance.musculature = gameplay_gene.base_strength;
    
    // ========== 纯视觉随机字段（不影响逻辑） ==========
    
    std::mt19937 rng(appearance.seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    appearance.base_tone = dist(rng);          // 皮肤/毛发颜色
    appearance.pattern_variation = dist(rng);  // 花纹变化
    appearance.scar_level = dist(rng);         // 伤疤程度
    
    // style_tags可以由势力/职业决定（未来扩展）
    // 这里留空
    
    return appearance;
}

void GeneSystem::ApplyStyleTags(AppearanceGene& appearance, const std::vector<std::string>& tags) {
    appearance.style_tags = tags;
}

} // namespace systems
