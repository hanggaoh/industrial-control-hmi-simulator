#pragma once

#include <charconv>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>

#include "domain/tags.hpp"

namespace transport::wire {

// Intentionally small, line-oriented protocol for the local simulator only.
// It is not Modbus, DNP3, or IEC 61850 and must not be represented as one.
inline std::string telemetry(const domain::ProcessTags& tags) {
    std::ostringstream message;
    message << "TEL " << tags.level_percent << ' ' << tags.setpoint_percent << ' '
            << tags.inlet_command_percent << ' ' << static_cast<int>(tags.quality) << ' '
            << static_cast<int>(tags.alarm) << "\n";
    return message.str();
}

inline std::string command(const domain::ControlCommand& value) {
    std::ostringstream message;
    message << "CMD " << value.setpoint_percent << ' ' << (value.automatic_mode ? 1 : 0) << "\n";
    return message.str();
}

inline std::string acknowledgement(std::string_view detail = "accepted") {
    return "ACK " + std::string(detail) + "\n";
}

inline std::optional<domain::ProcessTags> parse_telemetry(std::string_view line) {
    std::istringstream input{std::string(line)};
    std::string kind;
    int quality{};
    int alarm{};
    domain::ProcessTags tags;
    if (!(input >> kind >> tags.level_percent >> tags.setpoint_percent >> tags.inlet_command_percent >> quality >> alarm) ||
        kind != "TEL" || quality < 0 || quality > 2 || alarm < 0 || alarm > 3) {
        return std::nullopt;
    }
    tags.timestamp = std::chrono::steady_clock::now();
    tags.quality = static_cast<domain::Quality>(quality);
    tags.alarm = static_cast<domain::AlarmSeverity>(alarm);
    return tags;
}

inline std::optional<domain::ControlCommand> parse_command(std::string_view line) {
    std::istringstream input{std::string(line)};
    std::string kind;
    int automatic{};
    domain::ControlCommand command;
    if (!(input >> kind >> command.setpoint_percent >> automatic) || kind != "CMD" ||
        (automatic != 0 && automatic != 1)) {
        return std::nullopt;
    }
    command.automatic_mode = automatic == 1;
    return command;
}

}  // namespace transport::wire
