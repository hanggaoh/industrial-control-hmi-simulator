#pragma once

#include <chrono>
#include <string>

namespace domain {

// The stable, protocol-independent data contract between a field device,
// control logic, historian/HMI consumers, and any later protocol adapter.
enum class Quality { good, stale, bad };
enum class AlarmSeverity { none, low_low, high_high, communication };

struct ProcessTags {
    std::chrono::steady_clock::time_point timestamp{};
    double level_percent{};
    double setpoint_percent{};
    double inlet_command_percent{};
    Quality quality{Quality::good};
    AlarmSeverity alarm{AlarmSeverity::none};
};

struct ControlCommand {
    double setpoint_percent{};
    bool automatic_mode{true};
};

struct ConnectionStatus {
    bool connected{};
    std::string detail;
};

}  // namespace domain
