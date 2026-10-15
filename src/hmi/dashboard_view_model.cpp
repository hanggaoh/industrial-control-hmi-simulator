#include "hmi/dashboard_view_model.hpp"

#include <algorithm>

namespace hmi {

DashboardViewModel::DashboardViewModel(const std::size_t trend_capacity,
                                       const std::size_t alarm_history_capacity)
    : trend_capacity_(std::max<std::size_t>(trend_capacity, 1U)),
      alarm_history_capacity_(std::max<std::size_t>(alarm_history_capacity, 1U)) {}

void DashboardViewModel::publish_tags(const domain::ProcessTags& tags) {
    std::scoped_lock lock(mutex_);
    const auto previous_alarm = current_tags_.alarm;
    current_tags_ = tags;
    trend_.push_back(tags);
    if (trend_.size() > trend_capacity_) {
        trend_.erase(trend_.begin(), trend_.begin() + static_cast<std::ptrdiff_t>(trend_.size() - trend_capacity_));
    }

    if (tags.alarm == previous_alarm) {
        return;
    }

    // Close the currently active entry before recording a newly raised state.
    // This also handles a direct low-low to high-high transition correctly.
    for (auto it = alarm_history_.rbegin(); it != alarm_history_.rend(); ++it) {
        if (it->active) {
            it->active = false;
            break;
        }
    }
    if (tags.alarm != domain::AlarmSeverity::none) {
        alarm_history_.push_back({next_alarm_sequence_++, tags.alarm, true, false});
        if (alarm_history_.size() > alarm_history_capacity_) {
            alarm_history_.erase(alarm_history_.begin(),
                                 alarm_history_.begin() + static_cast<std::ptrdiff_t>(alarm_history_.size() - alarm_history_capacity_));
        }
    }
}

std::vector<domain::ProcessTags> DashboardViewModel::recent_trend() const {
    std::scoped_lock lock(mutex_);
    return trend_;
}

domain::ConnectionStatus DashboardViewModel::connection_status() const {
    std::scoped_lock lock(mutex_);
    return connection_;
}

std::vector<AlarmEvent> DashboardViewModel::alarm_history() const {
    std::scoped_lock lock(mutex_);
    return alarm_history_;
}

bool DashboardViewModel::acknowledge_active_alarm() {
    std::scoped_lock lock(mutex_);
    for (auto it = alarm_history_.rbegin(); it != alarm_history_.rend(); ++it) {
        if (it->active && !it->acknowledged) {
            it->acknowledged = true;
            return true;
        }
    }
    return false;
}

void DashboardViewModel::set_connection_status(domain::ConnectionStatus status) {
    std::scoped_lock lock(mutex_);
    connection_ = std::move(status);
}

void DashboardViewModel::set_operator_command(const domain::ControlCommand command) {
    std::scoped_lock lock(mutex_);
    operator_command_ = command;
}

domain::ControlCommand DashboardViewModel::operator_command() const {
    std::scoped_lock lock(mutex_);
    return operator_command_;
}

domain::ProcessTags DashboardViewModel::current_tags() const {
    std::scoped_lock lock(mutex_);
    return current_tags_;
}

}  // namespace hmi
