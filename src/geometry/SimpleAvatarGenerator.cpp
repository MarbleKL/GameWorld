#include "geometry/SimpleAvatarGenerator.h"
#include <cmath>

namespace geometry {

AvatarBundle SimpleAvatarGenerator::generate(const AppearanceGene& gene) {
    AvatarBundle bundle;
    
    // 暂时不生成骨架（简化版本）
    bundle.skeleton = Skeleton();
    
    // 根据外观基因生成简单的人形几何体
    // 身体 = 主立方体，头部 = 球体，四肢 = 小立方体
    
    // 生成身体部位
    MeshData body = GenerateBody(gene);
    MeshData head = GenerateHead(gene);
    MeshData limbs = GenerateLimbs(gene);
    
    bundle.meshes.push_back(body);
    bundle.meshes.push_back(head);
    bundle.meshes.push_back(limbs);
    
    return bundle;
}

MeshData SimpleAvatarGenerator::GenerateBody(const AppearanceGene& gene) {
    MeshData mesh;
    
    // 躯干比例：受fatness影响宽度，受height影响整体高度
    float torso_width = 0.35f * (1.0f + gene.fatness * 0.3f);   // 胖子更宽
    float torso_height = gene.height * 0.4f;                     // 躯干占总高40%
    float torso_depth = 0.2f * (1.0f + gene.fatness * 0.2f);    // 厚度也受影响
    
    float torso_bottom = gene.height * 0.35f;  // 躯干底部（留出腿部空间）
    float torso_top = torso_bottom + torso_height;
    
    // 躯干立方体（8个顶点）
    mesh.positions = {
        // 底部4个顶点
        {-torso_width, torso_bottom, -torso_depth},  // 0: 左下前
        { torso_width, torso_bottom, -torso_depth},  // 1: 右下前
        { torso_width, torso_bottom,  torso_depth},  // 2: 右下后
        {-torso_width, torso_bottom,  torso_depth},  // 3: 左下后
        // 顶部4个顶点（略窄，模拟肩部）
        {-torso_width * 1.1f, torso_top, -torso_depth * 0.9f},  // 4: 左上前
        { torso_width * 1.1f, torso_top, -torso_depth * 0.9f},  // 5: 右上前
        { torso_width * 1.1f, torso_top,  torso_depth * 0.9f},  // 6: 右上后
        {-torso_width * 1.1f, torso_top,  torso_depth * 0.9f}   // 7: 左上后
    };
    
    // 为每个面生成正确的法线
    mesh.normals = {
        {0, -1, 0}, {0, -1, 0}, {0, -1, 0}, {0, -1, 0},  // 底部顶点法线向下
        {0,  1, 0}, {0,  1, 0}, {0,  1, 0}, {0,  1, 0}   // 顶部顶点法线向上
    };
    
    // 躯干颜色（基于base_tone）
    Color body_color = {gene.base_tone, gene.base_tone * 0.8f, gene.base_tone * 0.6f, 1.0f};
    mesh.colors = std::vector<Color>(8, body_color);
    
    // 索引（6个面 x 2个三角形 = 12个三角形）
    mesh.indices = {
        // 前面 (Z-)
        0, 1, 5,  0, 5, 4,
        // 后面 (Z+)
        3, 7, 6,  3, 6, 2,
        // 左面 (X-)
        3, 0, 4,  3, 4, 7,
        // 右面 (X+)
        1, 2, 6,  1, 6, 5,
        // 顶面 (Y+)
        4, 5, 6,  4, 6, 7,
        // 底面 (Y-)
        3, 2, 1,  3, 1, 0
    };
    
    return mesh;
}

MeshData SimpleAvatarGenerator::GenerateHead(const AppearanceGene& gene) {
    MeshData mesh;
    
    // ====================================
    // 1. 基础椭球参数（基因驱动，美观约束）
    // ====================================

    float base_size = 0.12f;

    // 基因参数美观范围限制
    float fatness_clamped = std::max(0.7f, std::min(1.8f, gene.fatness));
    float height_clamped = std::max(0.9f, std::min(1.6f, gene.height));
    float muscle_clamped = std::max(0.5f, std::min(1.3f, gene.musculature));

    // 脸型宽度：0.80-1.00 倍基础尺寸（更窄的脸）
    float face_width  = base_size * (0.80f + (fatness_clamped - 0.7f) * 0.18f);

    // 脸型高度：1.20-1.45 倍基础尺寸（更长的脸）
    float face_height = base_size * (1.20f + (height_clamped - 0.9f) * 0.36f);

    // 脸型深度：0.60-0.70 倍基础尺寸（大幅减少，真实人脸很扁！）
    float face_depth  = base_size * (0.60f + (muscle_clamped - 0.5f) * 0.125f);

    // 下颌宽度系数：0.50（非常尖的下巴） - 0.70（稍宽下巴）
    float jaw_width_factor = 0.50f + (muscle_clamped - 0.5f) * 0.25f;
    
    // 头部位置
    float neck_y = gene.height * 0.75f;
    float head_center_y = neck_y + face_height * 0.8f;
    
    // ====================================
    // 2. 生成椭球顶点（参数化球体变形，美观曲线）
    // ====================================

    const int h_segments = 16;  // 水平分段（提高精度以获得更平滑的曲线）
    const int v_segments = 12;  // 垂直分段（增加以改善下巴/额头曲线）

    for (int v = 0; v <= v_segments; ++v) {
        float v_ratio = (float)v / v_segments;  // 0~1 (0=头顶, 1=下巴）
        float v_angle = 3.14159f * v_ratio - 3.14159f / 2;  // -π/2 到 π/2

        float y_base = std::sin(v_angle);  // -1 到 1
        float xz_radius = std::cos(v_angle);  // 圆环半径

        // ====================================
        // 垂直方向的不对称形状调整
        // ====================================

        float width_multiplier = 1.0f;

        if (v_ratio < 0.30f) {
            // 头顶部分（0-30%）：保持较宽，形成圆润的后脑勺
            width_multiplier = 0.95f + v_ratio * 0.17f;  // 0.95 到 1.00
        }
        else if (v_ratio < 0.45f) {
            // 额头到眼睛（30-45%）：略微变宽到最宽点
            float t = (v_ratio - 0.30f) / 0.15f;  // 0~1
            width_multiplier = 1.00f + t * 0.05f;  // 1.00 到 1.05（最宽点）
        }
        else if (v_ratio < 0.55f) {
            // 眼睛到鼻子（45-55%）：保持最宽（颧骨区域）
            width_multiplier = 1.05f;
        }
        else if (v_ratio < 0.70f) {
            // 鼻子到嘴巴（55-70%）：开始收窄
            float t = (v_ratio - 0.55f) / 0.15f;  // 0~1
            width_multiplier = 1.05f - t * 0.25f;  // 1.05 到 0.80
        }
        else {
            // 嘴巴到下巴（70-100%）：快速收窄成尖下巴
            float t = (v_ratio - 0.70f) / 0.30f;  // 0~1
            // 使用三次方曲线，越往下越窄
            float curve = t * t * t;
            width_multiplier = 0.80f - curve * (0.80f - jaw_width_factor);
        }

        // 应用宽度调整
        float effective_width = face_width * width_multiplier;
        
        for (int h = 0; h <= h_segments; ++h) {
            float h_angle = 2.0f * 3.14159f * h / h_segments;
            
            float x_base = xz_radius * std::cos(h_angle);
            float z_base = xz_radius * std::sin(h_angle);
            
            // ====================================
            // 3. 应用椭球变形 + 面部特征（微调以保持美观）
            // ====================================

            // 基础椭球缩放（使用不对称的宽度）
            float x = x_base * effective_width;
            float y = y_base * face_height;
            float z = z_base * face_depth;

            // ====================================
            // 面部前部微调（Z+ 方向是脸的正面，温和变化）
            // ====================================

            if (z > 0) {
                // 眼窝区域（轻微凹陷，更自然）
                if (v_ratio > 0.38f && v_ratio < 0.52f) {
                    float eye_socket_depth = 1.0f - 0.05f * (1.0f - std::abs(x_base) * 2.5f);
                    eye_socket_depth = std::max(0.96f, eye_socket_depth);  // 限制凹陷深度
                    z *= eye_socket_depth;
                }

                // 鼻子区域（非常明显的中央凸起）
                if (v_ratio > 0.38f && v_ratio < 0.68f && std::abs(x_base) < 0.18f) {
                    // 进一步增强突出强度到 0.50
                    float nose_width_factor = 1.0f - std::abs(x_base) / 0.18f;
                    nose_width_factor = nose_width_factor * nose_width_factor;  // 平方使鼻梁更窄

                    // 垂直方向的形状（鼻尖在中间）
                    float vertical_pos = (v_ratio - 0.38f) / 0.30f;  // 0~1
                    float vertical_factor;
                    if (vertical_pos < 0.5f) {
                        // 上半部分：从鼻根到鼻尖
                        vertical_factor = vertical_pos * 2.0f;  // 0~1
                    } else {
                        // 下半部分：从鼻尖到鼻底
                        vertical_factor = (1.0f - vertical_pos) * 2.0f;  // 1~0
                    }
                    vertical_factor = vertical_factor * vertical_factor;  // 平方使鼻尖更尖

                    float nose_protrusion = 0.55f * nose_width_factor * vertical_factor;
                    z += nose_protrusion * face_depth;
                }

                // 嘴巴区域（增强凹陷，让嘴巴更明显）
                if (v_ratio > 0.65f && v_ratio < 0.78f && std::abs(x_base) < 0.30f) {
                    float mouth_width_factor = 1.0f - std::abs(x_base) / 0.30f;
                    // 增强凹陷深度从 0.03 到 0.08
                    float mouth_indent = 0.08f * mouth_width_factor;
                    z -= mouth_indent * face_depth;
                }
            }

            // 颧骨（温和的侧面凸起，避免过度夸张）
            if (std::abs(x_base) > 0.55f && v_ratio > 0.38f && v_ratio < 0.58f) {
                float cheekbone_strength = muscle_clamped * 0.04f * face_width;
                x += (x > 0 ? cheekbone_strength : -cheekbone_strength);
            }
            
            // 最终位置
            Vec3 pos = {x, head_center_y + y, z};
            mesh.positions.push_back(pos);
            
            // ====================================
            // 4. 计算法线
            // ====================================
            
            // 简化：使用原始球体法线（对于平滑椭球足够）
            float len = std::sqrt(x_base*x_base + y_base*y_base + z_base*z_base);
            if (len > 0.001f) {
                mesh.normals.push_back({x_base/len, y_base/len, z_base/len});
            } else {
                mesh.normals.push_back({0, 1, 0});
            }
        }
    }
    
    // ====================================
    // 5. 颜色（肤色）
    // ====================================
    
    Color skin_color = {
        gene.base_tone * 1.05f,
        gene.base_tone * 0.95f,
        gene.base_tone * 0.85f,
        1.0f
    };
    mesh.colors = std::vector<Color>(mesh.positions.size(), skin_color);
    
    // ====================================
    // 6. 生成索引
    // ====================================
    
    for (int v = 0; v < v_segments; ++v) {
        for (int h = 0; h < h_segments; ++h) {
            int current = v * (h_segments + 1) + h;
            int next = current + h_segments + 1;
            
            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current + 1);
            
            mesh.indices.push_back(current + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }

    // ====================================
    // 7. 添加眼球
    // ====================================

    int vertex_offset = mesh.positions.size();

    // 眼球位置和大小
    float eye_radius = face_width * 0.25f;  // 增大眼球（从0.15到0.25）
    float eye_spacing = face_width * 0.45f;  // 增大两眼间距
    float eye_y = head_center_y + face_height * 0.18f;  // 眼睛Y位置稍高
    float eye_z = face_depth * 1.2f;  // 眼睛更突出

    // 左眼
    GenerateEye(mesh, {-eye_spacing, eye_y, eye_z}, eye_radius, vertex_offset);
    vertex_offset = mesh.positions.size();

    // 右眼
    GenerateEye(mesh, {eye_spacing, eye_y, eye_z}, eye_radius, vertex_offset);
    vertex_offset = mesh.positions.size();

    // 为眼球设置深色（瞳孔颜色）
    Color eye_color = {0.15f, 0.1f, 0.08f, 1.0f};  // 深褐色眼球
    int eye_vertex_count = (8 + 1) * (6 + 1) * 2;  // 两只眼睛的顶点数
    for (int i = 0; i < eye_vertex_count; ++i) {
        mesh.colors.push_back(eye_color);
    }

    // ====================================
    // 8. 添加耳朵
    // ====================================

    float ear_size = face_height * 0.20f;  // 耳朵大小
    float ear_y = head_center_y + face_height * 0.05f;  // 耳朵Y位置（与眼睛平齐偏下）
    float ear_x = face_width * 0.95f;  // 耳朵X位置（头部侧面）
    float ear_z = -face_depth * 0.2f;  // 耳朵Z位置（略靠后）

    // 左耳
    GenerateEar(mesh, {-ear_x, ear_y, ear_z}, ear_size, true, vertex_offset);
    vertex_offset = mesh.positions.size();

    // 右耳
    GenerateEar(mesh, {ear_x, ear_y, ear_z}, ear_size, false, vertex_offset);

    // 为耳朵设置肤色
    int ear_vertex_count = (6 + 1) * 2 * 2;  // 两只耳朵的顶点数
    for (int i = 0; i < ear_vertex_count; ++i) {
        mesh.colors.push_back(skin_color);
    }

    return mesh;
}

MeshData SimpleAvatarGenerator::GenerateLimbs(const AppearanceGene& gene) {
    MeshData mesh;
    
    // 四肢粗细：受musculature影响
    float arm_thickness = 0.06f * (1.0f + gene.musculature * 0.4f);
    float leg_thickness = 0.08f * (1.0f + gene.musculature * 0.3f) * (1.0f + gene.fatness * 0.2f);
    
    // 四肢长度
    float arm_length = gene.height * 0.35f;
    float leg_length = gene.height * 0.35f;
    
    // 肩膀和臀部位置
    float shoulder_y = gene.height * 0.7f;
    float shoulder_x = 0.35f * (1.0f + gene.fatness * 0.3f);
    float hip_y = gene.height * 0.35f;
    float hip_x = 0.15f * (1.0f + gene.fatness * 0.2f);
    
    int vertex_offset = 0;
    
    // === 左臂 ===
    GenerateLimb(mesh, 
                 {-shoulder_x, shoulder_y, 0},  // 起点（肩膀）
                 {-shoulder_x, shoulder_y - arm_length, 0},  // 终点（手）
                 arm_thickness, vertex_offset);
    vertex_offset = mesh.positions.size();
    
    // === 右臂 ===
    GenerateLimb(mesh, 
                 {shoulder_x, shoulder_y, 0}, 
                 {shoulder_x, shoulder_y - arm_length, 0}, 
                 arm_thickness, vertex_offset);
    vertex_offset = mesh.positions.size();
    
    // === 左腿 ===
    GenerateLimb(mesh, 
                 {-hip_x, hip_y, 0},  // 起点（臀部）
                 {-hip_x, 0, 0},      // 终点（脚）
                 leg_thickness, vertex_offset);
    vertex_offset = mesh.positions.size();
    
    // === 右腿 ===
    GenerateLimb(mesh, 
                 {hip_x, hip_y, 0}, 
                 {hip_x, 0, 0}, 
                 leg_thickness, vertex_offset);
    
    // 四肢颜色（与身体相同）
    Color limb_color = {gene.base_tone * 0.95f, gene.base_tone * 0.75f, gene.base_tone * 0.55f, 1.0f};
    mesh.colors = std::vector<Color>(mesh.positions.size(), limb_color);
    
    return mesh;
}

// 辅助函数：生成单个肢体（圆柱体）
void SimpleAvatarGenerator::GenerateLimb(MeshData& mesh, 
                                          Vec3 start, Vec3 end, 
                                          float thickness, 
                                          int vertex_offset) {
    const int segments = 6;  // 圆柱体分段数
    
    Vec3 direction = {end.x - start.x, end.y - start.y, end.z - start.z};
    float length = std::sqrt(direction.x * direction.x + 
                            direction.y * direction.y + 
                            direction.z * direction.z);
    
    // 生成圆柱体顶点（底部圆 + 顶部圆）
    for (int ring = 0; ring < 2; ++ring) {
        Vec3 center = (ring == 0) ? start : end;
        
        for (int i = 0; i < segments; ++i) {
            float angle = 2.0f * 3.14159f * i / segments;
            float x = thickness * std::cos(angle);
            float z = thickness * std::sin(angle);
            
            mesh.positions.push_back({center.x + x, center.y, center.z + z});
            
            // 法线（径向）
            float len = std::sqrt(x*x + z*z);
            mesh.normals.push_back({x/len, 0, z/len});
        }
    }
    
    // 生成圆柱体侧面索引
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        
        int bottom_current = vertex_offset + i;
        int bottom_next = vertex_offset + next;
        int top_current = vertex_offset + segments + i;
        int top_next = vertex_offset + segments + next;
        
        // 第一个三角形
        mesh.indices.push_back(bottom_current);
        mesh.indices.push_back(top_current);
        mesh.indices.push_back(bottom_next);
        
        // 第二个三角形
        mesh.indices.push_back(bottom_next);
        mesh.indices.push_back(top_current);
        mesh.indices.push_back(top_next);
    }
}

// 辅助函数：生成眼球
void SimpleAvatarGenerator::GenerateEye(MeshData& mesh, Vec3 center, float radius, int vertex_offset) {
    const int segments = 8;  // 低面数球体：8个分段
    const int rings = 6;

    // 生成球体顶点
    for (int ring = 0; ring <= rings; ++ring) {
        float v = (float)ring / rings;  // 0~1
        float phi = 3.14159f * v;  // 0 到 π

        for (int seg = 0; seg <= segments; ++seg) {
            float u = (float)seg / segments;  // 0~1
            float theta = 2.0f * 3.14159f * u;  // 0 到 2π

            float x = radius * std::sin(phi) * std::cos(theta);
            float y = radius * std::cos(phi);
            float z = radius * std::sin(phi) * std::sin(theta);

            mesh.positions.push_back({center.x + x, center.y + y, center.z + z});

            // 球体法线
            float len = std::sqrt(x*x + y*y + z*z);
            if (len > 0.001f) {
                mesh.normals.push_back({x/len, y/len, z/len});
            } else {
                mesh.normals.push_back({0, 1, 0});
            }
        }
    }

    // 生成索引
    for (int ring = 0; ring < rings; ++ring) {
        for (int seg = 0; seg < segments; ++seg) {
            int current = vertex_offset + ring * (segments + 1) + seg;
            int next = current + segments + 1;

            mesh.indices.push_back(current);
            mesh.indices.push_back(next);
            mesh.indices.push_back(current + 1);

            mesh.indices.push_back(current + 1);
            mesh.indices.push_back(next);
            mesh.indices.push_back(next + 1);
        }
    }
}

// 辅助函数：生成耳朵（简化C形）
void SimpleAvatarGenerator::GenerateEar(MeshData& mesh, Vec3 position, float size, bool is_left, int vertex_offset) {
    const int segments = 6;  // 简化的耳朵
    float sign = is_left ? -1.0f : 1.0f;  // 左右镜像

    // 生成C形曲线的顶点（半圆环）
    for (int i = 0; i <= segments; ++i) {
        float angle = 3.14159f * (float)i / segments;  // 0 到 π（半圆）

        // 外环
        float outer_radius = size;
        float x_outer = sign * outer_radius * std::cos(angle);
        float y_outer = outer_radius * std::sin(angle);
        mesh.positions.push_back({position.x + x_outer, position.y + y_outer, position.z});

        // 内环（较小）
        float inner_radius = size * 0.6f;
        float x_inner = sign * inner_radius * std::cos(angle);
        float y_inner = inner_radius * std::sin(angle);
        mesh.positions.push_back({position.x + x_inner, position.y + y_inner, position.z});

        // 简化法线（指向侧面）
        mesh.normals.push_back({sign, 0, 0});
        mesh.normals.push_back({sign, 0, 0});
    }

    // 生成索引（连接外环和内环）
    for (int i = 0; i < segments; ++i) {
        int base = vertex_offset + i * 2;

        // 外侧三角形
        mesh.indices.push_back(base);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 1);

        // 内侧三角形
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
    }
}

} // namespace geometry
