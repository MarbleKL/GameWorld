#pragma once

#include "AtomicProcesses.h"

// ============================================================
// ProcessScheduler - Process调度和组合
// 提供高层Process组合接口
// ============================================================

namespace process {

class ProcessScheduler {
public:
    ProcessScheduler(ProcessContext& ctx)
        : ctx_(ctx),
          update_pop_growth_(),
          spawn_creatures_(),
          aggregate_creatures_(),
          process_lifecycle_(),
          process_migration_() {}

    // 执行所有种群增长Process
    void execute_all_population_growth(float dt);

    // 执行所有个体生命周期Process
    void execute_all_creature_lifecycle(float dt);

    // LQ→HQ转换：生成个体
    void convert_lq_to_hq(EntityId pop_id, uint32_t spawn_count);

    // HQ→LQ转换：聚合统计并销毁个体
    void convert_hq_to_lq(uint32_t region_id, SpeciesId species_id);

    // 访问ProcessContext（供ConversionSystem使用）
    ProcessContext& ctx_;

private:
    // 原子Process实例
    UpdatePopulationGrowth update_pop_growth_;
    SpawnCreaturesFromPopulation spawn_creatures_;
    AggregateCreaturesToPopulation aggregate_creatures_;
    ProcessCreatureLifecycle process_lifecycle_;
    ProcessMigration process_migration_;
};

} // namespace process
