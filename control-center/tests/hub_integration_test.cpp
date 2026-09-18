#include "isb/hub/hub.hpp"

#include <cassert>

int main() {
    using namespace isb::hub;

    MockProvider provider;
    Hub hub(provider);

    const auto env = hub.environment();
    assert(env.mode == ProviderMode::Mock);
    assert(env.provenance.synthetic);

    const auto capabilities = hub.capabilities();
    assert(capabilities.identity.gpu_variant == cal::GpuVariant::V100_SXM2);
    assert(capabilities.hardware.tensor_cores.state == cal::CapabilityState::Available);

    const auto telemetry = hub.telemetry();
    assert(telemetry.synthetic);
    assert(telemetry.temperature_c == 42);

    const auto plan = hub.profile_plan("Gaming");
    assert(plan.dry_run);
    assert(plan.operations.empty());
    assert(!plan.unknown.empty());

    const auto rejected = hub.apply(plan, false);
    assert(rejected.state == OperationState::Planned);
    assert(!rejected.mutated);

    const auto unsupported = hub.apply(plan, true);
    assert(unsupported.state == OperationState::Failed);
    assert(!unsupported.mutated);

    return 0;
}
