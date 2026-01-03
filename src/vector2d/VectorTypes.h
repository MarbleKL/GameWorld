#pragma once

#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

// ============================================================
// 矢量图基础数据结构
// ============================================================

namespace vector2d {

// 颜色（RGBA，0-255）
struct Color {
    uint8_t r, g, b, a;

    Color() : r(0), g(0), b(0), a(255) {}

    Color(uint8_t r_, uint8_t g_, uint8_t b_, uint8_t a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {}

    // 转换为 #RRGGBB 格式
    std::string toHex() const {
        std::stringstream ss;
        ss << "#"
           << std::hex << std::setfill('0')
           << std::setw(2) << (int)r
           << std::setw(2) << (int)g
           << std::setw(2) << (int)b;
        return ss.str();
    }

    // 转换为 rgba(r,g,b,a) 格式
    std::string toRGBA() const {
        std::stringstream ss;
        ss << "rgba(" << (int)r << "," << (int)g << ","
           << (int)b << "," << (a / 255.0f) << ")";
        return ss.str();
    }

    // 预定义颜色
    static Color Black() { return Color(0, 0, 0, 255); }
    static Color White() { return Color(255, 255, 255, 255); }
    static Color Transparent() { return Color(0, 0, 0, 0); }
};

// 2D点
struct Point {
    float x, y;

    Point() : x(0), y(0) {}
    Point(float x_, float y_) : x(x_), y(y_) {}
};

} // namespace vector2d
