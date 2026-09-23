#include <chrono>
#include <iostream>

#include <boost/asio.hpp>

// A deliberately local-only example of the event-loop style used when a
// controller communicates asynchronously with a device. It does not claim an
// industrial protocol implementation: production Modbus/DNP3/IEC 61850
// implementations have framing, retries, security, and protocol semantics.
int main() {
    namespace asio = boost::asio;
    using namespace std::chrono_literals;

    asio::io_context io;
    asio::steady_timer timer(io);
    int sample_count = 0;

    std::function<void()> schedule_sample;
    schedule_sample = [&] {
        timer.expires_after(250ms);
        timer.async_wait([&](const boost::system::error_code& error) {
            if (error) {
                return;
            }
            std::cout << "async telemetry sample " << ++sample_count << '\n';
            if (sample_count < 4) {
                schedule_sample();
            }
        });
    };

    schedule_sample();
    io.run();
}
