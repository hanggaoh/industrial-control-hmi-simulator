# Industrial Control & HMI Simulator

A learning project that models a small industrial water-tank process and the
building blocks around it: discrete-time control, process I/O, alarms, trends,
and a Qt operator HMI. It is deliberately a simulator, not a safety-certified or
production SCADA product.

## Why this project

The project is a focused refresher of control-systems and C++ skills:

- discrete sampling and a PID control loop;
- process variables, manipulated variables, disturbances, and limits;
- alarm conditions and event-oriented observability;
- a runnable Qt HMI with trend charts, alarm/status views, and operator controls;
- a testable C++20 implementation rather than simulation-only control logic;
- a thread-safe field-device-to-controller event path and a local Boost.Asio
  TCP device simulator/client when Boost is installed.

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
Boost optionally. If installed, it adds a local asynchronous TCP device
simulator/client with telemetry, command acknowledgement, and connection-state
handling:

```sh
cmake -S . -B build/cmake
cmake --build build/cmake
./build/cmake/tcp_device_demo
```

If Boost is not found, the control simulator and both tests still build; CMake
prints that the optional transport targets were skipped. The newline-delimited
wire format is intentionally local, not a claim of Modbus, DNP3, OPC UA, or
IEC 61850 support.

With Qt 6 installed, CMake also builds the live operator dashboard:

```sh
cmake -S . -B build/qt -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build/qt
./build/qt/industrial_control_hmi
```

The HMI presents live level, setpoint, inlet-command, alarm, and connection
tags, a rendered trend, and Auto/Manual setpoint controls. Its dashboard
view-model is separate from the control and transport interfaces.

## Implemented evidence map

| Implemented in this repository | Industrial/SCADA-HMI concept it demonstrates |
| --- | --- |
| PID controller, discrete 0.5 s sampling, outlet disturbance | Closed-loop process control and process variables |
| `ProcessSample` events and a thread-safe `EventQueue` | Device-to-controller event hand-off and concurrency boundary |
| Atomic actuator command | Simulated output/tag write without a shared mutable control object |
| CSV-like console trend and high/high / low/low conditions | Trend and alarm concepts |
| Qt 6 dashboard: tags, trend, alarms, connection state, Auto/Manual controls | Operator-facing HMI and view-model boundary |
| Local Boost.Asio TCP server/client | Asynchronous telemetry, command acknowledgement, and connection-state pattern |
| Unit tests for PID behaviour, queue delivery, and wire encoding | Testable control, concurrency, and transport primitives |

The control core is an in-process educational simulator. It does not implement
industrial field-device protocols, durable historian storage, or safety functions.

## Roadmap

1. **Control core** - PID, sampling, water-tank model, bounds, and unit tests.
2. **DCS concepts** - named I/O tags, alarm limits, acknowledgement state, and
   CSV trend export.
3. **C++ communication** - completed local TCP device simulator/client with a
   documented wire format, telemetry, command acknowledgement, and connection
   state. Add a protocol adapter only when it is actually implemented.
4. **Qt HMI** - completed process overview, trend chart, setpoint entry,
   manual/auto mode, alarm, and connection status. Next: add an alarm list and
   connect it to the TCP transport.
5. **Portfolio polish** - screenshots, architecture diagram, tests, and a
   short engineering write-up.

## Honest portfolio framing

This repository demonstrates industrial-control and SCADA/HMI concepts through
a learning simulator. It does not claim OPC UA/Modbus interoperability,
functional-safety certification, or commercial DCS deployment.
