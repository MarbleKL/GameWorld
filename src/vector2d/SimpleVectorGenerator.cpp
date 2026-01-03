#include "vector2d/SimpleVectorGenerator.h"
#include <cmath>
#include <algorithm>

namespace vector2d {

SimpleVectorGenerator::SimpleVectorGenerator()
    : canvas_width_(150), canvas_height_(200) {  // 更紧凑的画布
}

VectorCharacter SimpleVectorGenerator::generate(const AppearanceGene& gene) {
    return generateWithDirection(gene, 0);  // 默认向下方向
}

std::vector<VectorCharacter> SimpleVectorGenerator::generateDirections(
    const AppearanceGene& gene,
    int num_directions
) {
    std::vector<VectorCharacter> characters;

    for (int dir = 0; dir < num_directions; ++dir) {
        characters.push_back(generateWithDirection(gene, dir));
    }

    return characters;
}

VectorCharacter SimpleVectorGenerator::generateWithDirection(
    const AppearanceGene& gene,
    int direction
) {
    VectorCharacter character(canvas_width_, canvas_height_);

    // 按从后到前的顺序添加图层
    addShadow(character, gene, direction);
    addLegs(character, gene, direction);
    addBody(character, gene, direction);
    addArms(character, gene, direction);
    addHead(character, gene, direction);
    addFace(character, gene, direction);

    return character;
}

// ============================================================
// 绘制各部位
// ============================================================

void SimpleVectorGenerator::addShadow(
    VectorCharacter& character,
    const AppearanceGene& gene,
    int direction
) {
    CharacterLayer layer("shadow", 0);

    float center_x = canvas_width_ / 2;
    float bottom_y = canvas_height_ - 20;
    float body_width = 30 + gene.fatness * 30;  // 30-60

    auto shadow = std::make_shared<Ellipse>(
        Point(center_x, bottom_y),
        body_width * 0.6f,  // rx
        6.0f                 // ry
    );

    shadow->fill_color = Color(0, 0, 0, 50);  // 半透明黑色
    shadow->stroke_width = 0;

    layer.shapes.push_back(shadow);
    character.addLayer(layer);
}

void SimpleVectorGenerator::addLegs(
    VectorCharacter& character,
    const AppearanceGene& gene,
    int direction
) {
    CharacterLayer layer("legs", 1);

    float center_x = canvas_width_ / 2;
    float bottom_y = canvas_height_ - 15;
    float leg_width = 4.5;  // 略粗一点，更自然
    float leg_height = 70 + gene.height * 20;  // 更长的腿

    // 黑丝颜色（半透明黑色）
    Color stockings_color(30, 30, 30, 180);
    Color skin_color = getSkinColor(gene.base_tone);

    // 大腿袜的高度（从底部往上）
    float stocking_height = leg_height * 0.7f;  // 袜子覆盖大部分腿
    float thigh_gap_height = leg_height - stocking_height;  // 大腿裸露部分

    if (direction == 0 || direction == 2) {
        // 向下或向上：两腿并排（略微分开）
        float leg_spacing = 7;  // 腿间距

        // 左腿 - 大腿裸露部分（肤色）
        auto left_thigh = std::make_shared<Rectangle>(
            Point(center_x - leg_spacing, bottom_y - leg_height),
            leg_width, thigh_gap_height, 1
        );
        left_thigh->fill_color = skin_color;
        left_thigh->stroke_color = Color::Black();
        left_thigh->stroke_width = 2;

        // 左腿 - 过膝袜部分（黑色）
        auto left_stocking = std::make_shared<Rectangle>(
            Point(center_x - leg_spacing, bottom_y - stocking_height),
            leg_width, stocking_height, 1
        );
        left_stocking->fill_color = stockings_color;
        left_stocking->stroke_color = Color::Black();
        left_stocking->stroke_width = 2;

        // 右腿 - 大腿裸露部分
        auto right_thigh = std::make_shared<Rectangle>(
            Point(center_x + leg_spacing - leg_width, bottom_y - leg_height),
            leg_width, thigh_gap_height, 1
        );
        right_thigh->fill_color = skin_color;
        right_thigh->stroke_color = Color::Black();
        right_thigh->stroke_width = 2;

        // 右腿 - 过膝袜部分
        auto right_stocking = std::make_shared<Rectangle>(
            Point(center_x + leg_spacing - leg_width, bottom_y - stocking_height),
            leg_width, stocking_height, 1
        );
        right_stocking->fill_color = stockings_color;
        right_stocking->stroke_color = Color::Black();
        right_stocking->stroke_width = 2;

        layer.shapes.push_back(left_thigh);
        layer.shapes.push_back(left_stocking);
        layer.shapes.push_back(right_thigh);
        layer.shapes.push_back(right_stocking);
    }
    else {
        // 向左或向右：侧视图显示一条腿

        // 大腿裸露部分
        auto thigh = std::make_shared<Rectangle>(
            Point(center_x - leg_width/2, bottom_y - leg_height),
            leg_width, thigh_gap_height, 1
        );
        thigh->fill_color = skin_color;
        thigh->stroke_color = Color::Black();
        thigh->stroke_width = 2;

        // 过膝袜部分
        auto stocking = std::make_shared<Rectangle>(
            Point(center_x - leg_width/2, bottom_y - stocking_height),
            leg_width, stocking_height, 1
        );
        stocking->fill_color = stockings_color;
        stocking->stroke_color = Color::Black();
        stocking->stroke_width = 2;

        layer.shapes.push_back(thigh);
        layer.shapes.push_back(stocking);
    }

    character.addLayer(layer);
}

void SimpleVectorGenerator::addBody(
    VectorCharacter& character,
    const AppearanceGene& gene,
    int direction
) {
    float center_x = canvas_width_ / 2;
    float leg_height = 70 + gene.height * 20;
    float body_bottom = canvas_height_ - 15 - leg_height;

    // 简单的小方块身体
    float body_width = 16 + gene.fatness * 3;
    float body_height = 25 + gene.height * 5;  // 更小的身体

    Color body_color = Color(240, 240, 250, 255);  // 白色上衣

    // 身体层
    CharacterLayer body_layer("body", 2);

    // 简单的矩形身体
    auto body = std::make_shared<Rectangle>(
        Point(center_x - body_width/2, body_bottom - body_height),
        body_width,
        body_height,
        2  // 稍微圆角
    );
    body->fill_color = body_color;
    body->stroke_color = Color::Black();
    body->stroke_width = 2;

    body_layer.shapes.push_back(body);
    character.addLayer(body_layer);
}

void SimpleVectorGenerator::addArms(
    VectorCharacter& character,
    const AppearanceGene& gene,
    int direction
) {
    CharacterLayer layer("arms", 4);

    float center_x = canvas_width_ / 2;
    float leg_height = 70 + gene.height * 20;
    float body_bottom = canvas_height_ - 15 - leg_height;
    float body_width = 16 + gene.fatness * 3;
    float body_height = 25 + gene.height * 5;
    float shoulder_y = body_bottom - body_height;

    float arm_width = 3;
    float arm_length = 20;
    Color arm_color = getSkinColor(gene.base_tone);

    if (direction == 0 || direction == 2) {
        // 向下或向上：两臂在两侧
        auto left_arm = std::make_shared<Rectangle>(
            Point(center_x - body_width / 2 - arm_width, shoulder_y + 2),
            arm_width, arm_length, 2
        );
        left_arm->fill_color = arm_color;
        left_arm->stroke_color = Color::Black();
        left_arm->stroke_width = 2;

        auto right_arm = std::make_shared<Rectangle>(
            Point(center_x + body_width / 2, shoulder_y + 2),
            arm_width, arm_length, 2
        );
        right_arm->fill_color = arm_color;
        right_arm->stroke_color = Color::Black();
        right_arm->stroke_width = 2;

        layer.shapes.push_back(left_arm);
        layer.shapes.push_back(right_arm);
    }
    else {
        // 向左或向右：只显示一只手臂
        auto arm = std::make_shared<Rectangle>(
            Point(center_x - arm_width/2, shoulder_y + 2),
            arm_width, arm_length, 2
        );
        arm->fill_color = arm_color;
        arm->stroke_color = Color::Black();
        arm->stroke_width = 2;

        layer.shapes.push_back(arm);
    }

    character.addLayer(layer);
}

void SimpleVectorGenerator::addHead(
    VectorCharacter& character,
    const AppearanceGene& gene,
    int direction
) {
    CharacterLayer layer("head", 5);

    float center_x = canvas_width_ / 2;
    float leg_height = 70 + gene.height * 20;
    float body_bottom = canvas_height_ - 15 - leg_height;
    float body_height = 35 + gene.height * 8;
    float shoulder_y = body_bottom - body_height;

    // 简单的圆形头部
    float head_radius = 12;
    Color skin_color = getSkinColor(gene.base_tone);
    Color hair_color = getHairColor(gene.base_tone);

    // 头发（在后面）
    auto hair = std::make_shared<Circle>(
        Point(center_x, shoulder_y - head_radius),
        head_radius + 2
    );
    hair->fill_color = hair_color;
    hair->stroke_color = Color::Black();
    hair->stroke_width = 2;

    // 脸部
    auto head = std::make_shared<Circle>(
        Point(center_x, shoulder_y - head_radius),
        head_radius
    );
    head->fill_color = skin_color;
    head->stroke_color = Color::Black();
    head->stroke_width = 2;

    layer.shapes.push_back(hair);
    layer.shapes.push_back(head);

    character.addLayer(layer);
}

void SimpleVectorGenerator::addFace(
    VectorCharacter& character,
    const AppearanceGene& gene,
    int direction
) {
    CharacterLayer layer("face", 6);

    float center_x = canvas_width_ / 2;
    float leg_height = 70 + gene.height * 20;
    float body_bottom = canvas_height_ - 15 - leg_height;
    float body_height = 35 + gene.height * 8;
    float shoulder_y = body_bottom - body_height;
    float head_radius = 12;
    float head_y = shoulder_y - head_radius;

    if (direction == 0) {
        // 向下：两只简单的点眼睛
        auto left_eye = std::make_shared<Circle>(
            Point(center_x - 4, head_y - 2),
            1.5f
        );
        left_eye->fill_color = Color::Black();
        left_eye->stroke_width = 0;

        auto right_eye = std::make_shared<Circle>(
            Point(center_x + 4, head_y - 2),
            1.5f
        );
        right_eye->fill_color = Color::Black();
        right_eye->stroke_width = 0;

        // 小嘴巴
        auto mouth = std::make_shared<Circle>(
            Point(center_x, head_y + 4),
            1.0f
        );
        mouth->fill_color = Color(200, 100, 100, 255);
        mouth->stroke_width = 0;

        layer.shapes.push_back(left_eye);
        layer.shapes.push_back(right_eye);
        layer.shapes.push_back(mouth);
    }
    else if (direction == 1 || direction == 3) {
        // 向左或向右：侧脸一只眼
        float eye_x = (direction == 1) ? (center_x - 4) : (center_x + 4);

        auto eye = std::make_shared<Circle>(
            Point(eye_x, head_y - 2),
            1.5f
        );
        eye->fill_color = Color::Black();
        eye->stroke_width = 0;

        layer.shapes.push_back(eye);
    }
    // 向上：不显示面部

    character.addLayer(layer);
}

// ============================================================
// 颜色生成
// ============================================================

Color SimpleVectorGenerator::getSkinColor(float base_tone) const {
    base_tone = std::max(0.0f, std::min(1.0f, base_tone));

    uint8_t r = static_cast<uint8_t>(180 + base_tone * 60);  // 180-240
    uint8_t g = static_cast<uint8_t>(140 + base_tone * 50);  // 140-190
    uint8_t b = static_cast<uint8_t>(100 + base_tone * 40);  // 100-140

    return Color(r, g, b, 255);
}

Color SimpleVectorGenerator::getBodyColor(float base_tone, float pattern_variation) const {
    const Color palette[] = {
        Color(80, 60, 100, 255),   // 紫色
        Color(60, 80, 120, 255),   // 蓝色
        Color(80, 120, 70, 255),   // 绿色
        Color(120, 70, 70, 255),   // 红色
        Color(90, 70, 110, 255),   // 深紫色
        Color(110, 100, 60, 255),  // 黄褐色
    };

    int index = static_cast<int>(pattern_variation * 6.0f) % 6;
    Color base = palette[index];

    // 根据base_tone调整亮度
    float brightness = 0.7f + base_tone * 0.3f;
    brightness = std::max(0.5f, std::min(1.0f, brightness));

    return Color(
        static_cast<uint8_t>(base.r * brightness),
        static_cast<uint8_t>(base.g * brightness),
        static_cast<uint8_t>(base.b * brightness),
        255
    );
}

Color SimpleVectorGenerator::getHairColor(float base_tone) const {
    const Color hair_colors[] = {
        Color(40, 30, 20, 255),    // 黑色
        Color(80, 60, 40, 255),    // 深棕色
        Color(120, 100, 60, 255),  // 棕色
        Color(140, 120, 80, 255),  // 浅棕色
        Color(160, 140, 100, 255), // 金色
    };

    int index = static_cast<int>(base_tone * 5.0f) % 5;
    return hair_colors[index];
}

} // namespace vector2d
