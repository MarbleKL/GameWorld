#pragma once

#include "vector2d/VectorCharacter.h"
#include <string>
#include <fstream>
#include <vector>

// ============================================================
// SVG导出器 - 将矢量角色导出为SVG文件
// ============================================================

namespace vector2d {

class SVGExporter {
public:
    // 导出单个角色为SVG文件
    static bool exportToSVG(const VectorCharacter& character,
                           const std::string& filepath);

    // 导出多个方向的角色到单个SVG（精灵表，横向排列）
    static bool exportSpriteSheet(const std::vector<VectorCharacter>& characters,
                                 const std::string& filepath,
                                 float spacing = 20);
};

} // namespace vector2d
