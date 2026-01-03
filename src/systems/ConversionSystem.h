#pragma once

#include "process/ProcessScheduler.h"
#include "simulation/SimulationState.h"

// ============================================================
// ConversionSystem - HQ/LQ模式转换系统
// 基于Region的target_mode进行转换
// ============================================================

class ConversionSystem {
public:
    ConversionSystem(process::ProcessScheduler& scheduler, SimulationState& state)
        : scheduler_(scheduler), state_(state) {}

    // 检查所有Region并触发HQ/LQ转换（基于target_mode）
    void update_region_modes();

private:
    process::ProcessScheduler& scheduler_;
    SimulationState& state_;
};
