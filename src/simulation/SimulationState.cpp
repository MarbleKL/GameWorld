#include "SimulationState.h"
#include <stdexcept>
#include <cmath>

SimulationState::SimulationState()
    : current_time(0.0f) {
}

void SimulationState::initialize() {
    // 创建5个Region的图结构
    // 拓扑结构：
    //   Forest(1) ─── Plains(2) ─── Hills(3)
    //      │            │            │
    //   Swamp(4) ─── River(5) ─── Mountain(6)

    regions_[1] = Region(1, "Forest", 1000.0f, 20.0f);
    regions_[2] = Region(2, "Plains", 1500.0f, 25.0f);
    regions_[3] = Region(3, "Hills", 800.0f, 18.0f);
    regions_[4] = Region(4, "Swamp", 600.0f, 22.0f);
    regions_[5] = Region(5, "River", 1200.0f, 23.0f);
    regions_[6] = Region(6, "Mountain", 500.0f, 10.0f);

    // 设置邻接关系
    regions_[1].neighbors = {2, 4};
    regions_[2].neighbors = {1, 3, 5};
    regions_[3].neighbors = {2, 6};
    regions_[4].neighbors = {1, 5};
    regions_[5].neighbors = {2, 4, 6};
    regions_[6].neighbors = {3, 5};

    // 初始化物种模板
    species_templates_.push_back(species_templates::rabbit());
    species_templates_.push_back(species_templates::wolf());
    species_templates_.push_back(species_templates::bear());
}

core::Result<core::RefWrapper<Region>, core::ErrorCode>
SimulationState::get_region(uint32_t id) {
    auto it = regions_.find(id);
    if (it == regions_.end()) {
        return core::Result<core::RefWrapper<Region>, core::ErrorCode>::Err(
            core::ErrorCode::REGION_NOT_FOUND
        );
    }
    return core::Result<core::RefWrapper<Region>, core::ErrorCode>::Ok(
        core::RefWrapper<Region>(it->second)
    );
}

core::Result<core::RefWrapper<const Region>, core::ErrorCode>
SimulationState::get_region(uint32_t id) const {
    auto it = regions_.find(id);
    if (it == regions_.end()) {
        return core::Result<core::RefWrapper<const Region>, core::ErrorCode>::Err(
            core::ErrorCode::REGION_NOT_FOUND
        );
    }
    return core::Result<core::RefWrapper<const Region>, core::ErrorCode>::Ok(
        core::RefWrapper<const Region>(it->second)
    );
}

core::Result<core::RefWrapper<const SpeciesTemplate>, core::ErrorCode>
SimulationState::get_species_template(SpeciesId id) const {
    for (const auto& st : species_templates_) {
        if (st.id == id) {
            return core::Result<core::RefWrapper<const SpeciesTemplate>, core::ErrorCode>::Ok(
                core::RefWrapper<const SpeciesTemplate>(st)
            );
        }
    }
    return core::Result<core::RefWrapper<const SpeciesTemplate>, core::ErrorCode>::Err(
        core::ErrorCode::SPECIES_NOT_FOUND
    );
}
