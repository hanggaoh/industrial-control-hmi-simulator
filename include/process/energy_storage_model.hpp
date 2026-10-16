#pragma once

#include <algorithm>

#include "domain/energy.hpp"

namespace process {

// Local, intentionally simplified battery/inverter model. It is useful for
// exercising the full control loop, not a physical battery model.
class EnergyStorageModel final {
public:
    explicit EnergyStorageModel(double initial_soc_percent = 55.0) : soc_percent_(initial_soc_percent) {}

    [[nodiscard]] domain::EnergyTelemetry step(double net_load_kw,
                                               const domain::PowerCommand& command,
                                               double sample_hours) {
        const double limited = std::clamp(command.requested_power_kw, -rated_power_kw_, rated_power_kw_);
        actual_power_kw_ += (limited - actual_power_kw_) * 0.65;
        // A 100 kWh nominal pack: positive output reduces state of charge.
        soc_percent_ = std::clamp(soc_percent_ - actual_power_kw_ * sample_hours, 0.0, 100.0);
        temperature_c_ = std::max(22.0, temperature_c_ + std::abs(actual_power_kw_) * 0.012 - 0.08);
        return {{}, net_load_kw,
                {soc_percent_, temperature_c_, 45.0, 45.0, false},
                {true, actual_power_kw_, rated_power_kw_, false}};
    }

private:
    double soc_percent_;
    double temperature_c_{25.0};
    double actual_power_kw_{};
    double rated_power_kw_{50.0};
};

}  // namespace process
