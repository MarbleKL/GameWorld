#pragma once

#include "process/ProcessScheduler.h"

// ============================================================
// CreatureSystem - 个体级模拟系统（HQ模式）
// ============================================================

class CreatureSystem {
public:
    CreatureSystem(process::ProcessScheduler& scheduler)
        : scheduler_(scheduler) {}

    // 主更新循环
    void update(float dt);

private:
    process::ProcessScheduler& scheduler_;
};
