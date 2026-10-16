#include <cassert>

#include "application/storage_dispatch.hpp"

namespace {

domain::EnergyTelemetry telemetry(double load_kw, double soc_percent = 55.0) {
    return {{}, load_kw, {soc_percent, 25.0, 30.0, 40.0, false}, {true, 0.0, 25.0, false}};
}

}  // namespace

int main() {
    application::RuleBasedStorageDispatch strategy;
    application::StorageControlService controller(strategy);

    auto command = controller.on_telemetry(telemetry(-40.0));
    assert(command.mode == domain::PowerMode::charge);
    assert(command.requested_power_kw == -25.0);  // inverter rating is the final limit

    command = controller.on_telemetry(telemetry(40.0));
    assert(command.mode == domain::PowerMode::discharge);
    assert(command.requested_power_kw == 25.0);

    command = controller.on_telemetry(telemetry(40.0, 10.0));
    assert(command.mode == domain::PowerMode::idle);
    assert(command.requested_power_kw == 0.0);

    auto failed = telemetry(40.0);
    failed.battery.fault = true;
    command = controller.on_telemetry(failed);
    assert(command.mode == domain::PowerMode::safe_stop);
    assert(command.requested_power_kw == 0.0);
}
