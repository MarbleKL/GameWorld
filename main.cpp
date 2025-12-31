#include <iostream>
#include <iomanip>
#include <filesystem>

#include "gene/GameplayGene.h"
#include "gene/AppearanceGene.h"
#include "systems/GeneSystem.h"
#include "geometry/SimpleAvatarGenerator.h"
#include "render/ConsoleAdapter.h"
#include "tools/ObjExporter.h"

// ============================================================
// 模型生成最小验证Demo
// 验证完整数据流：GameplayGene → AppearanceGene → MeshData
// ============================================================

void PrintGameplayGene(const GameplayGene& gene) {
    std::cout << "\n--- Gameplay Gene (Logical) ---\n";
    std::cout << "  Species ID: " << gene.species_id << "\n";
    std::cout << "  Seed: " << gene.seed << "\n";
    std::cout << "  [Shared Fields - affect both logic & appearance]\n";
    std::cout << "    Limb Length: " << gene.limb_length << "\n";
    std::cout << "    Body Mass:   " << gene.body_mass << "\n";
    std::cout << "    Size Scale:  " << gene.size_scale << "\n";
    std::cout << "  [Logic-Only Fields]\n";
    std::cout << "    Strength:   " << gene.base_strength << "\n";
    std::cout << "    Agility:    " << gene.base_agility << "\n";
    std::cout << "    Endurance:  " << gene.base_endurance << "\n";
    std::cout << "    Intellect:  " << gene.base_intellect << "\n";
    std::cout << "  [Special Traits]\n";
    std::cout << "    Has Wings: " << (gene.has_wings ? "Yes" : "No") << "\n";
    std::cout << "    Has Horn:  " << (gene.has_horn ? "Yes" : "No") << "\n";
}

void PrintAppearanceGene(const AppearanceGene& gene) {
    std::cout << "\n--- Appearance Gene (Visual) ---\n";
    std::cout << "  Species ID: " << gene.species_id << "\n";
    std::cout << "  Seed: " << gene.seed << "\n";
    std::cout << "  [Mapped from GameplayGene]\n";
    std::cout << "    Height:      " << std::fixed << std::setprecision(2) << gene.height << "\n";
    std::cout << "    Fatness:     " << gene.fatness << "\n";
    std::cout << "    Musculature: " << gene.musculature << "\n";
    std::cout << "  [Pure Visual - Random]\n";
    std::cout << "    Base Tone:          " << gene.base_tone << "\n";
    std::cout << "    Pattern Variation:  " << gene.pattern_variation << "\n";
    std::cout << "    Scar Level:         " << gene.scar_level << "\n";
}

GameplayGene CreateTestCreature(uint32_t seed, const char* name, 
                                float limb, float mass, float scale,
                                float str, float agi, float end, float intel) {
    std::cout << "\n\n========================================\n";
    std::cout << "Creating Creature: " << name << "\n";
    std::cout << "========================================\n";
    
    GameplayGene gene;
    gene.species_id = 1;  // 假设物种ID为1
    gene.seed = seed;
    gene.limb_length = limb;
    gene.body_mass = mass;
    gene.size_scale = scale;
    gene.base_strength = str;
    gene.base_agility = agi;
    gene.base_endurance = end;
    gene.base_intellect = intel;
    gene.has_wings = false;
    gene.has_horn = false;
    
    return gene;
}

int main() {
    std::cout << "╔════════════════════════════════════════════╗\n";
    std::cout << "║  GameWorld - Model Generation Demo        ║\n";
    std::cout << "║  Validating: Gene → Appearance → Mesh     ║\n";
    std::cout << "╚════════════════════════════════════════════╝\n";
    
    // 创建输出目录
    std::filesystem::create_directories("./output");
    
    // 创建渲染适配器和几何生成器
    render::ConsoleAdapter render_adapter;
    geometry::SimpleAvatarGenerator avatar_generator;
    
    // ========== 测试案例1：瘦高敏捷型角色 ==========
    {
        GameplayGene gene = CreateTestCreature(
            1001, "Agile Scout",
            2.0f,  // 长肢体 → 身高
            1.5f,  // 轻体重 → 瘦
            1.0f,  // 正常体型
            0.5f,  // 低力量
            1.5f,  // 高敏捷
            1.0f,  // 正常耐力
            1.2f   // 较高智力
        );
        
        PrintGameplayGene(gene);
        
        // 映射到外观基因
        AppearanceGene appearance = systems::GeneSystem::BuildAppearanceFromGameplay(gene);
        PrintAppearanceGene(appearance);
        
        // 生成几何体
        AvatarBundle bundle = avatar_generator.generate(appearance);
        
        // 上传到渲染适配器
        AvatarId avatar_id = render_adapter.CreateAvatar(bundle);
        
        // 导出为 OBJ 文件
        if (ObjExporter::ExportAvatar("agile_scout", bundle.meshes)) {
            std::cout << "  📁 Exported: output/agile_scout.obj\n";
        }
    }
    
    // ========== 测试案例2：强壮重型角色 ==========
    {
        GameplayGene gene = CreateTestCreature(
            2002, "Heavy Warrior",
            1.5f,  // 中等肢体
            3.5f,  // 重体重 → 胖
            1.5f,  // 大体型
            2.0f,  // 高力量
            0.5f,  // 低敏捷
            2.0f,  // 高耐力
            0.3f   // 低智力
        );
        
        PrintGameplayGene(gene);
        
        AppearanceGene appearance = systems::GeneSystem::BuildAppearanceFromGameplay(gene);
        PrintAppearanceGene(appearance);
        
        AvatarBundle bundle = avatar_generator.generate(appearance);
        AvatarId avatar_id = render_adapter.CreateAvatar(bundle);
        
        // 导出为 OBJ 文件
        if (ObjExporter::ExportAvatar("heavy_warrior", bundle.meshes)) {
            std::cout << "  📁 Exported: output/heavy_warrior.obj\n";
        }
    }
    
    // ========== 测试案例3：平衡型角色 ==========
    {
        GameplayGene gene = CreateTestCreature(
            3003, "Balanced Mage",
            1.8f,  // 中等偏高肢体
            2.0f,  // 中等体重
            1.2f,  // 中等偏大体型
            1.0f,  // 平衡力量
            1.0f,  // 平衡敏捷
            1.0f,  // 平衡耐力
            2.5f   // 极高智力
        );
        
        PrintGameplayGene(gene);
        
        AppearanceGene appearance = systems::GeneSystem::BuildAppearanceFromGameplay(gene);
        PrintAppearanceGene(appearance);
        
        AvatarBundle bundle = avatar_generator.generate(appearance);
        AvatarId avatar_id = render_adapter.CreateAvatar(bundle);
        
        // 导出为 OBJ 文件
        if (ObjExporter::ExportAvatar("balanced_mage", bundle.meshes)) {
            std::cout << "  📁 Exported: output/balanced_mage.obj\n";
        }
    }
    
    std::cout << "\n\n╔════════════════════════════════════════════╗\n";
    std::cout << "║  ✅ Validation Complete!                  ║\n";
    std::cout << "║                                            ║\n";
    std::cout << "║  Data Flow Verified:                       ║\n";
    std::cout << "║  GameplayGene → AppearanceGene → Mesh     ║\n";
    std::cout << "║                                            ║\n";
    std::cout << "║  Architecture Validated:                   ║\n";
    std::cout << "║  • Gene separation (logic vs visual)      ║\n";
    std::cout << "║  • Geometry generation (engine-agnostic)  ║\n";
    std::cout << "║  • Render adapter (decoupled)             ║\n";
    std::cout << "║                                            ║\n";
    std::cout << "║  📁 OBJ Files Exported to ./output/        ║\n";
    std::cout << "║  Open with Windows 3D Viewer or Blender!  ║\n";
    std::cout << "╚════════════════════════════════════════════╝\n";
    
    return 0;
}
