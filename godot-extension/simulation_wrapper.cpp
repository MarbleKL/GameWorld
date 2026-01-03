#include "simulation_wrapper.h"
#include "coordinate_mapper.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "components/Components.h"
#include "simulation/Region.h"
#include "simulation/SpeciesTemplate.h"

// ============================================================
// 构造和析构
// ============================================================

SimulationWrapper::SimulationWrapper()
    : registry(nullptr)
    , recorder(nullptr)
    , state(nullptr)
    , context(nullptr)
    , scheduler(nullptr)
    , pop_system(nullptr)
    , creature_system(nullptr)
    , conversion_system(nullptr)
    , initialized(false)
{
}

SimulationWrapper::~SimulationWrapper() {
    // 清理所有 C++ 对象
    if (conversion_system) delete conversion_system;
    if (creature_system) delete creature_system;
    if (pop_system) delete pop_system;
    if (scheduler) delete scheduler;
    if (context) delete context;
    if (state) delete state;
    if (recorder) delete recorder;
    if (registry) delete registry;
}

// ============================================================
// 核心 API 实现
// ============================================================

void SimulationWrapper::initialize() {
    if (initialized) {
        UtilityFunctions::push_warning("SimulationWrapper already initialized!");
        return;
    }

    // 创建核心组件
    registry = new ecs::Registry();
    recorder = new ecs::EffectRecorder();
    state = new SimulationState();

    // 初始化世界
    state->initialize();

    // 创建 Process 上下文和调度器
    context = new ProcessContext(*registry, *recorder, *state);
    scheduler = new process::ProcessScheduler(*context);

    // 创建系统
    pop_system = new PopulationSystem(*scheduler);
    creature_system = new CreatureSystem(*scheduler);
    conversion_system = new ConversionSystem(*scheduler, *state);

    // 初始化世界种群
    _initialize_world_populations();

    initialized = true;

    UtilityFunctions::print("SimulationWrapper initialized! Time: ", state->current_time);
}

void SimulationWrapper::update(float delta) {
    if (!initialized) {
        UtilityFunctions::push_error("SimulationWrapper not initialized! Call initialize() first.");
        return;
    }

    // 清空Effect记录
    recorder->clear();

    // 1. HQ/LQ 转换检查（基于target_mode）
    conversion_system->update_region_modes();

    // 2. 更新LQ区域的种群
    pop_system->update(delta);

    // 3. 更新HQ区域的个体
    creature_system->update(delta);

    // 4. 时间推进
    state->current_time += delta;
}

void SimulationWrapper::set_camera_position(Vector3 pos) {
    if (!initialized) return;

    // 找到玩家当前所在的Region
    uint32_t current_region = CoordinateMapper::find_nearest_region(pos);

    // 设置所有Region的target_mode
    // 玩家所在Region → HQ，其他Region → LQ
    auto& all_regions = const_cast<std::map<uint32_t, Region>&>(state->get_all_regions());

    for (auto& [region_id, region] : all_regions) {
        if (region_id == current_region) {
            region.target_mode = Region::Mode::HQ;
        } else {
            region.target_mode = Region::Mode::LQ;
        }
    }
}

float SimulationWrapper::get_current_time() const {
    return initialized ? state->current_time : 0.0f;
}

int SimulationWrapper::get_current_region(Vector3 pos) {
    if (!initialized) return 0;
    return static_cast<int>(CoordinateMapper::find_nearest_region(pos));
}

// ============================================================
// Region 查询 API
// ============================================================

TypedArray<Dictionary> SimulationWrapper::get_all_regions() {
    TypedArray<Dictionary> result;

    if (!initialized) return result;

    const auto& all_regions = state->get_all_regions();

    for (const auto& [region_id, region] : all_regions) {
        result.append(_region_to_dict(region_id, region));
    }

    return result;
}

Dictionary SimulationWrapper::get_region(int region_id) {
    Dictionary result;

    if (!initialized) return result;

    try {
        const Region& region = state->get_region(static_cast<uint32_t>(region_id));
        result = _region_to_dict(region_id, region);
    } catch (...) {
        UtilityFunctions::push_error("Invalid region_id: ", region_id);
    }

    return result;
}

// ============================================================
// Creature 查询 API
// ============================================================

TypedArray<Dictionary> SimulationWrapper::get_creatures_in_region(int region_id) {
    TypedArray<Dictionary> result;

    if (!initialized) return result;

    // 遍历所有拥有 Position 组件的实体
    const auto& entities = registry->view<component::Position>();

    for (EntityId entity_id : entities) {
        const auto& pos = registry->get_component<component::Position>(entity_id);

        // 过滤：只返回指定Region的Creature
        if (pos.region_id != static_cast<uint32_t>(region_id)) {
            continue;
        }

        // 只处理Creature类型实体
        if (!registry->has_component<component::SpeciesRef>(entity_id)) {
            continue;
        }

        result.append(_creature_to_dict(entity_id));
    }

    return result;
}

// ============================================================
// Population 查询 API
// ============================================================

TypedArray<Dictionary> SimulationWrapper::get_populations_in_region(int region_id) {
    TypedArray<Dictionary> result;

    if (!initialized) return result;

    // 遍历所有 Population 实体
    const auto& populations = registry->view<component::Population>();

    for (EntityId entity_id : populations) {
        const auto& pop = registry->get_component<component::Population>(entity_id);

        if (pop.region_id != static_cast<uint32_t>(region_id)) {
            continue;
        }

        Dictionary pop_data;
        pop_data["entity_id"] = static_cast<int>(entity_id);
        pop_data["species_id"] = static_cast<int>(pop.species_id);
        pop_data["region_id"] = static_cast<int>(pop.region_id);
        pop_data["count"] = static_cast<int>(pop.estimated_count);
        pop_data["mode"] = (pop.mode == component::Population::Mode::Simulated) ? "Simulated" : "DerivedFromIndividuals";

        result.append(pop_data);
    }

    return result;
}

// ============================================================
// 全局统计 API
// ============================================================

Dictionary SimulationWrapper::get_global_statistics() {
    Dictionary stats;

    if (!initialized) return stats;

    int total_rabbits = 0;
    int total_wolves = 0;
    int total_bears = 0;

    // 遍历所有 Population 实体
    const auto& populations = registry->view<component::Population>();

    for (EntityId entity_id : populations) {
        const auto& pop = registry->get_component<component::Population>(entity_id);

        switch (pop.species_id) {
            case 1: total_rabbits += pop.estimated_count; break;
            case 2: total_wolves += pop.estimated_count; break;
            case 3: total_bears += pop.estimated_count; break;
        }
    }

    stats["rabbit_count"] = total_rabbits;
    stats["wolf_count"] = total_wolves;
    stats["bear_count"] = total_bears;
    stats["total_count"] = total_rabbits + total_wolves + total_bears;

    return stats;
}

// ============================================================
// 辅助函数
// ============================================================

void SimulationWrapper::_initialize_world_populations() {
    const auto& all_species = state->get_all_species_templates();
    const auto& all_regions = state->get_all_regions();

    // 为每个Region创建物种种群
    for (const auto& [region_id, region] : all_regions) {
        for (const auto& species : all_species) {
            // 跳过某些不适合的组合（熊不在沼泽和河流）
            if (species.id == 3 && (region_id == 4 || region_id == 5)) {
                continue;
            }

            // 创建Population实体
            EntityId pop_id = registry->create_entity(EntityType::Population);

            // 初始种群数量
            uint32_t initial_count = 0;
            if (species.id == 1) {  // Rabbit
                initial_count = 100 + (region_id * 20);
            } else if (species.id == 2) {  // Wolf
                initial_count = 10 + (region_id * 2);
            } else if (species.id == 3) {  // Bear
                initial_count = 5 + region_id;
            }

            // 添加Population组件
            registry->add_component(pop_id, component::Population{
                species.id,
                region_id,
                initial_count,
                species.base_birth_rate,
                species.base_death_rate,
                component::Population::Mode::Simulated,  // 初始为LQ模式
                species.limb_length_mean,
                species.body_mass_mean,
                species.size_scale_mean,
                species.limb_length_std,
                species.body_mass_std,
                species.size_scale_std
            });
        }
    }

    UtilityFunctions::print("Initialized ", all_species.size(), " species in ", all_regions.size(), " regions.");
}

Dictionary SimulationWrapper::_region_to_dict(uint32_t region_id, const Region& region) {
    Dictionary dict;

    dict["id"] = static_cast<int>(region_id);
    dict["name"] = String(region.name.c_str());
    dict["mode"] = (region.mode == Region::Mode::LQ) ? "LQ" : "HQ";
    dict["target_mode"] = (region.target_mode == Region::Mode::LQ) ? "LQ" : "HQ";
    dict["food_capacity"] = region.food_capacity;
    dict["current_food"] = region.current_food;
    dict["temperature"] = region.temperature;

    return dict;
}

Dictionary SimulationWrapper::_creature_to_dict(EntityId entity_id) {
    Dictionary dict;

    dict["entity_id"] = static_cast<int>(entity_id);

    // Position (必须有)
    const auto& pos = registry->get_component<component::Position>(entity_id);
    dict["region_id"] = static_cast<int>(pos.region_id);

    Vector3 world_pos = CoordinateMapper::to_godot_world(
        pos.region_id,
        pos.local_pos.x,
        pos.local_pos.z
    );
    dict["position"] = world_pos;

    // SpeciesRef (必须有)
    const auto& species_ref = registry->get_component<component::SpeciesRef>(entity_id);
    dict["species_id"] = static_cast<int>(species_ref.species_id);

    // Lifecycle (可选)
    if (registry->has_component<component::Lifecycle>(entity_id)) {
        const auto& lifecycle = registry->get_component<component::Lifecycle>(entity_id);
        dict["age"] = lifecycle.age;
        dict["lifespan"] = lifecycle.lifespan;
        dict["hunger"] = lifecycle.hunger;
        dict["health"] = lifecycle.health;
    }

    // GameplayGene (可选)
    if (registry->has_component<component::GameplayGene>(entity_id)) {
        const auto& gene = registry->get_component<component::GameplayGene>(entity_id);

        Dictionary gene_data;
        gene_data["limb_length"] = gene.limb_length;
        gene_data["body_mass"] = gene.body_mass;
        gene_data["size_scale"] = gene.size_scale;

        dict["gene"] = gene_data;
    }

    return dict;
}

// ============================================================
// Godot 绑定注册
// ============================================================

void SimulationWrapper::_bind_methods() {
    // 核心方法
    ClassDB::bind_method(D_METHOD("initialize"), &SimulationWrapper::initialize);
    ClassDB::bind_method(D_METHOD("update", "delta"), &SimulationWrapper::update);
    ClassDB::bind_method(D_METHOD("set_camera_position", "pos"), &SimulationWrapper::set_camera_position);
    ClassDB::bind_method(D_METHOD("get_current_time"), &SimulationWrapper::get_current_time);
    ClassDB::bind_method(D_METHOD("get_current_region", "pos"), &SimulationWrapper::get_current_region);

    // 查询方法
    ClassDB::bind_method(D_METHOD("get_all_regions"), &SimulationWrapper::get_all_regions);
    ClassDB::bind_method(D_METHOD("get_region", "region_id"), &SimulationWrapper::get_region);
    ClassDB::bind_method(D_METHOD("get_creatures_in_region", "region_id"), &SimulationWrapper::get_creatures_in_region);
    ClassDB::bind_method(D_METHOD("get_populations_in_region", "region_id"), &SimulationWrapper::get_populations_in_region);
    ClassDB::bind_method(D_METHOD("get_global_statistics"), &SimulationWrapper::get_global_statistics);
}
