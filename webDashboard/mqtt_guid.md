# MQTT Guide

## Overview
- Base topic: `thermostat`
- Device publishes telemetry to `thermostat/telemetry/<key>`
- Device subscribes to commands on `thermostat/cmd/<command>`
- Dashboard subscribes to `thermostat/telemetry/#` and publishes to `thermostat/cmd/<command>`

## Transport
- Device: MQTT over TLS on port `8883`
- Web dashboard (browser): MQTT over WebSockets TLS on port `8884`, path `/mqtt`

## Telemetry (device -> dashboard)
- `thermostat/telemetry/temp_c` -> string float (e.g. `23.50`)
  - UI: current temperature
- `thermostat/telemetry/humidity` -> string float (e.g. `41.20`)
  - UI: humidity
- `thermostat/telemetry/state` -> `idle`, `prestart`, `heating`, `cooling`, `fan_only`, `off`
  - UI: power state + fan indicator
- `thermostat/telemetry/mode` -> `summer` or `winter`
  - UI: mode icon
- `thermostat/telemetry/setpoint_c` -> string float
  - UI: target setpoint
- `thermostat/telemetry/fan_speed` -> `0`, `1`, or `2`
  - UI: fan speed indicator
- `thermostat/telemetry/hysteresis_c` -> string float
  - UI: not displayed
- `thermostat/telemetry/pump_desired` -> `0` or `1`
  - UI: pump icon animation
- `thermostat/telemetry/sensor_ok` -> `0` or `1`
  - UI: not displayed
- `thermostat/telemetry/status` -> `online` on connect, `offline` as last will
  - UI: not displayed

## Commands (dashboard -> device)
- `thermostat/cmd/setpoint` -> float string (e.g. `24.0`)
  - Device: update target temperature
- `thermostat/cmd/hysteresis` -> float string
  - Device: update hysteresis
- `thermostat/cmd/mode` -> `summer` or `winter`
  - Device: switch mode
- `thermostat/cmd/fan_only` -> `0`, `1`, or `2`
  - `0`: fan off
  - `1`: fan slow
  - `2`: fan fast
- `thermostat/cmd/power` -> `0`, `off`, or `false`
  - Device: power off (power on is done by sending a mode)

## UI Behavior Notes
- The dashboard updates only on telemetry.
- Sending a command does not change the UI unless telemetry is published afterward.
