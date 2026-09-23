cmake_minimum_required(VERSION 3.20)
project(industrial_control_hmi_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(control_simulator src/main.cpp)
target_include_directories(control_simulator PRIVATE include)

add_executable(pid_controller_test tests/pid_controller_test.cpp)
target_include_directories(pid_controller_test PRIVATE include)

add_executable(event_queue_test tests/event_queue_test.cpp)
target_include_directories(event_queue_test PRIVATE include)

include(CTest)
if(BUILD_TESTING)
  add_test(NAME pid_controller_test COMMAND pid_controller_test)
  add_test(NAME event_queue_test COMMAND event_queue_test)
endif()

# Optional: Boost.Asio is not bundled with this repository. When Boost headers
# are installed, CMake adds a small asynchronous timer/event-loop demonstration.
find_package(Boost CONFIG QUIET)
if(Boost_FOUND)
  add_executable(boost_asio_telemetry_demo src/boost_telemetry_demo.cpp)
  target_include_directories(boost_asio_telemetry_demo PRIVATE include)
  target_link_libraries(boost_asio_telemetry_demo PRIVATE Boost::headers)
  target_compile_definitions(boost_asio_telemetry_demo PRIVATE BOOST_ERROR_CODE_HEADER_ONLY)
  message(STATUS "Boost found: boost_asio_telemetry_demo enabled")
else()
  message(STATUS "Boost not found: build remains fully functional; boost_asio_telemetry_demo is skipped")
endif()
