#pragma once

#include <chrono>
#include <deque>
#include <memory>
#include <string>

#include <boost/asio.hpp>

#include "transport/device_protocol.hpp"

namespace transport {

// Local TCP implementation of IDeviceClient. It reconnects with a bounded
// delay and exposes protocol acknowledgements as connection-status detail.
class AsioDeviceClient final : public IDeviceClient, public std::enable_shared_from_this<AsioDeviceClient> {
public:
    AsioDeviceClient(boost::asio::io_context& io, std::string host, unsigned short port,
                     std::chrono::milliseconds reconnect_delay = std::chrono::milliseconds{250});

    void start(TagsHandler on_tags, StatusHandler on_status) override;
    void stop() override;
    void send_command(const domain::ControlCommand& command) override;

private:
    void connect();
    void schedule_reconnect(std::string detail);
    void read_line();
    void write_next();
    void notify(bool connected, std::string detail);

    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::steady_timer reconnect_timer_;
    boost::asio::streambuf read_buffer_;
    std::deque<std::string> writes_;
    std::string host_;
    std::string service_;
    std::chrono::milliseconds reconnect_delay_;
    TagsHandler on_tags_;
    StatusHandler on_status_;
    bool running_{false};
    bool writing_{false};
};

}  // namespace transport
