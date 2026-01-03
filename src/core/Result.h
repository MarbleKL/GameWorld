#pragma once
#include "Error.h"
#include <utility>  // std::move
#include <cstdlib>  // abort

namespace core {

// RefWrapper - 用于 Result<RefWrapper<T>> 返回引用
template<typename T>
class RefWrapper {
public:
    explicit RefWrapper(T& ref) : ptr_(&ref) {}

    T& get() const { return *ptr_; }
    T* operator->() const { return ptr_; }
    T& operator*() const { return *ptr_; }

private:
    T* ptr_;
};

// Result<T, E> - Rust 风格的错误处理容器
template<typename T, typename E = ErrorCode>
class Result {
public:
    // 成功构造
    static Result Ok(T&& value) {
        Result r;
        r.is_ok_ = true;
        new (&r.value_) T(std::move(value));
        return r;
    }

    static Result Ok(const T& value) {
        Result r;
        r.is_ok_ = true;
        new (&r.value_) T(value);
        return r;
    }

    // 失败构造
    static Result Err(E error) {
        Result r;
        r.is_ok_ = false;
        r.error_ = error;
        return r;
    }

    // 析构函数
    ~Result() {
        if (is_ok_) {
            value_.~T();
        }
    }

    // 拷贝构造 (如果需要)
    Result(const Result& other) : is_ok_(other.is_ok_) {
        if (is_ok_) {
            new (&value_) T(other.value_);
        } else {
            error_ = other.error_;
        }
    }

    // 移动构造
    Result(Result&& other) noexcept : is_ok_(other.is_ok_) {
        if (is_ok_) {
            new (&value_) T(std::move(other.value_));
        } else {
            error_ = other.error_;
        }
    }

    // 查询状态
    bool is_ok() const { return is_ok_; }
    bool is_err() const { return !is_ok_; }

    // 获取值 (调用前必须检查 is_ok())
    T& value() {
        if (!is_ok_) {
            // Debug 版本可以输出错误信息
            #ifndef NDEBUG
            std::abort();  // 不使用异常，直接终止
            #endif
        }
        return value_;
    }

    const T& value() const {
        if (!is_ok_) {
            #ifndef NDEBUG
            std::abort();
            #endif
        }
        return value_;
    }

    // 获取错误码 (调用前必须检查 is_err())
    E error() const {
        return error_;
    }

    // 便捷方法：如果错误则返回默认值
    T value_or(T&& default_value) const {
        if (is_ok_) {
            return value_;
        } else {
            return std::move(default_value);
        }
    }

private:
    Result() : is_ok_(false), error_(static_cast<E>(0)) {}

    bool is_ok_;
    union {
        T value_;
        E error_;
    };
};

// 特化：Result<void, E> (用于无返回值的操作)
template<typename E>
class Result<void, E> {
public:
    static Result Ok() {
        Result r;
        r.is_ok_ = true;
        return r;
    }

    static Result Err(E error) {
        Result r;
        r.is_ok_ = false;
        r.error_ = error;
        return r;
    }

    bool is_ok() const { return is_ok_; }
    bool is_err() const { return !is_ok_; }

    E error() const { return error_; }

private:
    Result() : is_ok_(false), error_(static_cast<E>(0)) {}

    bool is_ok_;
    E error_;
};

} // namespace core
