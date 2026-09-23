# Industrial Control & HMI Simulator

A learning project that models a small industrial water-tank process and the
building blocks around it: discrete-time control, process I/O, alarms, trends,
and an eventual HMI. It is deliberately a simulator, not a safety-certified or
production SCADA product.

## Why this project

The project is a focused refresher of control-systems and C++ skills:

- discrete sampling and a PID control loop;
- process variables, manipulated variables, disturbances, and limits;
- alarm conditions and event-oriented observability;
- a path to a Qt HMI with trend charts and alarm/status views;
- a testable C++20 implementation rather than simulation-only control logic;
- a thread-safe field-device-to-controller event path, plus an optional
  Boost.Asio event-loop demo when Boost is installed.

## Current scope

The executable simulates a single water tank. A field-device task owns the
process model, samples it every 0.5 seconds, and sends samples through a
mutex-protected queue. A separate controller execution context consumes each
sample, runs a PID controller, and updates the simulated inlet command. A
configurable outlet flow acts as a disturbance. The console output is shaped
like a basic trend log: time, process variable, setpoint, controller output,
and alarm state.

## Build and run

Requires a C++20 compiler (Apple Clang 17 or GCC/Clang equivalent).

```sh
make run
make test
make cmake-test
```

The `make` workflow needs no third-party dependency. The CMake workflow detects
Boost optionally. If installed, it adds `boost_asio_telemetry_demo`, a small
`boost::asio::io_context` / `steady_timer` event-loop example:

```sh
cmake -S . -B build/cmake
cmake --build build/cmake
./build/cmake/boost_asio_telemetry_demo
```

If Boost is not found, the control simulator and both tests still build; CMake
prints that the optional target was skipped. The demo is intentionally local
and timer-based, not a claim of Modbus, DNP3, OPC UA, or IEC 61850 support.

## Implemented evidence map

| Implemented in this repository | Industrial/SCADA-HMI concept it demonstrates |
| --- | --- |
| PID controller, discrete 0.5 s sampling, outlet disturbance | Closed-loop process control and process variables |
| `ProcessSample` events and a thread-safe `EventQueue` | Device-to-controller event hand-off and concurrency boundary |
| Atomic actuator command | Simulated output/tag write without a shared mutable control object |
| CSV-like console trend and high/high / low/low conditions | Trend and alarm concepts for a future HMI |
| Unit tests for PID behaviour and queue delivery | Testable control and concurrency primitives |
| Optional Boost.Asio timer event loop | Asynchronous I/O/event-loop programming pattern |

This is an in-process educational simulator. It does not implement field-device
protocols, a networked HMI, durable historian storage, or safety functions.

## Roadmap

1. **Control core** - PID, sampling, water-tank model, bounds, and unit tests.
2. **DCS concepts** - named I/O tags, alarm limits, acknowledgement state, and
   CSV trend export.
3. **C++ communication** - a TCP device simulator with a documented wire
   format, retries, and connection-state alarms (then a protocol adapter only
   when it is actually implemented).
4. **Qt HMI** - a process overview, trend chart, setpoint entry, manual/auto
   mode, and alarm list.
5. **Portfolio polish** - screenshots, architecture diagram, tests, and a
   short engineering write-up.

## Honest portfolio framing

This repository demonstrates industrial-control and SCADA/HMI concepts through
a learning simulator. It does not claim OPC UA/Modbus interoperability,
functional-safety certification, or commercial DCS deployment.
