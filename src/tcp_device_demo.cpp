#include <chrono>
#include <iostream>

#include <boost/asio.hpp>

#include "transport/asio_device_client.hpp"
#include "transport/asio_device_server.hpp"

int main() {
    namespace asio = boost::asio;
    using namespace std::chrono_literals;
    asio::io_context io;
    auto server = std::make_shared<transport::AsioDeviceServer>(io, 0, 80ms);
    server->start();
    auto client = std::make_shared<transport::AsioDeviceClient>(io, "127.0.0.1", server->port());
    int samples = 0;
    client->start([&](const domain::ProcessTags& tags) {
        std::cout << "telemetry level=" << tags.level_percent << " setpoint=" << tags.setpoint_percent << '\n';
        if (++samples == 2) client->send_command({65.0, true});
        if (samples == 5) { client->stop(); server->stop(); }
    }, [](const domain::ConnectionStatus& status) { std::cout << "connection " << status.connected << ": " << status.detail << '\n'; });
    io.run();
}
