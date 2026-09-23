#pragma once

namespace runtime {

struct ProcessSample {
    double elapsed_seconds{};
    double level_percent{};
    double outlet_disturbance_percent{};
    bool terminal{};
};

}  // namespace runtime
