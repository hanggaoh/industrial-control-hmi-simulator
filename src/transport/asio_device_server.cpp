#include "transport/asio_device_server.hpp"

#include <istream>

#include "transport/wire_protocol.hpp"

namespace transport {
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

AsioDeviceServer::AsioDeviceServer(asio::io_context& io, unsigned short port, std::chrono::milliseconds telemetry_period)
    : acceptor_(io, tcp::endpoint(tcp::v4(), port)), socket_(io), timer_(io), telemetry_period_(telemetry_period) {
    tags_.level_percent = 42.0;
    tags_.setpoint_percent = 50.0;
    tags_.inlet_command_percent = 30.0;
}

void AsioDeviceServer::start() { running_ = true; accept(); }
void AsioDeviceServer::stop() {
    running_ = false;
    boost::system::error_code ignored;
    timer_.cancel(); socket_.close(ignored); acceptor_.close(ignored);
}
unsigned short AsioDeviceServer::port() const { return acceptor_.local_endpoint().port(); }

void AsioDeviceServer::accept() {
    if (!running_) return;
    auto self = shared_from_this();
    acceptor_.async_accept(socket_, [self](const boost::system::error_code& error) {
        if (error || !self->running_) return;
        self->read_command(); self->schedule_telemetry();
    });
}

void AsioDeviceServer::read_command() {
    auto self = shared_from_this();
    asio::async_read_until(socket_, read_buffer_, '\n', [self](const boost::system::error_code& error, std::size_t) {
        if (error) { if (self->running_) self->accept(); return; }
        std::istream input(&self->read_buffer_); std::string line; std::getline(input, line);
        if (auto command = wire::parse_command(line)) {
            self->tags_.setpoint_percent = command->setpoint_percent;
            self->write(wire::acknowledgement("command accepted"));
        }
        self->read_command();
    });
}

void AsioDeviceServer::schedule_telemetry() {
    if (!running_ || !socket_.is_open()) return;
    auto self = shared_from_this();
    timer_.expires_after(telemetry_period_);
    timer_.async_wait([self](const boost::system::error_code& error) {
        if (error || !self->running_ || !self->socket_.is_open()) return;
        self->tags_.timestamp = std::chrono::steady_clock::now();
        self->tags_.level_percent += (self->tags_.setpoint_percent - self->tags_.level_percent) * 0.12;
        self->tags_.inlet_command_percent = self->tags_.setpoint_percent;
        self->write(wire::telemetry(self->tags_)); self->schedule_telemetry();
    });
}

void AsioDeviceServer::write(std::string message) {
    auto payload = std::make_shared<std::string>(std::move(message));
    asio::async_write(socket_, asio::buffer(*payload), [payload](const boost::system::error_code&, std::size_t) {});
}
}  // namespace transport
