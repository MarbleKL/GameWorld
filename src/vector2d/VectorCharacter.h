#pragma once

#include "vector2d/VectorShapes.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <sstream>

// ============================================================
// 矢量图角色表示
// ============================================================

namespace vector2d {

// 角色的单个图层
struct CharacterLayer {
    std::string name;                              // 图层名称
    std::vector<std::shared_ptr<Shape>> shapes;    // 该图层的所有形状
    int z_index;                                   // 层级（用于排序，越大越靠前）

    CharacterLayer() : z_index(0) {}

    CharacterLayer(const std::string& n, int z)
        : name(n), z_index(z) {}

    // 转换为SVG <g>元素
    std::string toSVG(int indent = 2) const {
        std::stringstream ss;
        std::string ind = std::string(indent, ' ');

        ss << ind << "<g id=\"" << name << "\">\n";

        for (const auto& shape : shapes) {
            ss << shape->toSVG(indent + 2) << "\n";
        }

        ss << ind << "</g>";

        return ss.str();
    }
};

// 角色的完整矢量图表示
class VectorCharacter {
public:
    std::vector<CharacterLayer> layers;
    float width, height;  // 画布尺寸

    VectorCharacter() : width(200), height(300) {}

    VectorCharacter(float w, float h) : width(w), height(h) {}

    // 添加图层
    void addLayer(const CharacterLayer& layer) {
        layers.push_back(layer);
        sortLayers();
    }

    // 按z_index排序图层
    void sortLayers() {
        std::sort(layers.begin(), layers.end(),
                 [](const CharacterLayer& a, const CharacterLayer& b) {
                     return a.z_index < b.z_index;
                 });
    }

    // 导出为完整的SVG文档
    std::string toSVG() const {
        std::stringstream ss;

        // SVG头部
        ss << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        ss << "<svg width=\"" << width << "\" height=\"" << height << "\""
           << " xmlns=\"http://www.w3.org/2000/svg\""
           << " viewBox=\"0 0 " << width << " " << height << "\">\n";

        // 添加描述
        ss << "  <desc>Procedurally generated vector character</desc>\n\n";

        // 渲染所有图层
        for (const auto& layer : layers) {
            ss << layer.toSVG(2) << "\n";
        }

        // SVG尾部
        ss << "</svg>";

        return ss.str();
    }
};

} // namespace vector2d
