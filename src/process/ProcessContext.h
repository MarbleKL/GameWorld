#pragma once

#include "ecs/Registry.h"
#include "EffectRecorder.h"
#include "simulation/SimulationState.h"

// ============================================================
// ProcessContext - Process执行上下文
// 提供Registry、EffectRecorder、SimulationState的访问接口
// ============================================================

class ProcessContext {
public:
    ProcessContext(ecs::Registry& registry, ecs::EffectRecorder& recorder, SimulationState& state)
        : registry_(registry), recorder_(recorder), state_(state) {}

    // 组件访问
    template<typename C>
    C& get(EntityId id) {
        return registry_.get_component<C>(id);
    }

    template<typename C>
    const C& get(EntityId id) const {
        return registry_.get_component<C>(id);
    }

    template<typename C>
    bool has(EntityId id) const {
        return registry_.has_component<C>(id);
    }

    // Effect记录
    void record(effect::Effect&& e) {
        recorder_.record(std::move(e));
    }

    // 世界状态访问 (返回 Result 以处理错误)
    core::Result<core::RefWrapper<Region>, core::ErrorCode> get_region(uint32_t id) {
        return state_.get_region(id);
    }

    core::Result<core::RefWrapper<const Region>, core::ErrorCode> get_region(uint32_t id) const {
        return state_.get_region(id);
    }

    core::Result<core::RefWrapper<const SpeciesTemplate>, core::ErrorCode>
    get_species_template(SpeciesId id) const {
        return state_.get_species_template(id);
    }

    float get_time() const {
        return state_.current_time;
    }

    // Entity创建（立即创建，不延迟）
    EntityId create_entity(EntityType type) {
        EntityId id = registry_.create_entity(type);
        recorder_.record(effect::EntityCreated{id, type, ""});
        return id;
    }

    // Entity销毁
    void destroy_entity(EntityId id, const std::string& reason) {
        recorder_.record(effect::EntityDestroyed{id, reason});
        registry_.destroy_entity(id);
    }

    // Registry访问（用于批量查询）
    ecs::Registry& get_registry() { return registry_; }
    const ecs::Registry& get_registry() const { return registry_; }

    // SimulationState访问
    SimulationState& get_state() { return state_; }
    const SimulationState& get_state() const { return state_; }

private:
    ecs::Registry& registry_;
    ecs::EffectRecorder& recorder_;
    SimulationState& state_;
};
