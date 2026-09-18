#include "isb/hub/runtime_provider.hpp"

#include <cassert>

int main() {
    auto provider = isb::hub::make_runtime_provider();
    assert(provider);

    const auto environment = provider->environment();
    assert(!environment.os.empty());

    const auto telemetry = provider->telemetry();
    assert(!telemetry.provenance.provider.empty());

    const auto capabilities = provider->capabilities();
    (void)capabilities;

    const auto controls = provider->controls();
    assert(!controls.empty());

    return 0;
}
