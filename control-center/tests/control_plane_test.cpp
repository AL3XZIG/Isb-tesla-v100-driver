#include "isb/control_center/control_plane.hpp"
#include <cassert>
#include <string>
using namespace isb::control_center;
int main() {
  Control unknown{"clock", "GPU Clock", isb::cal::CapabilityState::Unknown, {}, {}, true, {}};
  Control unsupported{"clock", "GPU Clock", isb::cal::CapabilityState::Unavailable, {}, {}, true, {}};
  Control available{"clock", "GPU Clock", isb::cal::CapabilityState::Available, {}, {}, true, {}};
  assert(!is_editable(unknown)); assert(!is_editable(unsupported)); assert(is_editable(available));
  ApplicationProfile blocked{"Game", "game.exe", "DX12", Profile::Gaming, isb::cal::CapabilityState::Unknown, {GraphicsBackend::OptiScaler}};
  assert(graphics_choices(blocked).empty()); blocked.compatibility = isb::cal::CapabilityState::Available;
  assert(graphics_choices(blocked).size() == 1); assert(serialize_application_profile(blocked) == "{\"api\":\"DX12\",\"compatible_backends\":[\"OptiScaler\"],\"compatibility\":\"available\",\"executable\":\"game.exe\",\"name\":\"Game\",\"profile\":\"Gaming\"}");
  MockControlPlane mock; const auto snapshot = mock.inspect(); assert(snapshot.mock); assert(snapshot.capabilities.hardware.rt_cores.state == isb::cal::CapabilityState::Unavailable); assert(snapshot.telemetry.front().value == std::nullopt);
  const auto plan = mock.plan_profile(Profile::Gaming); assert(plan.stages.size() == 7); assert(mock.apply(plan) == OperationState::Unsupported); assert(mock.verify(plan) == OperationState::Unknown);
}
