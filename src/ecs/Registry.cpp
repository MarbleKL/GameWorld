#include "Registry.h"

namespace ecs {

EntityId Registry::create_entity(EntityType type) {
    EntityId id = next_entity_id_++;
    entity_types_[id] = type;
    return id;
}

void Registry::destroy_entity(EntityId id) {
    // 从所有组件存储中移除
    for (auto& [type_idx, storage] : component_storages_) {
        storage->remove(id);
    }

    // 从实体类型映射中移除
    entity_types_.erase(id);
}

bool Registry::entity_exists(EntityId id) const {
    return entity_types_.find(id) != entity_types_.end();
}

core::Result<EntityType, core::ErrorCode> Registry::get_entity_type(EntityId id) const {
    auto it = entity_types_.find(id);
    if (it == entity_types_.end()) {
        return core::Result<EntityType, core::ErrorCode>::Err(
            core::ErrorCode::ENTITY_NOT_FOUND
        );
    }
    return core::Result<EntityType, core::ErrorCode>::Ok(it->second);
}

} // namespace ecs
