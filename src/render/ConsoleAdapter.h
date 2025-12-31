#pragma once

#include "render/IRenderAdapter.h"
#include <iostream>
#include <iomanip>
#include <atomic>

// ============================================================
// 控制台渲染适配器
// 将几何数据输出到控制台（用于Demo验证）
// ============================================================

namespace render {

class ConsoleAdapter : public IRenderAdapter {
public:
    AvatarId CreateAvatar(const AvatarBundle& bundle) override {
        AvatarId id = next_id_++;
        
        std::cout << "\n========== Avatar #" << id << " Created ==========\n";
        
        // 输出骨架信息
        std::cout << "Skeleton: " << bundle.skeleton.bones.size() << " bones\n";
        
        // 输出Mesh信息
        std::cout << "Meshes: " << bundle.meshes.size() << " parts\n";
        for (size_t i = 0; i < bundle.meshes.size(); ++i) {
            const auto& mesh = bundle.meshes[i];
            std::cout << "  [Mesh " << i << "]\n";
            std::cout << "    Vertices: " << mesh.positions.size() << "\n";
            std::cout << "    Indices:  " << mesh.indices.size() << "\n";
            std::cout << "    Normals:  " << mesh.normals.size() << "\n";
            std::cout << "    Colors:   " << mesh.colors.size() << "\n";
            
            // 输出几个顶点示例
            if (!mesh.positions.empty()) {
                std::cout << "    Sample vertices:\n";
                size_t sample_count = std::min(size_t(3), mesh.positions.size());
                for (size_t v = 0; v < sample_count; ++v) {
                    const auto& pos = mesh.positions[v];
                    std::cout << "      [" << v << "] pos(" 
                              << std::fixed << std::setprecision(2)
                              << pos.x << ", " << pos.y << ", " << pos.z << ")";
                    
                    if (v < mesh.colors.size()) {
                        const auto& col = mesh.colors[v];
                        std::cout << " color(" 
                                  << col.r << ", " << col.g << ", " << col.b << ")";
                    }
                    std::cout << "\n";
                }
            }
        }
        
        std::cout << "=======================================\n";
        
        return id;
    }
    
    void DestroyAvatar(AvatarId avatar_id) override {
        std::cout << "[ConsoleAdapter] Destroyed Avatar #" << avatar_id << "\n";
    }

private:
    static std::atomic<AvatarId> next_id_;
};

// 静态成员初始化
std::atomic<AvatarId> ConsoleAdapter::next_id_{1};

} // namespace render
