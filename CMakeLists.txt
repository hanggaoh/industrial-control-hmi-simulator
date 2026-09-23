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

# Optional desktop HMI.  Keeping this target separate means the simulator and
# tests remain buildable on machines that do not have Qt installed.
find_package(Qt6 COMPONENTS Widgets QUIET)
if(Qt6Widgets_FOUND)
  set(INDUSTRIAL_HMI_QT_TARGET Qt6::Widgets)
elseif(NOT Qt6Widgets_FOUND)
  find_package(Qt5 COMPONENTS Widgets QUIET)
  if(Qt5Widgets_FOUND)
    set(INDUSTRIAL_HMI_QT_TARGET Qt5::Widgets)
  endif()
endif()

if(INDUSTRIAL_HMI_QT_TARGET)
  add_executable(industrial_control_hmi
    src/qt_hmi_main.cpp
    src/hmi/dashboard_view_model.cpp
  )
  target_include_directories(industrial_control_hmi PRIVATE include)
  target_link_libraries(industrial_control_hmi PRIVATE ${INDUSTRIAL_HMI_QT_TARGET})
  message(STATUS "Qt Widgets found: industrial_control_hmi enabled")
else()
  message(STATUS "Qt Widgets not found: industrial_control_hmi is skipped")
endif()

# Optional: Boost.Asio is not bundled with this repository. When Boost headers
# are installed, CMake adds a small asynchronous timer/event-loop demonstration.
find_package(Boost CONFIG QUIET)
if(Boost_FOUND)
  find_package(Threads REQUIRED)
  add_library(transport_tcp
    src/transport/asio_device_client.cpp
    src/transport/asio_device_server.cpp)
  target_include_directories(transport_tcp PUBLIC include)
  target_link_libraries(transport_tcp PUBLIC Boost::headers Threads::Threads)
  target_compile_definitions(transport_tcp PUBLIC BOOST_ERROR_CODE_HEADER_ONLY)

  add_executable(tcp_device_demo src/tcp_device_demo.cpp)
  target_link_libraries(tcp_device_demo PRIVATE transport_tcp)

  add_executable(wire_protocol_test tests/transport/wire_protocol_test.cpp)
  target_include_directories(wire_protocol_test PRIVATE include)
  add_test(NAME wire_protocol_test COMMAND wire_protocol_test)

  add_executable(boost_asio_telemetry_demo src/boost_telemetry_demo.cpp)
  target_include_directories(boost_asio_telemetry_demo PRIVATE include)
  target_link_libraries(boost_asio_telemetry_demo PRIVATE Boost::headers)
  target_compile_definitions(boost_asio_telemetry_demo PRIVATE BOOST_ERROR_CODE_HEADER_ONLY)
  message(STATUS "Boost found: boost_asio_telemetry_demo enabled")
else()
  message(STATUS "Boost not found: build remains fully functional; boost_asio_telemetry_demo is skipped")
endif()
