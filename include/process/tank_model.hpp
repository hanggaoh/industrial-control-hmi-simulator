#pragma once

#include <algorithm>

namespace process {

class TankModel {
public:
    explicit TankModel(double initial_level_percent = 20.0) : level_percent_(initial_level_percent) {}

    void step(double inlet_percent, double outlet_disturbance_percent, double sample_seconds) {
        // A deliberately simple first-order process for a learning simulator.
        const double net_flow = inlet_percent - outlet_disturbance_percent;
        level_percent_ = std::clamp(level_percent_ + net_flow * gain_per_second_ * sample_seconds, 0.0, 100.0);
    }

    [[nodiscard]] double level_percent() const { return level_percent_; }

private:
    double level_percent_;
    double gain_per_second_{0.18};
};

}  // namespace process

