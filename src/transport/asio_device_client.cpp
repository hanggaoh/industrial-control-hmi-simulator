#include "transport/asio_device_client.hpp"

#include <istream>

#include "transport/wire_protocol.hpp"

namespace transport {
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

AsioDeviceClient::AsioDeviceClient(asio::io_context& io, std::string host, unsigned short port,
                                   std::chrono::milliseconds reconnect_delay)
    : resolver_(io), socket_(io), reconnect_timer_(io), host_(std::move(host)),
      service_(std::to_string(port)), reconnect_delay_(reconnect_delay) {}

void AsioDeviceClient::start(TagsHandler on_tags, StatusHandler on_status) {
    on_tags_ = std::move(on_tags);
    on_status_ = std::move(on_status);
    running_ = true;
    connect();
}

void AsioDeviceClient::stop() {
    running_ = false;
    boost::system::error_code ignored;
    reconnect_timer_.cancel();
    resolver_.cancel();
    socket_.shutdown(tcp::socket::shutdown_both, ignored);
    socket_.close(ignored);
    notify(false, "stopped");
}

void AsioDeviceClient::send_command(const domain::ControlCommand& command) {
    auto self = shared_from_this();
    asio::post(socket_.get_executor(), [self, message = wire::command(command)] {
        self->writes_.push_back(message);
        if (self->socket_.is_open() && !self->writing_) self->write_next();
    });
}

void AsioDeviceClient::connect() {
    if (!running_) return;
    auto self = shared_from_this();
    resolver_.async_resolve(host_, service_, [self](const boost::system::error_code& error, tcp::resolver::results_type results) {
        if (error) return self->schedule_reconnect("resolve failed: " + error.message());
        asio::async_connect(self->socket_, results, [self](const boost::system::error_code& connect_error, const tcp::endpoint&) {
            if (connect_error) return self->schedule_reconnect("connect failed: " + connect_error.message());
            self->notify(true, "connected to local device simulator");
            self->read_line();
            if (!self->writes_.empty()) self->write_next();
        });
    });
}

void AsioDeviceClient::schedule_reconnect(std::string detail) {
    boost::system::error_code ignored;
    socket_.close(ignored);
    writing_ = false;
    if (!running_) return;
    notify(false, std::move(detail));
    auto self = shared_from_this();
    reconnect_timer_.expires_after(reconnect_delay_);
    reconnect_timer_.async_wait([self](const boost::system::error_code& error) { if (!error) self->connect(); });
}

void AsioDeviceClient::read_line() {
    auto self = shared_from_this();
    asio::async_read_until(socket_, read_buffer_, '\n', [self](const boost::system::error_code& error, std::size_t) {
        if (error) return self->schedule_reconnect("read failed: " + error.message());
        std::istream input(&self->read_buffer_);
        std::string line;
        std::getline(input, line);
        if (auto tags = wire::parse_telemetry(line); tags && self->on_tags_) self->on_tags_(*tags);
        else if (line.rfind("ACK ", 0) == 0) self->notify(true, line.substr(4));
        self->read_line();
    });
}

void AsioDeviceClient::write_next() {
    if (writes_.empty() || !socket_.is_open()) return;
    writing_ = true;
    auto self = shared_from_this();
    asio::async_write(socket_, asio::buffer(writes_.front()), [self](const boost::system::error_code& error, std::size_t) {
        self->writing_ = false;
        if (error) return self->schedule_reconnect("write failed: " + error.message());
        self->writes_.pop_front();
        if (!self->writes_.empty()) self->write_next();
    });
}

void AsioDeviceClient::notify(bool connected, std::string detail) {
    if (on_status_) on_status_(domain::ConnectionStatus{connected, std::move(detail)});
}
}  // namespace transport
