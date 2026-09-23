#pragma once

#include <cstddef>
#include <mutex>
#include <vector>

#include "hmi/hmi_view_model.hpp"

namespace hmi {

// Concrete HMI-facing store.  It deliberately owns display state only: a
// transport adapter may call publish_tags(), while a future application
// coordinator may consume the operator_command() returned by the UI.
class DashboardViewModel final : public IHmiViewModel {
public:
    explicit DashboardViewModel(std::size_t trend_capacity = 120);

    void publish_tags(const domain::ProcessTags& tags) override;
    [[nodiscard]] std::vector<domain::ProcessTags> recent_trend() const override;
    [[nodiscard]] domain::ConnectionStatus connection_status() const override;

    void set_connection_status(domain::ConnectionStatus status);
    void set_operator_command(domain::ControlCommand command);
    [[nodiscard]] domain::ControlCommand operator_command() const;
    [[nodiscard]] domain::ProcessTags current_tags() const;

private:
    mutable std::mutex mutex_;
    std::size_t trend_capacity_;
    std::vector<domain::ProcessTags> trend_;
    domain::ProcessTags current_tags_{};
    domain::ConnectionStatus connection_{false, "Waiting for device"};
    domain::ControlCommand operator_command_{};
};

}  // namespace hmi
