#include "EffectRecorder.h"
#include <iostream>
#include <sstream>

namespace ecs {

void EffectRecorder::record(effect::Effect&& e) {
    effects_.push_back(std::move(e));
}

std::string EffectRecorder::effect_to_string(const effect::Effect& e) const {
    std::ostringstream oss;

    std::visit([&oss](const auto& eff) {
        using T = std::decay_t<decltype(eff)>;

        if constexpr (std::is_same_v<T, effect::EntityCreated>) {
            oss << "EntityCreated[id=" << eff.entity_id << ", type=";
            switch (eff.type) {
                case EntityType::Creature: oss << "Creature"; break;
                case EntityType::Population: oss << "Population"; break;
                case EntityType::Faction: oss << "Faction"; break;
                case EntityType::Location: oss << "Location"; break;
            }
            oss << ", desc=" << eff.description << "]";
        }
        else if constexpr (std::is_same_v<T, effect::EntityDestroyed>) {
            oss << "EntityDestroyed[id=" << eff.entity_id << ", reason=" << eff.reason << "]";
        }
        else if constexpr (std::is_same_v<T, effect::ResourceChanged>) {
            oss << "ResourceChanged[id=" << eff.entity_id << ", " << eff.resource_name
                << ": " << eff.old_value << " -> " << eff.new_value << "]";
        }
        else if constexpr (std::is_same_v<T, effect::Migration>) {
            oss << "Migration[id=" << eff.entity_id << ", region " << eff.from_region
                << " -> " << eff.to_region << ", count=" << eff.migrant_count << "]";
        }
        else if constexpr (std::is_same_v<T, effect::Death>) {
            oss << "Death[id=" << eff.entity_id << ", cause=" << eff.cause << "]";
        }
        else if constexpr (std::is_same_v<T, effect::Reproduction>) {
            oss << "Reproduction[parent=" << eff.parent_id << ", child=" << eff.child_id
                << ", species=" << eff.species_id << "]";
        }
        else if constexpr (std::is_same_v<T, effect::ComponentAdded>) {
            oss << "ComponentAdded[id=" << eff.entity_id << ", type=" << eff.component_type << "]";
        }
    }, e);

    return oss.str();
}

void EffectRecorder::log_to_console() const {
    if (effects_.empty()) {
        return;
    }

    std::cout << "=== Effects (" << effects_.size() << ") ===" << std::endl;
    for (const auto& eff : effects_) {
        std::cout << "  " << effect_to_string(eff) << std::endl;
    }
}

void EffectRecorder::log_to_file(const std::string& path) const {
    std::ofstream file(path, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open effect log file: " << path << std::endl;
        return;
    }

    for (const auto& eff : effects_) {
        file << effect_to_string(eff) << std::endl;
    }

    file.close();
}

} // namespace ecs
