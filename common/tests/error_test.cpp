#include <cassert>
#include <stdexcept>
#include <string>

#include "isb/common/error.hpp"

int main() {
    using namespace isb::common;

    const auto ok = Status::success();
    assert(ok.ok());
    assert(ok.code() == ErrorCode::Ok);

    const auto failure = Status::failure(ErrorCode::QueryFailed, "query failed");
    assert(!failure.ok());
    assert(failure.code() == ErrorCode::QueryFailed);
    assert(failure.message() == "query failed");

    const auto invalid_failure = Status::failure(ErrorCode::Ok, "invalid failure");
    assert(!invalid_failure.ok());
    assert(invalid_failure.code() == ErrorCode::InvalidArgument);

    Result<int> success(42);
    assert(success.ok());
    assert(success.value() == 42);

    Result<int> failed(failure);
    assert(!failed.ok());

    Result<int> invalid_result(Status::success());
    assert(!invalid_result.ok());
    assert(invalid_result.status().code() == ErrorCode::InvalidArgument);

    // Test that failed Result correctly reports error state without calling value()
    // Note: calling .value() on a failed Result triggers assert() in debug builds,
    // which aborts the program. The safety contract is: check .ok() before accessing .value().
    // In release builds (NDEBUG defined), .value() throws std::logic_error.
    assert(!failed.ok());
    assert(failed.status().code() == ErrorCode::QueryFailed);
    assert(failed.status().message() == "query failed");

    return 0;
}
