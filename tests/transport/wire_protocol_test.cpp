#include <cassert>
#include <cmath>

#include "transport/wire_protocol.hpp"

int main() {
    domain::ProcessTags expected{};
    expected.level_percent = 42.5; expected.setpoint_percent = 55.0; expected.inlet_command_percent = 13.0;
    expected.quality = domain::Quality::stale; expected.alarm = domain::AlarmSeverity::high_high;
    const auto parsed = transport::wire::parse_telemetry(transport::wire::telemetry(expected));
    assert(parsed); assert(std::abs(parsed->level_percent - 42.5) < 0.001);
    assert(parsed->quality == domain::Quality::stale); assert(parsed->alarm == domain::AlarmSeverity::high_high);
    assert(!transport::wire::parse_telemetry("TEL bad"));
    const auto command = transport::wire::parse_command("CMD 67.5 0\n");
    assert(command && std::abs(command->setpoint_percent - 67.5) < 0.001 && !command->automatic_mode);
    assert(!transport::wire::parse_command("CMD 50 2"));
}
