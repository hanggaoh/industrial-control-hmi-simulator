#include <iomanip>
#include <iostream>
#include <atomic>
#include <thread>

#include "control/pid_controller.hpp"
#include "process/tank_model.hpp"
#include "runtime/event_queue.hpp"
#include "runtime/process_event.hpp"

int main() {
    constexpr double sample_seconds = 0.5;
    constexpr double setpoint_percent = 65.0;

    control::PidController controller({.kp = 1.6, .ki = 0.12, .kd = 0.0, .output_min = 0.0, .output_max = 100.0});
    runtime::EventQueue<runtime::ProcessSample> events;
    std::atomic<double> inlet_command{0.0};

    std::cout << "time_s,level_percent,setpoint_percent,inlet_percent,alarm\n";

    // Field-device task: owns the process model and publishes samples through
    // a mutex-protected event queue. The controller below consumes them on a
    // different execution context and writes a new actuator command atomically.
    std::thread device([&] {
        process::TankModel tank(20.0);
        for (int tick = 0; tick < 120; ++tick) {
            const double elapsed_seconds = tick * sample_seconds;
            const double outlet_disturbance = elapsed_seconds >= 30.0 ? 38.0 : 20.0;
            tank.step(inlet_command.load(), outlet_disturbance, sample_seconds);
            events.push({.elapsed_seconds = elapsed_seconds,
                         .level_percent = tank.level_percent(),
                         .outlet_disturbance_percent = outlet_disturbance});
        }
        events.close();
    });

    while (const auto event = events.wait_pop()) {
        if (event->terminal) {
            break;
        }

        const double inlet = controller.update(setpoint_percent, event->level_percent, sample_seconds);
        inlet_command.store(inlet);

        const char* alarm = event->level_percent < 10.0 ? "LOW_LOW" :
                            event->level_percent > 90.0 ? "HIGH_HIGH" : "NORMAL";
        std::cout << std::fixed << std::setprecision(1) << event->elapsed_seconds << ',' << event->level_percent << ','
                  << setpoint_percent << ',' << inlet << ',' << alarm << '\n';
    }
    device.join();
}
