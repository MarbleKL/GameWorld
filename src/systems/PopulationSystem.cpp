#include "PopulationSystem.h"

void PopulationSystem::update(float dt) {
    // 执行所有种群增长Process
    scheduler_.execute_all_population_growth(dt);
}
