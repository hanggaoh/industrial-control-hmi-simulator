#pragma once

#include <algorithm>

namespace control {

struct PidConfig {
    double kp{1.0};
    double ki{0.0};
    double kd{0.0};
    double output_min{0.0};
    double output_max{100.0};
};

class PidController {
public:
    explicit PidController(PidConfig config) : config_(config) {}

    [[nodiscard]] double update(double setpoint, double process_value, double sample_seconds) {
        const double error = setpoint - process_value;
        const double derivative = initialized_ ? (error - previous_error_) / sample_seconds : 0.0;
        const double candidate_integral = integral_ + error * sample_seconds;

        const double unclamped = config_.kp * error + config_.ki * candidate_integral + config_.kd * derivative;
        const double output = std::clamp(unclamped, config_.output_min, config_.output_max);

        // Conditional integration prevents integral wind-up while saturated.
        if (output == unclamped || (output == config_.output_max && error < 0.0) ||
            (output == config_.output_min && error > 0.0)) {
            integral_ = candidate_integral;
        }

        previous_error_ = error;
        initialized_ = true;
        return output;
    }

    void reset() {
        integral_ = 0.0;
        previous_error_ = 0.0;
        initialized_ = false;
    }

private:
    PidConfig config_;
    double integral_{0.0};
    double previous_error_{0.0};
    bool initialized_{false};
};

}  // namespace control

