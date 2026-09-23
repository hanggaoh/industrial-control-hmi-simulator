#include "hmi/dashboard_view_model.hpp"

#include <algorithm>

namespace hmi {

DashboardViewModel::DashboardViewModel(const std::size_t trend_capacity)
    : trend_capacity_(std::max<std::size_t>(trend_capacity, 1U)) {}

void DashboardViewModel::publish_tags(const domain::ProcessTags& tags) {
    std::scoped_lock lock(mutex_);
    current_tags_ = tags;
    trend_.push_back(tags);
    if (trend_.size() > trend_capacity_) {
        trend_.erase(trend_.begin(), trend_.begin() + static_cast<std::ptrdiff_t>(trend_.size() - trend_capacity_));
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
