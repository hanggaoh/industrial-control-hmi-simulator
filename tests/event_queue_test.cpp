#include <cassert>
#include <iostream>
#include <thread>

#include "runtime/event_queue.hpp"

int main() {
    runtime::EventQueue<int> queue;
    std::thread producer([&] {
        queue.push(42);
        queue.close();
    });
    const auto event = queue.wait_pop();

    assert(event.has_value());
    assert(*event == 42);
    producer.join();
    std::cout << "Event queue tests passed\n";
}
