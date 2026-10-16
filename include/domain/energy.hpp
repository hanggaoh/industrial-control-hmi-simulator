#pragma once

#include <chrono>

namespace domain {

// Positive power supplies the local load (battery discharge); negative power
// absorbs excess generation (battery charge). Keeping the convention here
// avoids UI, transport and control layers each inventing their own sign.
enum class PowerMode { idle, charge, discharge, safe_stop };

struct BatteryTelemetry {
    double soc_percent{};
    double temperature_c{};
    double available_charge_kw{};
    double available_discharge_kw{};
    bool fault{};
};

struct InverterTelemetry {
    bool connected{};
    double actual_power_kw{};
    double rated_power_kw{};
    bool fault{};
};

struct EnergyTelemetry {
    std::chrono::steady_clock::time_point timestamp{};
    // Positive means the site is importing to serve load; negative represents
    // surplus generation available for charging.
    double net_load_kw{};
    BatteryTelemetry battery;
    InverterTelemetry inverter;
};

struct PowerCommand {
    double requested_power_kw{};
    PowerMode mode{PowerMode::idle};
};

}  // namespace domain
