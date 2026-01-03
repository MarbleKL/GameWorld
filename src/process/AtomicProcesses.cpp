#include "AtomicProcesses.h"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace process {

// ========== Process 1: UpdatePopulationGrowth ==========

void UpdatePopulationGrowth::execute(ProcessContext& ctx, EntityId pop_id, float dt) {
    auto& pop = ctx.get<component::Population>(pop_id);

    // 只在Simulated模式下更新
    if (pop.mode != component::Population::Mode::Simulated) {
        return;
    }

    // 获取 Region 和 Species (处理错误)
    auto region_result = ctx.get_region(pop.region_id);
    if (region_result.is_err()) {
        #ifndef NDEBUG
        std::cerr << "Error: Region not found for population " << pop_id << std::endl;
        #endif
        return;
    }
    const auto& region = region_result.value().get();

    auto species_result = ctx.get_species_template(pop.species_id);
    if (species_result.is_err()) {
        #ifndef NDEBUG
        std::cerr << "Error: Species not found for population " << pop_id << std::endl;
        #endif
        return;
    }
    const auto& species = species_result.value().get();

    uint32_t old_count = pop.estimated_count;

    // Logistic增长模型：dN/dt = r*N*(1 - N/K) - predation
    float growth_rate = calculate_growth_rate(pop, region, species, ctx);
    float predation_loss = calculate_predation_loss(pop, region, species, ctx);

    float dN = growth_rate * pop.estimated_count * dt - predation_loss * dt;

    // 更新种群数量（确保非负）
    float new_count_f = std::max(0.0f, static_cast<float>(old_count) + dN);
    pop.estimated_count = static_cast<uint32_t>(new_count_f);

    // 记录Effect
    if (pop.estimated_count != old_count) {
        ctx.record(effect::ResourceChanged{
            pop_id,
            "estimated_count",
            static_cast<float>(old_count),
            static_cast<float>(pop.estimated_count)
        });
    }

    // 检查灭绝
    if (pop.estimated_count == 0 && old_count > 0) {
        ctx.record(effect::Death{pop_id, "population_extinction"});
    }
}

float UpdatePopulationGrowth::calculate_growth_rate(const component::Population& pop,
                                                      const Region& region,
                                                      const SpeciesTemplate& species,
                                                      ProcessContext& ctx) {
    // r = birth_rate - death_rate
    float r = pop.birth_rate - pop.death_rate;

    // Carrying capacity: K = food_capacity / food_requirement
    float K = region.food_capacity / species.food_requirement;

    // Logistic factor: (1 - N/K)
    float logistic_factor = 1.0f - (pop.estimated_count / K);
    logistic_factor = std::max(0.0f, logistic_factor);  // 避免负增长过度

    return r * logistic_factor;
}

float UpdatePopulationGrowth::calculate_predation_loss(const component::Population& pop,
                                                         const Region& region,
                                                         const SpeciesTemplate& species,
                                                         ProcessContext& ctx) {
    float total_loss = 0.0f;

    // 查找所有在同一区域的捕食者种群
    const auto& all_pops = ctx.get_registry().view<component::Population>();

    for (EntityId predator_pop_id : all_pops) {
        const auto& predator_pop = ctx.get<component::Population>(predator_pop_id);

        // 必须在同一区域
        if (predator_pop.region_id != pop.region_id) {
            continue;
        }

        // 获取捕食者物种模板
        auto predator_species_result = ctx.get_species_template(predator_pop.species_id);
        if (predator_species_result.is_err()) {
            continue;  // 跳过无效的物种
        }
        const auto& predator_species = predator_species_result.value().get();

        // 检查是否捕食当前物种
        bool is_prey = false;
        for (SpeciesId prey_id : predator_species.prey_species) {
            if (prey_id == pop.species_id) {
                is_prey = true;
                break;
            }
        }

        if (is_prey) {
            // 计算被捕食数量：predator_count * hunt_efficiency * prey_count
            float kill_rate = predator_pop.estimated_count * predator_species.hunt_efficiency;
            total_loss += kill_rate;
        }
    }

    return total_loss;
}

// ========== Process 2: SpawnCreaturesFromPopulation ==========

void SpawnCreaturesFromPopulation::execute(ProcessContext& ctx, EntityId pop_id, uint32_t count) {
    const auto& pop = ctx.get<component::Population>(pop_id);

    auto species_result = ctx.get_species_template(pop.species_id);
    if (species_result.is_err()) {
        #ifndef NDEBUG
        std::cerr << "Error: Species not found for population " << pop_id << std::endl;
        #endif
        return;
    }
    const auto& species = species_result.value().get();

    // 限制最大生成数量
    const uint32_t MAX_SPAWN = 200;
    count = std::min(count, MAX_SPAWN);
    count = std::min(count, pop.estimated_count);

    for (uint32_t i = 0; i < count; ++i) {
        // 1. 创建Creature实体
        EntityId creature_id = ctx.create_entity(EntityType::Creature);

        // 2. 采样基因
        GameplayGene gene = sample_gene_from_distribution(pop, species, i);

        // 3. 添加组件
        ctx.get_registry().add_component(creature_id, component::GameplayGene{gene});
        ctx.get_registry().add_component(creature_id, component::SpeciesRef{pop.species_id});
        ctx.get_registry().add_component(creature_id, component::Position{
            pop.region_id,
            Vec3{0.0f, 0.0f, 0.0f}  // 简化：都在原点
        });

        // 4. 添加生命周期组件
        std::uniform_real_distribution<float> age_dist(0.0f, species.maturity_age);
        ctx.get_registry().add_component(creature_id, component::Lifecycle{
            age_dist(rng_),              // 随机年龄
            species.average_lifespan,    // 预期寿命
            0.0f,                        // 初始无饥饿
            1.0f                         // 满健康
        });

        ctx.record(effect::ComponentAdded{creature_id, "GameplayGene"});
        ctx.record(effect::ComponentAdded{creature_id, "SpeciesRef"});
        ctx.record(effect::ComponentAdded{creature_id, "Position"});
        ctx.record(effect::ComponentAdded{creature_id, "Lifecycle"});
    }

    std::cout << "[SpawnCreaturesFromPopulation] Spawned " << count << " creatures of species "
              << pop.species_id << " in region " << pop.region_id << std::endl;
}

GameplayGene SpawnCreaturesFromPopulation::sample_gene_from_distribution(
    const component::Population& pop,
    const SpeciesTemplate& species,
    uint32_t seed) {

    std::mt19937 local_rng(seed + static_cast<uint32_t>(pop.species_id) * 1000);

    GameplayGene gene;

    // 从正态分布采样（使用种群统计数据或物种模板）
    std::normal_distribution<float> limb_dist(
        pop.avg_limb_length > 0 ? pop.avg_limb_length : species.limb_length_mean,
        pop.std_limb_length > 0 ? pop.std_limb_length : species.limb_length_std
    );
    std::normal_distribution<float> mass_dist(
        pop.avg_body_mass > 0 ? pop.avg_body_mass : species.body_mass_mean,
        pop.std_body_mass > 0 ? pop.std_body_mass : species.body_mass_std
    );
    std::normal_distribution<float> scale_dist(
        pop.avg_size_scale > 0 ? pop.avg_size_scale : species.size_scale_mean,
        pop.std_size_scale > 0 ? pop.std_size_scale : species.size_scale_std
    );

    gene.limb_length = std::max(0.1f, limb_dist(local_rng));
    gene.body_mass = std::max(0.1f, mass_dist(local_rng));
    gene.size_scale = std::max(0.5f, scale_dist(local_rng));

    // 其他属性使用物种默认值
    std::normal_distribution<float> str_dist(species.base_strength_mean, species.base_strength_std);
    std::normal_distribution<float> agi_dist(species.base_agility_mean, species.base_agility_std);
    std::normal_distribution<float> end_dist(species.base_endurance_mean, species.base_endurance_std);
    std::normal_distribution<float> int_dist(species.base_intellect_mean, species.base_intellect_std);

    gene.base_strength = std::max(1.0f, str_dist(local_rng));
    gene.base_agility = std::max(1.0f, agi_dist(local_rng));
    gene.base_endurance = std::max(1.0f, end_dist(local_rng));
    gene.base_intellect = std::max(1.0f, int_dist(local_rng));

    gene.has_wings = false;
    gene.has_horn = false;

    return gene;
}

// ========== Process 3: AggregateCreaturesToPopulation ==========

void AggregateCreaturesToPopulation::execute(ProcessContext& ctx, uint32_t region_id, SpeciesId species_id) {
    // 1. 找到或创建Population实体
    EntityId pop_id = 0;
    const auto& all_pops = ctx.get_registry().view<component::Population>();

    for (EntityId pid : all_pops) {
        const auto& p = ctx.get<component::Population>(pid);
        if (p.region_id == region_id && p.species_id == species_id) {
            pop_id = pid;
            break;
        }
    }

    if (pop_id == 0) {
        // 创建新的Population实体
        pop_id = ctx.create_entity(EntityType::Population);

        auto species_result = ctx.get_species_template(species_id);
        if (species_result.is_err()) {
            #ifndef NDEBUG
            std::cerr << "Error: Species not found when aggregating creatures" << std::endl;
            #endif
            return;
        }
        const auto& species = species_result.value().get();

        ctx.get_registry().add_component(pop_id, component::Population{
            species_id,
            region_id,
            0,  // 初始数量
            species.base_birth_rate,
            species.base_death_rate,
            component::Population::Mode::DerivedFromIndividuals,
            0, 0, 0, 0, 0, 0  // 统计数据待计算
        });
    }

    // 2. 收集该区域内所有该物种的Creature
    std::vector<GameplayGene> genes;
    const auto& all_creatures = ctx.get_registry().view<component::SpeciesRef>();

    for (EntityId cid : all_creatures) {
        if (!ctx.has<component::Position>(cid) || !ctx.has<component::GameplayGene>(cid)) {
            continue;
        }

        const auto& species_ref = ctx.get<component::SpeciesRef>(cid);
        const auto& pos = ctx.get<component::Position>(cid);

        if (pos.region_id == region_id && species_ref.species_id == species_id) {
            const auto& gene_comp = ctx.get<component::GameplayGene>(cid);
            genes.push_back(gene_comp.gene);
        }
    }

    // 3. 计算统计数据
    auto& pop = ctx.get<component::Population>(pop_id);
    uint32_t old_count = pop.estimated_count;
    pop.estimated_count = static_cast<uint32_t>(genes.size());

    calculate_statistics(genes, pop);

    // 4. 记录Effect
    ctx.record(effect::ResourceChanged{
        pop_id,
        "estimated_count",
        static_cast<float>(old_count),
        static_cast<float>(pop.estimated_count)
    });

    std::cout << "[AggregateCreaturesToPopulation] Aggregated " << genes.size()
              << " creatures into population " << pop_id << " (species " << species_id
              << ", region " << region_id << ")" << std::endl;
}

void AggregateCreaturesToPopulation::calculate_statistics(
    const std::vector<GameplayGene>& genes,
    component::Population& pop) {

    if (genes.empty()) {
        pop.avg_limb_length = 0;
        pop.avg_body_mass = 0;
        pop.avg_size_scale = 0;
        pop.std_limb_length = 0;
        pop.std_body_mass = 0;
        pop.std_size_scale = 0;
        return;
    }

    // 计算均值
    float sum_limb = 0, sum_mass = 0, sum_scale = 0;
    for (const auto& gene : genes) {
        sum_limb += gene.limb_length;
        sum_mass += gene.body_mass;
        sum_scale += gene.size_scale;
    }

    pop.avg_limb_length = sum_limb / genes.size();
    pop.avg_body_mass = sum_mass / genes.size();
    pop.avg_size_scale = sum_scale / genes.size();

    // 计算标准差
    float var_limb = 0, var_mass = 0, var_scale = 0;
    for (const auto& gene : genes) {
        var_limb += (gene.limb_length - pop.avg_limb_length) * (gene.limb_length - pop.avg_limb_length);
        var_mass += (gene.body_mass - pop.avg_body_mass) * (gene.body_mass - pop.avg_body_mass);
        var_scale += (gene.size_scale - pop.avg_size_scale) * (gene.size_scale - pop.avg_size_scale);
    }

    pop.std_limb_length = std::sqrt(var_limb / genes.size());
    pop.std_body_mass = std::sqrt(var_mass / genes.size());
    pop.std_size_scale = std::sqrt(var_scale / genes.size());
}

// ========== Process 4: ProcessCreatureLifecycle ==========

void ProcessCreatureLifecycle::execute(ProcessContext& ctx, EntityId creature_id, float dt) {
    if (!ctx.has<component::Lifecycle>(creature_id)) {
        return;
    }

    auto& life = ctx.get<component::Lifecycle>(creature_id);

    float old_age = life.age;
    float old_hunger = life.hunger;

    // 1. 年龄增长
    life.age += dt;

    // 2. 饥饿增加（简化：每天增加0.1）
    life.hunger += 0.1f * dt;
    life.hunger = std::min(1.0f, life.hunger);

    // 3. 检查死亡条件
    bool should_die = check_death_conditions(life);

    if (should_die) {
        std::string cause = "unknown";
        if (life.hunger > 0.95f) cause = "starvation";
        else if (life.health < 0.05f) cause = "illness";
        else if (life.age > life.lifespan) cause = "old_age";

        ctx.record(effect::Death{creature_id, cause});
        ctx.destroy_entity(creature_id, cause);
    } else {
        // 记录变化
        if (std::abs(life.age - old_age) > 0.01f) {
            ctx.record(effect::ResourceChanged{
                creature_id, "age", old_age, life.age
            });
        }
        if (std::abs(life.hunger - old_hunger) > 0.01f) {
            ctx.record(effect::ResourceChanged{
                creature_id, "hunger", old_hunger, life.hunger
            });
        }
    }
}

bool ProcessCreatureLifecycle::check_death_conditions(const component::Lifecycle& life) {
    if (life.hunger > 0.95f) return true;
    if (life.health < 0.05f) return true;
    if (life.age > life.lifespan) return true;
    return false;
}

// ========== Process 5: ProcessMigration ==========

void ProcessMigration::execute(ProcessContext& ctx, EntityId entity_id, uint32_t target_region) {
    // 暂时简化：只支持Creature迁移，修改Position组件
    if (!ctx.has<component::Position>(entity_id)) {
        return;
    }

    auto& pos = ctx.get<component::Position>(entity_id);
    uint32_t old_region = pos.region_id;

    if (old_region == target_region) {
        return;
    }

    pos.region_id = target_region;

    ctx.record(effect::Migration{
        entity_id,
        old_region,
        target_region,
        1.0f  // Creature迁移数量为1
    });
}

} // namespace process
