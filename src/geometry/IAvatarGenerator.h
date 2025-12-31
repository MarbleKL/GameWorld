#pragma once

#include "geometry/MeshData.h"
#include "gene/AppearanceGene.h"

// ============================================================
// Avatar生成器接口
// 从AppearanceGene生成几何数据（引擎无关）
// ============================================================

class IAvatarGenerator {
public:
    virtual ~IAvatarGenerator() = default;
    
    // 根据外观基因生成Avatar
    virtual AvatarBundle generate(const AppearanceGene& gene) = 0;
};
