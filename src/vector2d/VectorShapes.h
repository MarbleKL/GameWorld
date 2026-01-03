#pragma once

#include "vector2d/VectorTypes.h"
#include <memory>
#include <vector>
#include <sstream>

// ============================================================
// 矢量图形状定义
// ============================================================

namespace vector2d {

// 形状类型枚举
enum class ShapeType {
    Circle,
    Ellipse,
    Rectangle,
    Polygon,
    Path
};

// 几何图形基类
class Shape {
public:
    ShapeType type;
    Color fill_color;
    Color stroke_color;
    float stroke_width;

    Shape(ShapeType t)
        : type(t)
        , fill_color(Color::White())
        , stroke_color(Color::Black())
        , stroke_width(1.0f) {}

    virtual ~Shape() = default;

    // 转换为SVG元素字符串
    virtual std::string toSVG(int indent = 2) const = 0;

protected:
    // 生成通用的SVG样式属性
    std::string styleAttributes() const {
        std::stringstream ss;

        // 填充颜色
        if (fill_color.a == 0) {
            ss << " fill=\"none\"";
        } else if (fill_color.a == 255) {
            ss << " fill=\"" << fill_color.toHex() << "\"";
        } else {
            ss << " fill=\"" << fill_color.toRGBA() << "\"";
        }

        // 描边
        if (stroke_width > 0 && stroke_color.a > 0) {
            if (stroke_color.a == 255) {
                ss << " stroke=\"" << stroke_color.toHex() << "\"";
            } else {
                ss << " stroke=\"" << stroke_color.toRGBA() << "\"";
            }
            ss << " stroke-width=\"" << stroke_width << "\"";
        } else {
            ss << " stroke=\"none\"";
        }

        return ss.str();
    }

    std::string indentStr(int indent) const {
        return std::string(indent, ' ');
    }
};

// 圆形
class Circle : public Shape {
public:
    Point center;
    float radius;

    Circle() : Shape(ShapeType::Circle), radius(10) {}

    Circle(Point c, float r)
        : Shape(ShapeType::Circle), center(c), radius(r) {}

    std::string toSVG(int indent = 2) const override {
        std::stringstream ss;
        ss << indentStr(indent) << "<circle"
           << " cx=\"" << center.x << "\""
           << " cy=\"" << center.y << "\""
           << " r=\"" << radius << "\""
           << styleAttributes()
           << " />";
        return ss.str();
    }
};

// 椭圆
class Ellipse : public Shape {
public:
    Point center;
    float rx, ry;  // 横向和纵向半径

    Ellipse() : Shape(ShapeType::Ellipse), rx(10), ry(10) {}

    Ellipse(Point c, float rx_, float ry_)
        : Shape(ShapeType::Ellipse), center(c), rx(rx_), ry(ry_) {}

    std::string toSVG(int indent = 2) const override {
        std::stringstream ss;
        ss << indentStr(indent) << "<ellipse"
           << " cx=\"" << center.x << "\""
           << " cy=\"" << center.y << "\""
           << " rx=\"" << rx << "\""
           << " ry=\"" << ry << "\""
           << styleAttributes()
           << " />";
        return ss.str();
    }
};

// 矩形
class Rectangle : public Shape {
public:
    Point top_left;
    float width, height;
    float corner_radius;  // 圆角半径（可选）

    Rectangle()
        : Shape(ShapeType::Rectangle)
        , width(10), height(10), corner_radius(0) {}

    Rectangle(Point tl, float w, float h, float cr = 0)
        : Shape(ShapeType::Rectangle)
        , top_left(tl), width(w), height(h), corner_radius(cr) {}

    std::string toSVG(int indent = 2) const override {
        std::stringstream ss;
        ss << indentStr(indent) << "<rect"
           << " x=\"" << top_left.x << "\""
           << " y=\"" << top_left.y << "\""
           << " width=\"" << width << "\""
           << " height=\"" << height << "\"";

        if (corner_radius > 0) {
            ss << " rx=\"" << corner_radius << "\"";
        }

        ss << styleAttributes() << " />";
        return ss.str();
    }
};

// 多边形
class Polygon : public Shape {
public:
    std::vector<Point> points;

    Polygon() : Shape(ShapeType::Polygon) {}

    Polygon(const std::vector<Point>& pts)
        : Shape(ShapeType::Polygon), points(pts) {}

    std::string toSVG(int indent = 2) const override {
        if (points.empty()) return "";

        std::stringstream ss;
        ss << indentStr(indent) << "<polygon points=\"";

        for (size_t i = 0; i < points.size(); ++i) {
            if (i > 0) ss << " ";
            ss << points[i].x << "," << points[i].y;
        }

        ss << "\"" << styleAttributes() << " />";
        return ss.str();
    }
};

// 路径（用于绘制曲线、复杂形状）
class Path : public Shape {
public:
    std::string path_data;  // SVG path 数据（如 "M 10 10 L 20 20"）

    Path() : Shape(ShapeType::Path) {}

    Path(const std::string& data)
        : Shape(ShapeType::Path), path_data(data) {}

    std::string toSVG(int indent = 2) const override {
        std::stringstream ss;
        ss << indentStr(indent) << "<path"
           << " d=\"" << path_data << "\""
           << styleAttributes()
           << " />";
        return ss.str();
    }
};

} // namespace vector2d
