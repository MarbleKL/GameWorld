#pragma once
#include <cstdint>

namespace core {

// 错误码枚举 (用于 Result<T, E>)
enum class ErrorCode : uint8_t {
    // 通用错误
    OK = 0,
    UNKNOWN = 1,

    // 查找错误 (2-9)
    NOT_FOUND = 2,
    INVALID_ID = 3,
    EMPTY_COLLECTION = 4,

    // 状态错误 (10-19)
    INVALID_STATE = 10,
    ALREADY_EXISTS = 11,
    NOT_INITIALIZED = 12,

    // ECS 错误 (20-29)
    ENTITY_NOT_FOUND = 20,
    COMPONENT_NOT_FOUND = 21,
    ENTITY_TYPE_MISMATCH = 22,

    // 模拟错误 (30-39)
    REGION_NOT_FOUND = 30,
    SPECIES_NOT_FOUND = 31,
    INVALID_REGION_ID = 32,
    INVALID_SPECIES_ID = 33,
};

// 错误信息辅助函数 (仅用于调试输出)
inline const char* error_code_to_string(ErrorCode code) {
    switch (code) {
        case ErrorCode::OK: return "OK";
        case ErrorCode::NOT_FOUND: return "NOT_FOUND";
        case ErrorCode::INVALID_ID: return "INVALID_ID";
        case ErrorCode::EMPTY_COLLECTION: return "EMPTY_COLLECTION";
        case ErrorCode::INVALID_STATE: return "INVALID_STATE";
        case ErrorCode::ALREADY_EXISTS: return "ALREADY_EXISTS";
        case ErrorCode::NOT_INITIALIZED: return "NOT_INITIALIZED";
        case ErrorCode::ENTITY_NOT_FOUND: return "ENTITY_NOT_FOUND";
        case ErrorCode::COMPONENT_NOT_FOUND: return "COMPONENT_NOT_FOUND";
        case ErrorCode::ENTITY_TYPE_MISMATCH: return "ENTITY_TYPE_MISMATCH";
        case ErrorCode::REGION_NOT_FOUND: return "REGION_NOT_FOUND";
        case ErrorCode::SPECIES_NOT_FOUND: return "SPECIES_NOT_FOUND";
        case ErrorCode::INVALID_REGION_ID: return "INVALID_REGION_ID";
        case ErrorCode::INVALID_SPECIES_ID: return "INVALID_SPECIES_ID";
        default: return "UNKNOWN";
    }
}

} // namespace core
