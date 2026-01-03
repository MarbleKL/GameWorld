#include "ProcessScheduler.h"
#include <iostream>

namespace process {

void ProcessScheduler::execute_all_population_growth(float dt) {
    const auto& all_pops = ctx_.get_registry().view<component::Population>();

    for (EntityId pop_id : all_pops) {
        update_pop_growth_.execute(ctx_, pop_id, dt);
    }
}

void ProcessScheduler::execute_all_creature_lifecycle(float dt) {
    const auto& all_creatures = ctx_.get_registry().view<component::Lifecycle>();

    for (EntityId creature_id : all_creatures) {
        process_lifecycle_.execute(ctx_, creature_id, dt);
    }
}

void ProcessScheduler::convert_lq_to_hq(EntityId pop_id, uint32_t spawn_count) {
    auto& pop = ctx_.get<component::Population>(pop_id);

    if (pop.mode != component::Population::Mode::Simulated) {
        std::cout << "[Scheduler] Population " << pop_id << " is already in HQ mode" << std::endl;
        return;
    }

    std::cout << "[Scheduler] Converting LQ→HQ: Population " << pop_id
              << " (species " << pop.species_id << ", region " << pop.region_id << ")" << std::endl;

    // 1. 生成个体
    spawn_creatures_.execute(ctx_, pop_id, spawn_count);

    // 2. 切换模式
    pop.mode = component::Population::Mode::DerivedFromIndividuals;
}

void ProcessScheduler::convert_hq_to_lq(uint32_t region_id, SpeciesId species_id) {
    std::cout << "[Scheduler] Converting HQ→LQ: Region " << region_id
              << ", Species " << species_id << std::endl;

    // 1. 聚合统计
    aggregate_creatures_.execute(ctx_, region_id, species_id);

    // 2. 找到该区域该物种的所有Creature并销毁
    std::vector<EntityId> creatures_to_destroy;
    const auto& all_creatures = ctx_.get_registry().view<component::SpeciesRef>();

    for (EntityId cid : all_creatures) {
        if (!ctx_.has<component::Position>(cid)) continue;

        const auto& species_ref = ctx_.get<component::SpeciesRef>(cid);
        const auto& pos = ctx_.get<component::Position>(cid);

        if (pos.region_id == region_id && species_ref.species_id == species_id) {
            creatures_to_destroy.push_back(cid);
        }
    }

    for (EntityId cid : creatures_to_destroy) {
        ctx_.destroy_entity(cid, "hq_to_lq_conversion");
    }

    // 3. 切换Population模式
    const auto& all_pops = ctx_.get_registry().view<component::Population>();
    for (EntityId pop_id : all_pops) {
        auto& pop = ctx_.get<component::Population>(pop_id);
        if (pop.region_id == region_id && pop.species_id == species_id) {
            pop.mode = component::Population::Mode::Simulated;
            std::cout << "[Scheduler] Population " << pop_id << " switched to LQ mode" << std::endl;
        }
    }
}

} // namespace process
