#pragma once

#include "domain/tags.hpp"

namespace application {

// Owns control policy only. It neither performs network I/O nor renders UI.
class IControlService {
public:
    virtual ~IControlService() = default;
    virtual domain::ControlCommand on_sample(const domain::ProcessTags& tags) = 0;
    virtual void set_operator_command(const domain::ControlCommand& command) = 0;
};

}  // namespace application
