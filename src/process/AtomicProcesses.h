#pragma once

#include "ProcessContext.h"
#include "components/Components.h"
#include <random>

// ============================================================
// 原子Process库
// 实现5个核心Process：
// 1. UpdatePopulationGrowth - 种群增长（LQ模式）
// 2. SpawnCreaturesFromPopulation - 从种群生成个体（LQ→HQ）
// 3. AggregateCreaturesToPopulation - 个体聚合到种群（HQ→LQ）
// 4. ProcessCreatureLifecycle - 个体生命周期（HQ模式）
// 5. ProcessMigration - 迁移（暂时简化）
// ============================================================

namespace process {

// ========== Process 1: UpdatePopulationGrowth ==========
// 使用Logistic增长模型更新种群数量
class UpdatePopulationGrowth {
public:
    void execute(ProcessContext& ctx, EntityId pop_id, float dt);

private:
    float calculate_growth_rate(
        const component::Population& pop,
        const Region& region,
        const SpeciesTemplate& species,
        ProcessContext& ctx);

    float calculate_predation_loss(
        const component::Population& pop,
        const Region& region,
        const SpeciesTemplate& species,
        ProcessContext& ctx);
};

// ========== Process 2: SpawnCreaturesFromPopulation ==========
// 从种群统计分布采样生成N个个体
class SpawnCreaturesFromPopulation {
public:
    void execute(ProcessContext& ctx, EntityId pop_id, uint32_t count);

private:
    GameplayGene sample_gene_from_distribution(
        const component::Population& pop,
        const SpeciesTemplate& species,
        uint32_t seed);

    std::mt19937 rng_{std::random_device{}()};
};

// ========== Process 3: AggregateCreaturesToPopulation ==========
// 统计区域内所有某物种的个体，更新种群数据
class AggregateCreaturesToPopulation {
public:
    void execute(ProcessContext& ctx, uint32_t region_id, SpeciesId species_id);

private:
    void calculate_statistics(
        const std::vector<GameplayGene>& genes,
        component::Population& pop);
};

// ========== Process 4: ProcessCreatureLifecycle ==========
// 处理单个生物的生命周期（年龄、饥饿、死亡检查）
class ProcessCreatureLifecycle {
public:
    void execute(ProcessContext& ctx, EntityId creature_id, float dt);

private:
    bool check_death_conditions(const component::Lifecycle& life);
};

// ========== Process 5: ProcessMigration ==========
// 处理迁移（暂时简化，不实现复杂逻辑）
class ProcessMigration {
public:
    void execute(ProcessContext& ctx, EntityId entity_id, uint32_t target_region);
};

} // namespace process
