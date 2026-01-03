#pragma once

#include "Region.h"
#include "SpeciesTemplate.h"
#include "core/Result.h"
#include "core/Error.h"
#include <map>
#include <vector>

// ============================================================
// SimulationState - 全局模拟状态管理
// 管理Region、物种模板、相机位置等
// ============================================================

class SimulationState {
public:
    SimulationState();

    // 初始化世界（创建5个Region的图结构）
    void initialize();

    // 时间管理
    float current_time;

    // Region访问 (返回 Result 以处理错误)
    core::Result<core::RefWrapper<Region>, core::ErrorCode> get_region(uint32_t id);
    core::Result<core::RefWrapper<const Region>, core::ErrorCode> get_region(uint32_t id) const;
    const std::map<uint32_t, Region>& get_all_regions() const { return regions_; }

    // 物种模板访问 (返回 Result 以处理错误)
    core::Result<core::RefWrapper<const SpeciesTemplate>, core::ErrorCode>
        get_species_template(SpeciesId id) const;
    const std::vector<SpeciesTemplate>& get_all_species_templates() const { return species_templates_; }

private:
    std::map<uint32_t, Region> regions_;
    std::vector<SpeciesTemplate> species_templates_;
};
