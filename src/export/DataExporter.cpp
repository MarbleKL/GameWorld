#include "DataExporter.h"
#include "components/Components.h"
#include <iostream>

DataExporter::DataExporter(const std::string& filepath)
    : is_open_(false) {
    file_.open(filepath);
    if (!file_.is_open()) {
        std::cerr << "Failed to open data export file: " << filepath << std::endl;
        return;
    }
    is_open_ = true;
    write_header();
}

DataExporter::~DataExporter() {
    finalize();
}

void DataExporter::write_header() {
    if (!is_open_) return;

    file_ << "time,region_id,region_name,species_id,species_name,mode,"
          << "population_count,avg_limb_length,avg_body_mass,avg_size_scale,"
          << "food_available,creature_count\n";
}

void DataExporter::write_timestep(float time, const ecs::Registry& registry, const SimulationState& state) {
    if (!is_open_) return;

    // 遍历所有Population实体
    const auto& all_pops = registry.view<component::Population>();

    for (EntityId pop_id : all_pops) {
        const auto& pop = registry.get_component<component::Population>(pop_id);
        const auto& region = state.get_region(pop.region_id);
        const auto& species = state.get_species_template(pop.species_id);

        // 统计该Region该物种的Creature数量
        uint32_t creature_count = 0;
        if (pop.mode == component::Population::Mode::DerivedFromIndividuals) {
            const auto& all_creatures = registry.view<component::SpeciesRef>();
            for (EntityId cid : all_creatures) {
                if (registry.has_component<component::Position>(cid)) {
                    const auto& species_ref = registry.get_component<component::SpeciesRef>(cid);
                    const auto& pos = registry.get_component<component::Position>(cid);
                    if (pos.region_id == pop.region_id && species_ref.species_id == pop.species_id) {
                        creature_count++;
                    }
                }
            }
        }

        // 写入CSV行
        file_ << time << ","
              << region.id << ","
              << region.name << ","
              << species.id << ","
              << species.name << ","
              << (pop.mode == component::Population::Mode::Simulated ? "LQ" : "HQ") << ","
              << pop.estimated_count << ","
              << pop.avg_limb_length << ","
              << pop.avg_body_mass << ","
              << pop.avg_size_scale << ","
              << region.current_food << ","
              << creature_count << "\n";
    }

    file_.flush();
}

void DataExporter::finalize() {
    if (is_open_) {
        file_.close();
        is_open_ = false;
    }
}
