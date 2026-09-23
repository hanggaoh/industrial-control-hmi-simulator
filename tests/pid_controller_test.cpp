#include <cassert>
#include <cmath>
#include <iostream>

#include "control/pid_controller.hpp"

int main() {
    control::PidController proportional({.kp = 2.0, .ki = 0.0, .kd = 0.0, .output_min = 0.0, .output_max = 100.0});
    assert(std::abs(proportional.update(60.0, 40.0, 1.0) - 40.0) < 1e-9);

    control::PidController limited({.kp = 10.0, .ki = 0.0, .kd = 0.0, .output_min = 0.0, .output_max = 50.0});
    assert(std::abs(limited.update(100.0, 0.0, 1.0) - 50.0) < 1e-9);
    assert(std::abs(limited.update(0.0, 100.0, 1.0)) < 1e-9);

    control::PidController integral({.kp = 0.0, .ki = 1.0, .kd = 0.0, .output_min = 0.0, .output_max = 100.0});
    assert(std::abs(integral.update(10.0, 0.0, 0.5) - 5.0) < 1e-9);
    assert(std::abs(integral.update(10.0, 0.0, 0.5) - 10.0) < 1e-9);

    std::cout << "PID controller tests passed\n";
}

