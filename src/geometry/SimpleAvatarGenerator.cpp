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
    // 1. 基础椭球参数（基因驱动）
    // ====================================
    
    float base_size = 0.12f;
    
    // 脸型参数（椭球三轴）
    float face_width  = base_size * (0.9f + gene.fatness * 0.4f);      // X轴：胖→宽
    float face_height = base_size * (1.1f + gene.height * 0.1f);       // Y轴：高→长
    float face_depth  = base_size * (0.85f + gene.musculature * 0.15f); // Z轴：肌肉→深
    
    // 下颌宽度系数（肌肉男下巴更宽）
    float jaw_width_factor = 0.7f + gene.musculature * 0.5f;
    
    // 头部位置
    float neck_y = gene.height * 0.75f;
    float head_center_y = neck_y + face_height * 0.8f;
    
    // ====================================
    // 2. 生成椭球顶点（参数化球体变形）
    // ====================================
    
    const int h_segments = 12;  // 水平分段（增加精度）
    const int v_segments = 10;  // 垂直分段
    
    for (int v = 0; v <= v_segments; ++v) {
        float v_ratio = (float)v / v_segments;  // 0~1
        float v_angle = 3.14159f * v_ratio - 3.14159f / 2;  // -π/2 到 π/2
        
        float y_base = std::sin(v_angle);  // -1 到 1
        float xz_radius = std::cos(v_angle);  // 圆环半径
        
        // 下半部分收窄（形成下巴）
        float jaw_taper = 1.0f;
        if (v_ratio > 0.6f) {  // 下半部分
            float taper_amount = (v_ratio - 0.6f) / 0.4f;  // 0~1
            jaw_taper = 1.0f - taper_amount * (1.0f - jaw_width_factor);
        }
        
        for (int h = 0; h <= h_segments; ++h) {
            float h_angle = 2.0f * 3.14159f * h / h_segments;
            
            float x_base = xz_radius * std::cos(h_angle);
            float z_base = xz_radius * std::sin(h_angle);
            
            // ====================================
            // 3. 应用椭球变形 + 面部特征
            // ====================================
            
            // 基础椭球缩放
            float x = x_base * face_width * jaw_taper;
            float y = y_base * face_height;
            float z = z_base * face_depth;
            
            // 面部前部微调（Z+ 方向是脸的正面）
            if (z > 0) {
                // 眼窝区域（略微凹陷）
                if (v_ratio > 0.35f && v_ratio < 0.55f) {
                    float eye_socket = 1.0f - 0.08f * (1.0f - std::abs(x_base) * 2.0f);
                    z *= eye_socket;
                }
                
                // 鼻子区域（中央凸起）
                if (v_ratio > 0.4f && v_ratio < 0.65f && std::abs(x_base) < 0.3f) {
                    float nose_protrusion = 0.15f * (1.0f - std::abs(x_base) / 0.3f);
                    z += nose_protrusion * face_depth;
                }
                
                // 嘴巴区域（略微凹陷）
                if (v_ratio > 0.65f && v_ratio < 0.8f && std::abs(x_base) < 0.4f) {
                    float mouth_indent = 0.05f * (1.0f - std::abs(x_base) / 0.4f);
                    z -= mouth_indent * face_depth;
                }
            }
            
            // 颧骨（肌肉感增强侧面凸起）
            if (std::abs(x_base) > 0.5f && v_ratio > 0.35f && v_ratio < 0.6f) {
                float cheekbone = gene.musculature * 0.05f * face_width;
                x += (x > 0 ? cheekbone : -cheekbone);
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

} // namespace geometry
