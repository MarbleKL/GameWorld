#pragma once

#include "Effect.h"
#include <vector>
#include <string>
#include <fstream>

// ============================================================
// Effect记录器 - 收集并记录所有状态变化
// ============================================================

namespace ecs {

class EffectRecorder {
public:
    EffectRecorder() = default;

    // 记录单个Effect
    void record(effect::Effect&& e);

    // 获取所有Effect
    const std::vector<effect::Effect>& get_effects() const {
        return effects_;
    }

    // 清空缓冲区
    void clear() {
        effects_.clear();
    }

    // 获取Effect数量
    size_t size() const {
        return effects_.size();
    }

    // 格式化输出到控制台
    void log_to_console() const;

    // 输出到日志文件
    void log_to_file(const std::string& path) const;

private:
    std::vector<effect::Effect> effects_;

    // 将Effect转换为字符串
    std::string effect_to_string(const effect::Effect& e) const;
};

} // namespace ecs
