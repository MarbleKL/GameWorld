#pragma once

#include "core/Types.h"
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <string>

// ============================================================
// 组件存储系统
// 使用sparse set实现高效的组件存储和迭代
// ============================================================

namespace ecs {

// 类型擦除的组件容器接口
class IComponentStorage {
public:
    virtual ~IComponentStorage() = default;
    virtual void remove(EntityId id) = 0;
    virtual bool has(EntityId id) const = 0;
};

// 具体类型的组件存储（使用sparse set）
template<typename Component>
class ComponentStorage : public IComponentStorage {
public:
    // 添加或更新组件
    Component& add(EntityId id, Component&& comp) {
        auto it = entity_to_index_.find(id);
        if (it != entity_to_index_.end()) {
            // 已存在，更新
            components_[it->second] = std::move(comp);
            return components_[it->second];
        } else {
            // 新增
            size_t index = components_.size();
            entity_to_index_[id] = index;
            entities_.push_back(id);
            components_.push_back(std::move(comp));
            return components_.back();
        }
    }

    // 获取组件（不存在则抛异常）
    Component& get(EntityId id) {
        auto it = entity_to_index_.find(id);
        if (it == entity_to_index_.end()) {
            throw std::runtime_error("Component not found for entity " + std::to_string(id));
        }
        return components_[it->second];
    }

    const Component& get(EntityId id) const {
        auto it = entity_to_index_.find(id);
        if (it == entity_to_index_.end()) {
            throw std::runtime_error("Component not found for entity " + std::to_string(id));
        }
        return components_[it->second];
    }

    // 检查是否存在
    bool has(EntityId id) const override {
        return entity_to_index_.find(id) != entity_to_index_.end();
    }

    // 移除组件
    void remove(EntityId id) override {
        auto it = entity_to_index_.find(id);
        if (it == entity_to_index_.end()) return;

        size_t index = it->second;
        size_t last_index = components_.size() - 1;

        // Swap with last element
        if (index != last_index) {
            components_[index] = std::move(components_[last_index]);
            entities_[index] = entities_[last_index];
            entity_to_index_[entities_[index]] = index;
        }

        // Remove last element
        components_.pop_back();
        entities_.pop_back();
        entity_to_index_.erase(id);
    }

    // 获取所有实体ID（用于迭代）
    const std::vector<EntityId>& get_entities() const {
        return entities_;
    }

    // 获取所有组件（用于迭代）
    std::vector<Component>& get_components() {
        return components_;
    }

    const std::vector<Component>& get_components() const {
        return components_;
    }

private:
    std::vector<Component> components_;        // 紧凑存储的组件数据
    std::vector<EntityId> entities_;           // 对应的实体ID
    std::unordered_map<EntityId, size_t> entity_to_index_;  // 实体ID到索引的映射
};

} // namespace ecs
