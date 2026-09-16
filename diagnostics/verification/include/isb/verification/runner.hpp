#ifndef ISB_VERIFICATION_RUNNER_HPP
#define ISB_VERIFICATION_RUNNER_HPP

#include "isb/verification/assertions.hpp"
#include "isb/verification/snapshot.hpp"
#include "isb/verification/types.hpp"

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <utility>

namespace isb::verification {

class MockBackend {
public:
    virtual ~MockBackend() = default;

    virtual void setupEnvironment(const VerificationConfig& config) = 0;
    virtual void teardownEnvironment() noexcept = 0;

    virtual BackendExecutionResult executeWorkload(
        const VerificationConfig& config,
        std::chrono::milliseconds timeout) = 0;

    virtual Snapshot captureSnapshot() = 0;
};

class DefaultMockBackend : public MockBackend {
public:
    explicit DefaultMockBackend(
        BackendBehaviorMode initial_mode = BackendBehaviorMode::Success);
    ~DefaultMockBackend() override = default;

    void setMode(BackendBehaviorMode mode) noexcept;
    BackendBehaviorMode getMode() const noexcept;
    void setFailureReason(std::string reason);

    void setupEnvironment(const VerificationConfig& config) override;
    void teardownEnvironment() noexcept override;

    BackendExecutionResult executeWorkload(
        const VerificationConfig& config,
        std::chrono::milliseconds timeout) override;

    Snapshot captureSnapshot() override;

private:
    BackendBehaviorMode mode_{BackendBehaviorMode::Success};
    bool environment_setup_{false};
    std::string failure_reason_{};
    Snapshot mock_snapshot_{};
};

class RegressionRunner {
public:
    explicit RegressionRunner(std::shared_ptr<MockBackend> backend);
    ~RegressionRunner() = default;

    RegressionRunner(const RegressionRunner&) = delete;
    RegressionRunner& operator=(const RegressionRunner&) = delete;
    RegressionRunner(RegressionRunner&&) noexcept = default;
    RegressionRunner& operator=(RegressionRunner&&) noexcept = default;

    TestResult runTestCase(const TestCase& test_case);
    TestSuiteResult runTestSuite(const TestSuite& test_suite);

private:
    class ScopeGuard {
    public:
        explicit ScopeGuard(std::function<void()> cleanup)
            : cleanup_(std::move(cleanup)), active_(true) {}

        ~ScopeGuard() noexcept {
            if (active_ && cleanup_) {
                cleanup_();
            }
        }

        void dismiss() noexcept { active_ = false; }

        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;

        ScopeGuard(ScopeGuard&& other) noexcept
            : cleanup_(std::move(other.cleanup_)), active_(other.active_) {
            other.active_ = false;
        }

        ScopeGuard& operator=(ScopeGuard&& other) noexcept {
            if (this != &other) {
                if (active_ && cleanup_) {
                    cleanup_();
                }
                cleanup_ = std::move(other.cleanup_);
                active_ = other.active_;
                other.active_ = false;
            }
            return *this;
        }

    private:
        std::function<void()> cleanup_;
        bool active_{false};
    };

    std::shared_ptr<MockBackend> backend_;
};

} // namespace isb::verification

#endif // ISB_VERIFICATION_RUNNER_HPP
