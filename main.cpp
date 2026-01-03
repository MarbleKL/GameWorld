#include <iostream>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ecs/Registry.h"
#include "process/EffectRecorder.h"
#include "process/ProcessContext.h"
#include "process/ProcessScheduler.h"
#include "simulation/SimulationState.h"
#include "systems/PopulationSystem.h"
#include "systems/CreatureSystem.h"
#include "systems/ConversionSystem.h"
#include "export/DataExporter.h"
#include "components/Components.h"

// ============================================================
// GameWorld ERPE生态模拟主程序
// 演示种群涌现、HQ/LQ切换、捕食者-猎物动态
// ============================================================

void initialize_populations(ecs::Registry& registry, SimulationState& state) {
    std::cout << "\n=== Initializing World Populations ===" << std::endl;

    const auto& all_species = state.get_all_species_templates();
    const auto& all_regions = state.get_all_regions();

    // 为每个Region创建物种种群
    for (const auto& [region_id, region] : all_regions) {
        for (const auto& species : all_species) {
            // 跳过某些不适合的组合（例如熊只在部分Region出现）
            if (species.id == 3 && (region_id == 4 || region_id == 5)) {
                continue;  // 熊不在沼泽和河流出现
            }

            // 创建Population实体
            EntityId pop_id = registry.create_entity(EntityType::Population);

            // 初始种群数量
            uint32_t initial_count = 0;
            if (species.id == 1) {  // Rabbit
                initial_count = 100 + (region_id * 20);  // 100-220
            } else if (species.id == 2) {  // Wolf
                initial_count = 10 + (region_id * 2);   // 10-22
            } else if (species.id == 3) {  // Bear
                initial_count = 5 + region_id;          // 6-11
            }

            // 添加Population组件
            registry.add_component(pop_id, component::Population{
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

            std::cout << "  Created Population: " << species.name
                      << " in " << region.name
                      << " (initial count: " << initial_count << ")" << std::endl;
        }
    }

    std::cout << "World initialization complete!\n" << std::endl;
}

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    std::cout << "╔═══════════════════════════════════════════════════╗\n";
    std::cout << "║  GameWorld - ERPE Ecosystem Simulation           ║\n";
    std::cout << "║  ERPE生态模拟系统 - 种群涌现与HQ/LQ切换演示       ║\n";
    std::cout << "╚═══════════════════════════════════════════════════╝\n";

    // 创建输出目录
    std::filesystem::create_directories("./output");

    // ========== 1. 初始化核心组件 ==========
    std::cout << "\n[1/7] Initializing core components..." << std::endl;
    ecs::Registry registry;
    ecs::EffectRecorder recorder;
    SimulationState state;

    state.initialize();

    // ========== 2. 创建数据导出器 ==========
    std::cout << "[2/7] Creating data exporter..." << std::endl;
    DataExporter exporter("output/simulation_data.csv");

    // ========== 3. 创建Process和System ==========
    std::cout << "[3/7] Creating process scheduler and systems..." << std::endl;
    ProcessContext ctx(registry, recorder, state);
    process::ProcessScheduler scheduler(ctx);
    PopulationSystem pop_system(scheduler);
    CreatureSystem creature_system(scheduler);
    ConversionSystem conversion_system(scheduler, state);

    // ========== 4. 初始化世界（创建种群） ==========
    std::cout << "[4/7] Initializing world populations..." << std::endl;
    initialize_populations(registry, state);

    // ========== 5. 主模拟循环 ==========
    std::cout << "\n[5/7] Starting simulation..." << std::endl;
    std::cout << "========================================\n" << std::endl;

    const float dt = 1.0f;              // 每步1天
    const float total_time = 500.0f;    // 总共500天
    const uint32_t log_interval = 10;   // 每10步输出一次

    for (uint32_t step = 0; state.current_time < total_time; ++step) {
        recorder.clear();

        // HQ/LQ转换检查
        conversion_system.update_region_modes();

        // 更新种群（LQ区域）
        pop_system.update(dt);

        // 更新个体（HQ区域）
        creature_system.update(dt);

        // 记录数据（每log_interval步）
        if (step % log_interval == 0) {
            exporter.write_timestep(state.current_time, registry, state);

            // 简单控制台输出
            if (step % (log_interval * 5) == 0) {  // 每50步输出摘要
                const auto& all_pops = registry.view<component::Population>();
                uint32_t total_rabbits = 0, total_wolves = 0, total_bears = 0;

                for (EntityId pop_id : all_pops) {
                    const auto& pop = registry.get_component<component::Population>(pop_id);
                    if (pop.species_id == 1) total_rabbits += pop.estimated_count;
                    else if (pop.species_id == 2) total_wolves += pop.estimated_count;
                    else if (pop.species_id == 3) total_bears += pop.estimated_count;
                }

                std::cout << "t=" << static_cast<int>(state.current_time)
                          << " | Rabbits: " << total_rabbits
                          << " | Wolves: " << total_wolves
                          << " | Bears: " << total_bears
                          << " | Effects: " << recorder.size() << std::endl;
            }
        }

        // 推进时间
        state.current_time += dt;
    }

    // ========== 6. 完成并提示 ==========
    std::cout << "\n========================================" << std::endl;
    std::cout << "[6/7] Simulation complete!" << std::endl;

    exporter.finalize();

    std::cout << "\n✅ Data exported to: output/simulation_data.csv" << std::endl;
    std::cout << "\n📊 To visualize results, run:" << std::endl;
#ifdef _WIN32
    std::cout << "   python\\python.exe python\\visualize.py output\\simulation_data.csv" << std::endl;
#else
    std::cout << "   python python/visualize.py output/simulation_data.csv" << std::endl;
#endif

    std::cout << "\n╔═══════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║  ERPE Simulation Demo Complete!                  ║" << std::endl;
    std::cout << "╚═══════════════════════════════════════════════════╝\n" << std::endl;

    return 0;
}
