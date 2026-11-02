#include "provider_selection.hpp"

#include <cassert>

int main() {
    auto mock = isb::cli::make_provider(true);
    assert(mock);
    assert(mock->environment().mode == isb::hub::ProviderMode::Mock);
    assert(mock->environment().provenance.synthetic);

    auto runtime = isb::cli::make_provider(false);
    assert(runtime);
    const auto environment = runtime->environment();
    assert(environment.provenance.provider != "mock-provider");
    assert(!environment.provenance.synthetic);
    assert(environment.mode == isb::hub::ProviderMode::Real ||
           environment.mode == isb::hub::ProviderMode::Unavailable);
    return 0;
}
