#pragma once

#include "vector2d/VectorCharacter.h"
#include "gene/AppearanceGene.h"

// ============================================================
// 矢量图角色生成器接口
// ============================================================

namespace vector2d {

class IVectorCharacterGenerator {
public:
    virtual ~IVectorCharacterGenerator() = default;

    // 从外观基因生成矢量角色
    virtual VectorCharacter generate(const AppearanceGene& gene) = 0;

    // 生成多个方向的角色
    // direction: 0=下(南), 1=左(西), 2=上(北), 3=右(东)
    virtual std::vector<VectorCharacter> generateDirections(
        const AppearanceGene& gene,
        int num_directions = 4
    ) = 0;
};

} // namespace vector2d
