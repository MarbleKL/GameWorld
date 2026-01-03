#include "CreatureSystem.h"

void CreatureSystem::update(float dt) {
    // 执行所有个体生命周期Process
    scheduler_.execute_all_creature_lifecycle(dt);
}
