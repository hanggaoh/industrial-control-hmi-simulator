#include <cassert>
#include <chrono>

#include "hmi/dashboard_view_model.hpp"

namespace {

domain::ProcessTags sample(domain::AlarmSeverity alarm) {
    return {std::chrono::steady_clock::now(), 50.0, 65.0, 40.0,
            domain::Quality::good, alarm};
}

}  // namespace

int main() {
    hmi::DashboardViewModel dashboard;
    dashboard.publish_tags(sample(domain::AlarmSeverity::none));
    assert(dashboard.alarm_history().empty());

    dashboard.publish_tags(sample(domain::AlarmSeverity::low_low));
    auto events = dashboard.alarm_history();
    assert(events.size() == 1U);
    assert(events.front().active);
    assert(!events.front().acknowledged);
    assert(dashboard.acknowledge_active_alarm());

    events = dashboard.alarm_history();
    assert(events.front().acknowledged);
    dashboard.publish_tags(sample(domain::AlarmSeverity::none));
    events = dashboard.alarm_history();
    assert(!events.front().active);
    assert(!dashboard.acknowledge_active_alarm());
}
