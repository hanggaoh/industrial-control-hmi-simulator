#pragma once

#include <algorithm>
#include <cmath>

#include "domain/energy.hpp"

namespace application {

class IDispatchStrategy {
public:
    virtual ~IDispatchStrategy() = default;
    [[nodiscard]] virtual domain::PowerCommand decide(const domain::EnergyTelemetry& telemetry) const = 0;
};

struct DispatchLimits {
    double reserve_soc_percent{15.0};
    double maximum_soc_percent{95.0};
};

// A deliberately explainable first dispatch policy. It is not an optimiser:
// surplus generation is charged, site load is discharged, and the SOC reserve
// remains protected. More advanced strategies can implement IDispatchStrategy
// without changing the transport or HMI layers.
class RuleBasedStorageDispatch final : public IDispatchStrategy {
public:
    explicit RuleBasedStorageDispatch(DispatchLimits limits = {}) : limits_(limits) {}

    [[nodiscard]] domain::PowerCommand decide(const domain::EnergyTelemetry& t) const override {
        if (t.net_load_kw < 0.0 && t.battery.soc_percent < limits_.maximum_soc_percent) {
            return {-std::min(-t.net_load_kw, t.battery.available_charge_kw), domain::PowerMode::charge};
        }
        if (t.net_load_kw > 0.0 && t.battery.soc_percent > limits_.reserve_soc_percent) {
            return {std::min(t.net_load_kw, t.battery.available_discharge_kw), domain::PowerMode::discharge};
        }
        return {0.0, domain::PowerMode::idle};
    }

private:
    DispatchLimits limits_;
};

struct SafetyLimits {
    double minimum_soc_percent{10.0};
    double maximum_soc_percent{98.0};
    double maximum_temperature_c{50.0};
};

class SafetyLimiter final {
public:
    explicit SafetyLimiter(SafetyLimits limits = {}) : limits_(limits) {}

    [[nodiscard]] domain::PowerCommand enforce(const domain::EnergyTelemetry& t,
                                                domain::PowerCommand requested) const {
        if (!t.inverter.connected || t.inverter.fault || t.battery.fault ||
            t.battery.temperature_c >= limits_.maximum_temperature_c) {
            return {0.0, domain::PowerMode::safe_stop};
        }

        if (requested.requested_power_kw > 0.0) {
            if (t.battery.soc_percent <= limits_.minimum_soc_percent) return {0.0, domain::PowerMode::idle};
            requested.requested_power_kw = std::min({requested.requested_power_kw,
                                                     t.battery.available_discharge_kw,
                                                     t.inverter.rated_power_kw});
            requested.mode = requested.requested_power_kw > 0.0 ? domain::PowerMode::discharge : domain::PowerMode::idle;
        } else if (requested.requested_power_kw < 0.0) {
            if (t.battery.soc_percent >= limits_.maximum_soc_percent) return {0.0, domain::PowerMode::idle};
            requested.requested_power_kw = -std::min({-requested.requested_power_kw,
                                                      t.battery.available_charge_kw,
                                                      t.inverter.rated_power_kw});
            requested.mode = requested.requested_power_kw < 0.0 ? domain::PowerMode::charge : domain::PowerMode::idle;
        }
        return requested;
    }

private:
    SafetyLimits limits_;
};

class StorageControlService final {
public:
    explicit StorageControlService(const IDispatchStrategy& strategy)
        : strategy_(strategy), limiter_{} {}

    StorageControlService(const IDispatchStrategy& strategy, SafetyLimiter limiter)
        : strategy_(strategy), limiter_(limiter) {}

    [[nodiscard]] domain::PowerCommand on_telemetry(const domain::EnergyTelemetry& telemetry) const {
        return limiter_.enforce(telemetry, strategy_.decide(telemetry));
    }

private:
    const IDispatchStrategy& strategy_;
    SafetyLimiter limiter_;
};

}  // namespace application
