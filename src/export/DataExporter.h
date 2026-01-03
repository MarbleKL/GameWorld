#pragma once

#include "ecs/Registry.h"
#include "simulation/SimulationState.h"
#include <string>
#include <fstream>

// ============================================================
// DataExporter - CSV数据导出器
// ============================================================

class DataExporter {
public:
    DataExporter(const std::string& filepath);
    ~DataExporter();

    // 写入一个时间步的数据
    void write_timestep(float time, const ecs::Registry& registry, const SimulationState& state);

    // 完成导出
    void finalize();

private:
    std::ofstream file_;
    bool is_open_;

    void write_header();
};
