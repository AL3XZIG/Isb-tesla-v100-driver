#pragma once

#include "isb/cal/capabilities.hpp"

#include <optional>
#include <string>
#include <vector>

namespace isb::hub {

enum class ProviderMode { Mock, Real, Unavailable };
enum class ControlState { Supported, Unsupported, Unknown, PermissionDenied, Error };
enum class OperationState { Planned, Applied, Verified, Failed, RolledBack };

struct Provenance {
    std::string provider;
    std::string detail;
    bool synthetic = false;
};

struct Environment {
    std::string os;
    std::string driver_version;
    ProviderMode mode = ProviderMode::Unavailable;
    Provenance provenance;
};

struct CapabilitySnapshot {
    cal::GpuCapabilities capabilities;
    Provenance provenance;
};

struct TelemetrySnapshot {
    std::string timestamp;
    bool synthetic = false;
    Provenance provenance;
    std::optional<int> temperature_c;
    std::optional<int> gpu_utilization_percent;
    std::optional<int> memory_utilization_percent;
    std::optional<int> power_w;
    std::optional<int> power_limit_w;
    std::optional<int> gpu_clock_mhz;
    std::optional<int> memory_clock_mhz;
    std::optional<int> vram_used_mib;
    std::optional<int> vram_total_mib;
    std::string performance_state;
    std::string ecc;
    std::string pcie;
    std::string nvlink;
    std::optional<int> process_count;
    std::string driver;
};

struct Control {
    std::string name;
    ControlState state = ControlState::Unknown;
    std::string reason;
    std::string current_value;
    std::string requested_value;
    std::string applied_value;
    bool requires_restart = false;
    bool requires_reset = false;
    bool reversible = false;
};

struct Operation {
    std::string id;
    std::string description;
    ControlState availability = ControlState::Unknown;
    bool requires_approval = true;
    bool reversible = false;
    std::string rollback;
    std::string requested_value;
    std::string current_value;
    std::string resulting_value;
    bool requires_restart = false;
    bool requires_reset = false;
};

struct OperationPlan {
    std::string id;
    bool dry_run = true;
    std::vector<Operation> operations;
    std::vector<std::string> unsupported;
    std::vector<std::string> unknown;
    std::vector<std::string> risks;
    std::vector<std::string> verification_steps;
};

struct VerificationResult {
    bool verified = false;
    std::string code;
    std::string message;
    Provenance provenance;
};

struct ApplyResult {
    OperationState state = OperationState::Planned;
    bool mutated = false;
    VerificationResult verification;
};

struct ErrorEvent {
    std::string code;
    std::string message;
    Provenance provenance;
};

struct BenchmarkResult {
    std::string name;
    std::string version;
    bool synthetic = false;
    bool correctness_verified = false;
    std::string result;
    Provenance provenance;
};

struct ReportBundle {
    std::string directory;
    std::string manifest;
};

class Provider {
public:
    virtual ~Provider() = default;
    virtual Environment environment() const = 0;
    virtual cal::GpuCapabilities capabilities() const = 0;
    virtual TelemetrySnapshot telemetry() const = 0;
    virtual std::vector<Control> controls() const = 0;
};

class MockProvider final : public Provider {
public:
    Environment environment() const override;
    cal::GpuCapabilities capabilities() const override;
    TelemetrySnapshot telemetry() const override;
    std::vector<Control> controls() const override;
};

class UnavailableProvider final : public Provider {
public:
    Environment environment() const override;
    cal::GpuCapabilities capabilities() const override;
    TelemetrySnapshot telemetry() const override;
    std::vector<Control> controls() const override;
};

class Hub {
public:
    explicit Hub(const Provider& provider);
    Environment environment() const;
    CapabilitySnapshot capability_snapshot() const;
    cal::GpuCapabilities capabilities() const;
    TelemetrySnapshot telemetry() const;
    std::vector<Control> controls() const;
    std::vector<std::string> profiles() const;
    OperationPlan profile_plan(const std::string& profile) const;
    OperationPlan optimize_plan() const;
    std::vector<ErrorEvent> diagnose() const;
    VerificationResult verify() const;
    BenchmarkResult benchmark() const;
    ApplyResult apply(const OperationPlan& plan, bool approved) const;
    ReportBundle report(const std::string& directory) const;
    std::string status_json() const;

private:
    const Provider& provider_;
};

std::string json(const Environment&);
std::string json(const TelemetrySnapshot&);
std::string json(const OperationPlan&);
std::string json(const BenchmarkResult&);
std::string json(const VerificationResult&);
const char* to_string(ProviderMode);
const char* to_string(ControlState);
const char* to_string(OperationState);

} // namespace isb::hub
