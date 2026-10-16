#include <iomanip>
#include <iostream>

#include "application/storage_dispatch.hpp"
#include "process/energy_storage_model.hpp"

namespace {

const char* mode_name(domain::PowerMode mode) {
    switch (mode) {
    case domain::PowerMode::charge: return "charge";
    case domain::PowerMode::discharge: return "discharge";
    case domain::PowerMode::safe_stop: return "safe_stop";
    case domain::PowerMode::idle: return "idle";
    }
    return "unknown";
}

}  // namespace

int main() {
    application::RuleBasedStorageDispatch strategy;
    application::StorageControlService controller(strategy);
    process::EnergyStorageModel plant;
    domain::PowerCommand command{};

    std::cout << "tick,net_load_kw,soc_percent,temperature_c,command_kw,mode\n";
    for (int tick = 0; tick < 18; ++tick) {
        const double net_load_kw = tick < 6 ? -28.0 : (tick < 13 ? 36.0 : 10.0);
        const auto telemetry = plant.step(net_load_kw, command, 1.0 / 60.0);
        command = controller.on_telemetry(telemetry);
        std::cout << tick << ',' << net_load_kw << ',' << std::fixed << std::setprecision(1)
                  << telemetry.battery.soc_percent << ',' << telemetry.battery.temperature_c << ','
                  << command.requested_power_kw << ',' << mode_name(command.mode) << '\n';
    }
}
