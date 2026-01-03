#include "vector2d/SVGExporter.h"
#include <iostream>
#include <sstream>

namespace vector2d {

bool SVGExporter::exportToSVG(
    const VectorCharacter& character,
    const std::string& filepath
) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    file << character.toSVG();
    file.close();

    std::cout << "Exported SVG to: " << filepath << std::endl;
    return true;
}

bool SVGExporter::exportSpriteSheet(
    const std::vector<VectorCharacter>& characters,
    const std::string& filepath,
    float spacing
) {
    if (characters.empty()) {
        std::cerr << "Cannot export empty character list" << std::endl;
        return false;
    }

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    // 计算总画布尺寸
    float char_width = characters[0].width;
    float char_height = characters[0].height;
    float total_width = char_width * characters.size() + spacing * (characters.size() - 1);
    float total_height = char_height;

    // SVG头部
    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    file << "<svg width=\"" << total_width << "\" height=\"" << total_height << "\""
         << " xmlns=\"http://www.w3.org/2000/svg\""
         << " viewBox=\"0 0 " << total_width << " " << total_height << "\">\n";

    file << "  <desc>Sprite sheet with " << characters.size() << " directions</desc>\n\n";

    // 绘制每个角色
    for (size_t i = 0; i < characters.size(); ++i) {
        float offset_x = i * (char_width + spacing);

        file << "  <g id=\"direction_" << i << "\""
             << " transform=\"translate(" << offset_x << ", 0)\">\n";

        // 渲染角色的所有图层
        for (const auto& layer : characters[i].layers) {
            file << layer.toSVG(4) << "\n";
        }

        file << "  </g>\n\n";
    }

    // SVG尾部
    file << "</svg>";

    file.close();

    std::cout << "Exported sprite sheet to: " << filepath << std::endl;
    return true;
}

} // namespace vector2d
