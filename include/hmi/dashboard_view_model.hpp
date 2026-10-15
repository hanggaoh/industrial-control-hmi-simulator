#pragma once

#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

#include "hmi/hmi_view_model.hpp"

namespace hmi {

// This is intentionally a small HMI event record rather than an alarm-system
// implementation.  It gives the view an auditable display contract: an alarm
// can be raised, returned-to-normal, and acknowledged without teaching the UI
// about the process model or transport.
struct AlarmEvent {
    std::uint64_t sequence{};
    domain::AlarmSeverity severity{domain::AlarmSeverity::none};
    bool active{};
    bool acknowledged{};
};

// Concrete HMI-facing store.  It deliberately owns display state only: a
// transport adapter may call publish_tags(), while a future application
// coordinator may consume the operator_command() returned by the UI.
class DashboardViewModel final : public IHmiViewModel {
public:
    explicit DashboardViewModel(std::size_t trend_capacity = 120,
                                std::size_t alarm_history_capacity = 80);

    void publish_tags(const domain::ProcessTags& tags) override;
    [[nodiscard]] std::vector<domain::ProcessTags> recent_trend() const override;
    [[nodiscard]] domain::ConnectionStatus connection_status() const override;
    [[nodiscard]] std::vector<AlarmEvent> alarm_history() const;
    // Acknowledgement does not clear an active process alarm.  The current
    // condition must still return to normal through publish_tags().
    [[nodiscard]] bool acknowledge_active_alarm();

    void set_connection_status(domain::ConnectionStatus status);
    void set_operator_command(domain::ControlCommand command);
    [[nodiscard]] domain::ControlCommand operator_command() const;
    [[nodiscard]] domain::ProcessTags current_tags() const;

private:
    mutable std::mutex mutex_;
    std::size_t trend_capacity_;
    std::size_t alarm_history_capacity_;
    std::vector<domain::ProcessTags> trend_;
    std::vector<AlarmEvent> alarm_history_;
    std::uint64_t next_alarm_sequence_{1};
    domain::ProcessTags current_tags_{};
    domain::ConnectionStatus connection_{false, "Waiting for device"};
    domain::ControlCommand operator_command_{};
};

}  // namespace hmi
