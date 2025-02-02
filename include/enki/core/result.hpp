#pragma once
/// @file result.hpp
/// @brief Result<T, E> type for error handling without exceptions.
/// Inspired by Rust's Result type and C++23's std::expected.

#include <variant>
#include <string>
#include <string_view>
#include <functional>
#include <stdexcept>
#include <utility>
#include <cassert>

namespace enki {

// ============================================================
// Error Type
// ============================================================

/// Error codes used throughout the framework.
enum class ErrorCode {
    None = 0,
    OutOfMemory,
    InvalidArgument,
    NotFound,
    AlreadyExists,
    PlatformError,
    WindowError,
    RenderingError,
    LayoutError,
    StateError,
    IOError,
    NotInitialized,
    NotSupported,
};

/// Structured error with code and message.
struct Error {
    ErrorCode code = ErrorCode::None;
    std::string message;

    Error() = default;
    Error(ErrorCode c, std::string msg = "")
        : code(c), message(std::move(msg)) {}

    [[nodiscard]] bool isNone() const { return code == ErrorCode::None; }

    [[nodiscard]] std::string_view codeString() const {
        switch (code) {
            case ErrorCode::None:            return "None";
            case ErrorCode::OutOfMemory:     return "OutOfMemory";
            case ErrorCode::InvalidArgument: return "InvalidArgument";
            case ErrorCode::NotFound:        return "NotFound";
            case ErrorCode::AlreadyExists:   return "AlreadyExists";
            case ErrorCode::PlatformError:   return "PlatformError";
            case ErrorCode::WindowError:     return "WindowError";
            case ErrorCode::RenderingError:  return "RenderingError";
            case ErrorCode::LayoutError:     return "LayoutError";
            case ErrorCode::StateError:      return "StateError";
            case ErrorCode::IOError:         return "IOError";
            case ErrorCode::NotInitialized:  return "NotInitialized";
            case ErrorCode::NotSupported:    return "NotSupported";
        }
        return "Unknown";
    }

    bool operator==(const Error& o) const { return code == o.code; }
};

// ============================================================
// Result<T>
// ============================================================

/// A result type that holds either a value of type T or an Error.
/// Usage:
///   Result<int> r = Result<int>::ok(42);
///   Result<int> e = Result<int>::err(ErrorCode::NotFound, "item missing");
///   if (r.isOk()) { use(r.value()); }
template<typename T>
class Result {
public:
    /// Create a successful result.
    static Result ok(T value) {
        Result r;
        r.data_.template emplace<0>(std::move(value));
        return r;
    }

    /// Create an error result.
    static Result err(ErrorCode code, std::string message = "") {
        Result r;
        r.data_.template emplace<1>(Error{code, std::move(message)});
        return r;
    }

    /// Create an error result from an Error object.
    static Result err(Error error) {
        Result r;
        r.data_.template emplace<1>(std::move(error));
        return r;
    }

    /// @return true if the result holds a value.
    [[nodiscard]] bool isOk() const { return data_.index() == 0; }

    /// @return true if the result holds an error.
    [[nodiscard]] bool isErr() const { return data_.index() == 1; }

    /// @return Reference to the held value. Asserts on error.
    [[nodiscard]] T& value() & {
        assert(isOk() && "Attempted to access value of an error Result");
        return std::get<0>(data_);
    }

    [[nodiscard]] const T& value() const& {
        assert(isOk() && "Attempted to access value of an error Result");
        return std::get<0>(data_);
    }

    [[nodiscard]] T&& value() && {
        assert(isOk() && "Attempted to access value of an error Result");
        return std::get<0>(std::move(data_));
    }

    /// @return The value if Ok, or the provided default.
    [[nodiscard]] T valueOr(T default_value) const {
        if (isOk()) return std::get<0>(data_);
        return default_value;
    }

    /// @return Reference to the held error. Asserts on success.
    [[nodiscard]] const Error& error() const {
        assert(isErr() && "Attempted to access error of a successful Result");
        return std::get<1>(data_);
    }

    /// Monadic map: applies f to value if Ok.
    template<typename F>
    auto map(F&& f) const -> Result<std::invoke_result_t<F, const T&>> {
        using U = std::invoke_result_t<F, const T&>;
        if (isOk()) {
            return Result<U>::ok(f(std::get<0>(data_)));
        }
        return Result<U>::err(std::get<1>(data_));
    }

    /// Monadic andThen: applies f (which returns Result<U>) if Ok.
    template<typename F>
    auto andThen(F&& f) const -> std::invoke_result_t<F, const T&> {
        if (isOk()) {
            return f(std::get<0>(data_));
        }
        using U = std::invoke_result_t<F, const T&>;
        return U::err(std::get<1>(data_));
    }

    /// Implicit bool conversion.
    explicit operator bool() const { return isOk(); }

private:
    Result() = default;
    std::variant<T, Error> data_;
};

/// Specialization for void (no value, just success/failure).
template<>
class Result<void> {
public:
    static Result ok() {
        Result r;
        r.is_ok_ = true;
        return r;
    }

    static Result err(ErrorCode code, std::string message = "") {
        Result r;
        r.is_ok_ = false;
        r.error_ = Error{code, std::move(message)};
        return r;
    }

    static Result err(Error error) {
        Result r;
        r.is_ok_ = false;
        r.error_ = std::move(error);
        return r;
    }

    [[nodiscard]] bool isOk() const { return is_ok_; }
    [[nodiscard]] bool isErr() const { return !is_ok_; }
    [[nodiscard]] const Error& error() const {
        assert(isErr());
        return error_;
    }

    explicit operator bool() const { return is_ok_; }

private:
    Result() = default;
    bool is_ok_ = false;
    Error error_;
};

// ============================================================
// Assertion / Check Macros
// ============================================================

/// Assert with message (debug only).
#define ENKI_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            assert(false && (msg)); \
        } \
    } while (0)

/// Check a condition and return an error Result if it fails.
#define ENKI_CHECK(cond, code, msg) \
    do { \
        if (!(cond)) { \
            return Result<void>::err(code, msg); \
        } \
    } while (0)

}  // namespace enki
