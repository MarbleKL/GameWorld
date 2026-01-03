#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector3.hpp>

#include "ecs/Registry.h"
#include "process/EffectRecorder.h"
#include "simulation/SimulationState.h"
#include "process/ProcessContext.h"
#include "process/ProcessScheduler.h"
#include "systems/PopulationSystem.h"
#include "systems/CreatureSystem.h"
#include "systems/ConversionSystem.h"

using namespace godot;

// ============================================================
// SimulationWrapper - GDExtension 绑定类
// 封装 C++ ECS 模拟核心，提供 GDScript API
// ============================================================

class SimulationWrapper : public RefCounted {
    GDCLASS(SimulationWrapper, RefCounted)

private:
    // C++ 模拟核心组件 (拥有所有权)
    ecs::Registry* registry;
    ecs::EffectRecorder* recorder;
    SimulationState* state;
    ProcessContext* context;
    process::ProcessScheduler* scheduler;
    PopulationSystem* pop_system;
    CreatureSystem* creature_system;
    ConversionSystem* conversion_system;

    bool initialized;

public:
    SimulationWrapper();
    ~SimulationWrapper();

    // ========== 核心 API ==========

    // 初始化模拟世界
    void initialize();

    // 每帧更新 (由 Godot _process 驱动)
    void update(float delta);

    // 设置相机位置 (用于HQ/LQ转换)
    void set_camera_position(Vector3 pos);

    // 获取相机当前所在的Region ID
    int get_current_region(Vector3 pos);

    // 获取模拟时间
    float get_current_time() const;

    // ========== Region 查询 ==========

    // 返回所有 Region 状态
    TypedArray<Dictionary> get_all_regions();

    // 获取单个 Region 状态
    Dictionary get_region(int region_id);

    // ========== Creature 查询 (HQ模式区域) ==========

    // 获取指定 Region 的所有 Creature 数据
    TypedArray<Dictionary> get_creatures_in_region(int region_id);

    // ========== Population 查询 (LQ模式区域) ==========

    // 获取指定 Region 的种群统计数据
    TypedArray<Dictionary> get_populations_in_region(int region_id);

    // ========== 全局统计 ==========

    // 获取全局种群统计 (所有Region的总和)
    Dictionary get_global_statistics();

protected:
    // Godot绑定注册
    static void _bind_methods();

private:
    // 辅助函数：初始化世界种群
    void _initialize_world_populations();

    // 辅助函数：将Region转换为Dictionary
    Dictionary _region_to_dict(uint32_t region_id, const Region& region);

    // 辅助函数：将Creature组件转换为Dictionary
    Dictionary _creature_to_dict(EntityId entity_id);
};
