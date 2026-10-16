# Module boundaries

The project is deliberately split before adding network or GUI features.

```text
Field device / TCP simulator
        │ ProcessTags + ConnectionStatus
        ▼
transport::IDeviceClient ──► application::IControlService
        │                           │ ControlCommand
        └──────────────► hmi::IHmiViewModel
                                      │
                                      ▼
                            Qt process overview / trend / alarms
```

## Contracts

- `domain/`: protocol-independent tags, commands, quality and alarm state.
- `transport/`: device lifecycle, telemetry callback and command dispatch. A
  Boost.Asio TCP client belongs here; Modbus/DNP3 would be separate adapters.
- `application/`: PID and auto/manual control policy with no networking or UI.
- `hmi/`: a UI-facing state/trend interface. Qt is an implementation detail.

## Parallel implementation boards

1. **Transport board**: implement an `IDeviceClient` backed by Boost.Asio TCP,
   including reconnect and communication alarms.
2. **Control board**: adapt the PID loop into `IControlService`; add bounds,
   alarm policy and deterministic tests.
3. **HMI board**: implement `IHmiViewModel` and an optional Qt Widgets process
   view with trends, alarm history/acknowledgement, simulated disturbance and
   manual/auto controls. Alarm acknowledgement never clears a process
   condition; only a later normal process tag returns it to normal.
4. **Portfolio board**: screenshots, a reproducible demo script, protocol
   notes and a concise engineering write-up.

No board should couple directly to another board's implementation. They share
only the headers above.

## Energy-storage control loop

```text
local BMS / inverter / meter simulator
              │ EnergyTelemetry
              ▼
    application::StorageControlService
       │                 │
       │                 ├── IDispatchStrategy (replaceable policy)
       │                 └── SafetyLimiter (always applied)
       ▼
      PowerCommand ──► simulated inverter
```

The dispatch strategy selects charge, discharge, or idle from site net load and
SOC. `SafetyLimiter` is outside the strategy so a future optimiser cannot
bypass connection, fault, temperature, state-of-charge, battery capability, or
inverter-rating limits. This is a local simulation; it makes no claim to model
an actual battery or inverter accurately.
