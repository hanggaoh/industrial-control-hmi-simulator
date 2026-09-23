#pragma once

#include <vector>

#include "domain/tags.hpp"

namespace hmi {

// Qt UI should bind to this view-model contract, not directly to the TCP
// device client or PID implementation.
class IHmiViewModel {
public:
    virtual ~IHmiViewModel() = default;
    virtual void publish_tags(const domain::ProcessTags& tags) = 0;
    virtual std::vector<domain::ProcessTags> recent_trend() const = 0;
    virtual domain::ConnectionStatus connection_status() const = 0;
};

}  // namespace hmi
