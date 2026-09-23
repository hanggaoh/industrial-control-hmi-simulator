#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <utility>

namespace runtime {

// Small thread-safe hand-off point between a simulated field device and a
// controller/HMI consumer. A production SCADA system needs stronger delivery,
// ordering, and recovery guarantees; this intentionally demonstrates only the
// in-process concurrency boundary.
template <typename Event>
class EventQueue {
public:
    void push(Event event) {
        {
            std::lock_guard lock(mutex_);
            events_.push(std::move(event));
        }
        event_available_.notify_one();
    }

    void close() {
        {
            std::lock_guard lock(mutex_);
            closed_ = true;
        }
        event_available_.notify_all();
    }

    [[nodiscard]] std::optional<Event> wait_pop() {
        std::unique_lock lock(mutex_);
        event_available_.wait(lock, [&] { return !events_.empty() || closed_; });

        if (events_.empty()) {
            return std::nullopt;
        }

        Event event = std::move(events_.front());
        events_.pop();
        return event;
    }

private:
    std::mutex mutex_;
    std::condition_variable_any event_available_;
    std::queue<Event> events_;
    bool closed_{false};
};

}  // namespace runtime
