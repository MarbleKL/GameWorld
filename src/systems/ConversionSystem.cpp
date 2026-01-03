#include "ConversionSystem.h"
#include <iostream>
#include <set>

void ConversionSystem::update_region_modes() {
    auto& registry = scheduler_.ctx_.get_registry();

    // 遍历所有Region
    for (auto [region_id, _] : state_.get_all_regions()) {
        auto region_result = state_.get_region(region_id);
        if (region_result.is_err()) {
            std::cerr << "Error: Region " << region_id << " not found during conversion" << std::endl;
            continue;
        }
        auto& region = region_result.value().get();

        // 如果当前模式与目标模式不同，执行转换
        if (region.mode != region.target_mode) {
            if (region.target_mode == Region::Mode::HQ) {
                // LQ → HQ：为该Region的所有种群生成个体
                std::cout << "\n=== Converting Region " << region_id << " (" << region.name
                          << ") to HQ mode ===" << std::endl;

                const auto& all_pops = registry.view<component::Population>();
                for (EntityId pop_id : all_pops) {
                    const auto& pop = registry.get_component<component::Population>(pop_id);

                    if (pop.region_id == region_id &&
                        pop.mode == component::Population::Mode::Simulated) {
                        // 转换这个种群
                        uint32_t spawn_count = std::min(pop.estimated_count, 150u);
                        scheduler_.convert_lq_to_hq(pop_id, spawn_count);
                    }
                }

                region.mode = Region::Mode::HQ;
            }
            else {
                // HQ → LQ：聚合该Region的所有物种并销毁个体
                std::cout << "\n=== Converting Region " << region_id << " (" << region.name
                          << ") to LQ mode ===" << std::endl;

                // 找出该Region有哪些物种
                std::set<SpeciesId> species_in_region;
                const auto& all_creatures = registry.view<component::SpeciesRef>();

                for (EntityId cid : all_creatures) {
                    if (registry.has_component<component::Position>(cid)) {
                        const auto& pos = registry.get_component<component::Position>(cid);
                        if (pos.region_id == region_id) {
                            const auto& species_ref = registry.get_component<component::SpeciesRef>(cid);
                            species_in_region.insert(species_ref.species_id);
                        }
                    }
                }

                // 对每个物种执行HQ→LQ转换
                for (SpeciesId sid : species_in_region) {
                    scheduler_.convert_hq_to_lq(region_id, sid);
                }

                region.mode = Region::Mode::LQ;
            }
        }
    }
}
