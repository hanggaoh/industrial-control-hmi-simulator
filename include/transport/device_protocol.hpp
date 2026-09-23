#pragma once

#include <functional>

#include "domain/tags.hpp"

namespace transport {

// Implementations may use an in-process simulator, Boost.Asio TCP, or a
// future industrial protocol adapter. The rest of the application only sees
// these callbacks and never depends on a wire format.
class IDeviceClient {
public:
    using TagsHandler = std::function<void(const domain::ProcessTags&)>;
    using StatusHandler = std::function<void(const domain::ConnectionStatus&)>;

    virtual ~IDeviceClient() = default;
    virtual void start(TagsHandler on_tags, StatusHandler on_status) = 0;
    virtual void stop() = 0;
    virtual void send_command(const domain::ControlCommand& command) = 0;
};

}  // namespace transport
