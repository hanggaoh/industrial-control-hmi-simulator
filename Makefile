CXX ?= c++
CXXFLAGS := -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
BUILD_DIR := build

.PHONY: all run test cmake-test clean

all: $(BUILD_DIR)/simulator

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/simulator: src/main.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/pid_controller_test: tests/pid_controller_test.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILD_DIR)/event_queue_test: tests/event_queue_test.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

run: $(BUILD_DIR)/simulator
	./$(BUILD_DIR)/simulator

test: $(BUILD_DIR)/pid_controller_test $(BUILD_DIR)/event_queue_test
	./$(BUILD_DIR)/pid_controller_test
	./$(BUILD_DIR)/event_queue_test

cmake-test:
	cmake -S . -B $(BUILD_DIR)/cmake
	cmake --build $(BUILD_DIR)/cmake
	ctest --test-dir $(BUILD_DIR)/cmake --output-on-failure

clean:
	rm -rf $(BUILD_DIR)
