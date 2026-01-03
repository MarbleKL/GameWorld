#pragma once

#include "process/ProcessScheduler.h"

// ============================================================
// PopulationSystem - 种群级模拟系统（LQ模式）
// ============================================================

class PopulationSystem {
public:
    PopulationSystem(process::ProcessScheduler& scheduler)
        : scheduler_(scheduler) {}

    // 主更新循环
    void update(float dt);

private:
    process::ProcessScheduler& scheduler_;
};
