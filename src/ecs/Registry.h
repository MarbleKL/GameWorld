#pragma once

#include "ComponentStorage.h"
#include "Entity.h"
#include "core/Result.h"
#include "core/Error.h"
#include <typeindex>
#include <unordered_map>
#include <memory>
#include <vector>
#include <string>

// ============================================================
// ECS Registry - 中央实体和组件管理器
// ============================================================

namespace ecs {

class Registry {
public:
    Registry() : next_entity_id_(1) {}

    // 创建实体
    EntityId create_entity(EntityType type);

    // 销毁实体（及其所有组件）
    void destroy_entity(EntityId id);

    // 检查实体是否存在
    bool entity_exists(EntityId id) const;

    // 获取实体类型 (返回 Result 以处理错误)
    core::Result<EntityType, core::ErrorCode> get_entity_type(EntityId id) const;

    // 添加组件
    template<typename Component>
    Component& add_component(EntityId id, Component&& comp) {
        if (!entity_exists(id)) {
            throw std::runtime_error("Entity does not exist: " + std::to_string(id));
        }

        auto type_idx = std::type_index(typeid(Component));

        // 获取或创建组件存储
        if (component_storages_.find(type_idx) == component_storages_.end()) {
            component_storages_[type_idx] = std::make_unique<ComponentStorage<Component>>();
        }

        auto* storage = static_cast<ComponentStorage<Component>*>(component_storages_[type_idx].get());
        return storage->add(id, std::move(comp));
    }

    // 获取组件
    template<typename Component>
    Component& get_component(EntityId id) {
        auto type_idx = std::type_index(typeid(Component));

        if (component_storages_.find(type_idx) == component_storages_.end()) {
            throw std::runtime_error("No storage for this component type");
        }

        auto* storage = static_cast<ComponentStorage<Component>*>(component_storages_[type_idx].get());
        return storage->get(id);
    }

    template<typename Component>
    const Component& get_component(EntityId id) const {
        auto type_idx = std::type_index(typeid(Component));

        auto it = component_storages_.find(type_idx);
        if (it == component_storages_.end()) {
            throw std::runtime_error("No storage for this component type");
        }

        auto* storage = static_cast<ComponentStorage<Component>*>(it->second.get());
        return storage->get(id);
    }

    // 检查是否有组件
    template<typename Component>
    bool has_component(EntityId id) const {
        auto type_idx = std::type_index(typeid(Component));

        auto it = component_storages_.find(type_idx);
        if (it == component_storages_.end()) {
            return false;
        }

        return it->second->has(id);
    }

    // 移除组件
    template<typename Component>
    void remove_component(EntityId id) {
        auto type_idx = std::type_index(typeid(Component));

        auto it = component_storages_.find(type_idx);
        if (it != component_storages_.end()) {
            it->second->remove(id);
        }
    }

    // 获取所有拥有指定组件的实体（用于单组件查询）
    template<typename Component>
    const std::vector<EntityId>& view() const {
        auto type_idx = std::type_index(typeid(Component));

        auto it = component_storages_.find(type_idx);
        if (it == component_storages_.end()) {
            static std::vector<EntityId> empty;
            return empty;
        }

        auto* storage = static_cast<ComponentStorage<Component>*>(it->second.get());
        return storage->get_entities();
    }

    // 获取所有拥有多个组件的实体（用于多组件联合查询）
    template<typename... Components>
    std::vector<EntityId> view_multi() const {
        std::vector<EntityId> result;

        // 获取第一个组件的所有实体
        auto first_entities = view<typename std::tuple_element<0, std::tuple<Components...>>::type>();

        // 过滤出同时拥有所有组件的实体
        for (EntityId id : first_entities) {
            if ((has_component<Components>(id) && ...)) {
                result.push_back(id);
            }
        }

        return result;
    }

    // 获取所有实体ID
    const std::unordered_map<EntityId, EntityType>& get_all_entities() const {
        return entity_types_;
    }

private:
    EntityId next_entity_id_;
    std::unordered_map<EntityId, EntityType> entity_types_;
    std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>> component_storages_;
};

} // namespace ecs
