#pragma once

#include <cassert>
#include <stdexcept>
#include <optional>
#include <string>
#include <utility>

namespace isb::common {

enum class ErrorCode {
    Ok = 0,
    InvalidArgument,
    NotFound,
    Unavailable,
    InitializationFailed,
    QueryFailed,
    Unsupported,
};

class Status {
public:
    Status() = default;

    static Status success() noexcept { return Status{}; }
    static Status failure(ErrorCode code, std::string message) {
        if (code == ErrorCode::Ok) {
            return Status(ErrorCode::InvalidArgument,
                          "failure status cannot use ErrorCode::Ok");
        }
        return Status(code, std::move(message));
    }

    bool ok() const noexcept { return code_ == ErrorCode::Ok; }
    ErrorCode code() const noexcept { return code_; }
    const std::string& message() const noexcept { return message_; }

private:
    Status(ErrorCode code, std::string message)
        : code_(code), message_(std::move(message)) {}

    ErrorCode code_ = ErrorCode::Ok;
    std::string message_;
};

template <typename T>
class Result {
public:
    Result(T value) : value_(std::move(value)), status_(Status::success()) {}
    Result(Status status) : status_(std::move(status)) {
        if (status_.ok()) {
            status_ = Status::failure(
                ErrorCode::InvalidArgument,
                "Result failure constructor requires a non-OK status");
        }
    }

    bool ok() const noexcept { return status_.ok(); }
    const Status& status() const noexcept { return status_; }

    T& value() & {
        assert(ok() && value_.has_value());
        if (!ok() || !value_.has_value()) {
            throw std::logic_error("Result does not contain a value");
        }
        return *value_;
    }
    const T& value() const & {
        assert(ok() && value_.has_value());
        if (!ok() || !value_.has_value()) {
            throw std::logic_error("Result does not contain a value");
        }
        return *value_;
    }
    T&& value() && {
        assert(ok() && value_.has_value());
        if (!ok() || !value_.has_value()) {
            throw std::logic_error("Result does not contain a value");
        }
        return std::move(*value_);
    }

private:
    std::optional<T> value_;
    Status status_;
};

} // namespace isb::common
