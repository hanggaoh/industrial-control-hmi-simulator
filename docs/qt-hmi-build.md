# Qt HMI build

`industrial_control_hmi` is an optional Qt Widgets desktop target.  CMake
searches first for Qt 6 Widgets, then Qt 5 Widgets.  If neither is available,
it prints a skip message and still builds the simulator and tests.

## Build when Qt is installed

```sh
cmake -S . -B build/cmake
cmake --build build/cmake --target industrial_control_hmi
./build/cmake/industrial_control_hmi
```

The board displays the latest level, setpoint, inlet command, alarm and
connection state.  A simple in-process simulator supplies tags every 500 ms;
the view model is intentionally independent of that simulator and can later
be updated by the TCP device client.  The trend draws level and setpoint, and
the Auto/Manual and setpoint controls publish an operator command to the view
model.

## Current workstation status

Qt 5/6 Widgets was not found on this workstation when this document was
created.  Therefore this target was deliberately skipped during verification;
the non-Qt CMake build and tests remain available.
