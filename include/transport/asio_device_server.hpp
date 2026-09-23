#pragma once

#include <chrono>
#include <memory>

#include <boost/asio.hpp>

#include "domain/tags.hpp"

namespace transport {

// Single-client field-device simulator for local development and demos.
class AsioDeviceServer final : public std::enable_shared_from_this<AsioDeviceServer> {
public:
    AsioDeviceServer(boost::asio::io_context& io, unsigned short port,
                     std::chrono::milliseconds telemetry_period = std::chrono::milliseconds{100});
    void start();
    void stop();
    [[nodiscard]] unsigned short port() const;

private:
    void accept();
    void read_command();
    void schedule_telemetry();
    void write(std::string message);

    boost::asio::ip::tcp::acceptor acceptor_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::steady_timer timer_;
    boost::asio::streambuf read_buffer_;
    std::chrono::milliseconds telemetry_period_;
    domain::ProcessTags tags_;
    bool running_{false};
};

}  // namespace transport
