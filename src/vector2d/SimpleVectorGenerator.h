#pragma once

#include "vector2d/IVectorCharacterGenerator.h"

// ============================================================
// 简单矢量图角色生成器
// 生成基于几何形状的简化人形角色
// ============================================================

namespace vector2d {

class SimpleVectorGenerator : public IVectorCharacterGenerator {
public:
    SimpleVectorGenerator();

    VectorCharacter generate(const AppearanceGene& gene) override;

    std::vector<VectorCharacter> generateDirections(
        const AppearanceGene& gene,
        int num_directions = 4
    ) override;

private:
    // 生成指定方向的角色
    VectorCharacter generateWithDirection(const AppearanceGene& gene, int direction);

    // 绘制各部位
    void addShadow(VectorCharacter& character, const AppearanceGene& gene, int direction);
    void addLegs(VectorCharacter& character, const AppearanceGene& gene, int direction);
    void addBody(VectorCharacter& character, const AppearanceGene& gene, int direction);
    void addArms(VectorCharacter& character, const AppearanceGene& gene, int direction);
    void addHead(VectorCharacter& character, const AppearanceGene& gene, int direction);
    void addFace(VectorCharacter& character, const AppearanceGene& gene, int direction);

    // 颜色生成
    Color getSkinColor(float base_tone) const;
    Color getBodyColor(float base_tone, float pattern_variation) const;
    Color getHairColor(float base_tone) const;

    // 画布尺寸
    float canvas_width_;
    float canvas_height_;
};

} // namespace vector2d
