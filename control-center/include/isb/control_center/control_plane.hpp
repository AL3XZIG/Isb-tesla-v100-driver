#pragma once

#include "isb/cal/capabilities.hpp"

#include <optional>
#include <string>
#include <vector>

namespace isb::control_center {

enum class OperationState { Success, Failed, Unsupported, Unknown, PermissionDenied, RequiresRestart, RequiresElevation, PartiallyApplied };
enum class Profile { Balanced, Gaming, Compute, AiTensor, MaximumPerformance, LowPower, Custom };
enum class GraphicsBackend { Native, Dlss, Fsr, Xess, OptiScaler };

struct Metric { std::string label; std::optional<std::string> value; };
struct Control { std::string id; std::string label; cal::CapabilityState capability = cal::CapabilityState::Unknown; std::optional<std::string> current; std::optional<std::string> requested; bool editable = false; std::string detail; };
struct PlanItem { std::string label; std::string before; std::string after; bool reversible = false; };
struct OperationPlan { std::vector<std::string> stages; std::vector<PlanItem> changes; OperationState verification = OperationState::Unknown; std::string risk; };
struct ApplicationProfile { std::string name; std::string executable; std::string api; Profile profile = Profile::Balanced; cal::CapabilityState compatibility = cal::CapabilityState::Unknown; std::vector<GraphicsBackend> compatible_backends; };
struct Snapshot { bool mock = false; cal::GpuCapabilities capabilities; std::vector<Metric> telemetry; std::vector<Control> controls; std::vector<ApplicationProfile> applications; };

class ControlPlane {
public:
    virtual ~ControlPlane() = default;
    virtual Snapshot inspect() const = 0;
    virtual OperationPlan plan_profile(Profile profile) const = 0;
    virtual OperationState apply(const OperationPlan& plan) = 0;
    virtual OperationState verify(const OperationPlan& plan) const = 0;
};

const char* to_string(OperationState state) noexcept;
const char* to_string(Profile profile) noexcept;
const char* to_string(GraphicsBackend backend) noexcept;
std::string display_capability(cal::CapabilityState state);
bool is_editable(const Control& control) noexcept;
std::vector<GraphicsBackend> graphics_choices(const ApplicationProfile& application);
std::string serialize_application_profile(const ApplicationProfile& profile);

class MockControlPlane final : public ControlPlane {
public:
    Snapshot inspect() const override;
    OperationPlan plan_profile(Profile profile) const override;
    OperationState apply(const OperationPlan& plan) override;
    OperationState verify(const OperationPlan& plan) const override;
};

} // namespace isb::control_center
