# Thermostat Project

Real, hardware-ready smart thermostat built on ESP32 with a touchscreen UI and a
web dashboard.

For the full implementation guide, read `IMPLEMENTATION_GUID.md`.

## What This Project Includes

- Firmware for the ESP32 (Arduino + PlatformIO + LVGL).
- Web dashboard for monitoring and control (Vite + React).
- Hardware wiring and component requirements (ESP32, TFT, touch, relays, sensor).

## Repository Layout

- `firmware/` - ESP32 firmware source, UI assets, state machine, sensors, outputs.
- `webDashboard/` - Web UI, MQTT client, and frontend assets.
- `icons/` - Project icon assets.
- `IMPLEMENTATION_GUID.md` - Detailed build, wiring, and system rules.

## Firmware (ESP32)

The firmware drives the state machine, relays, sensor reads, and LVGL UI.
Key components:

- State machine: `firmware/src/state_machine.cpp`
- Sensors (SHT3x): `firmware/src/sensors.cpp`
- Outputs/relays: `firmware/src/outputs.cpp`
- UI + LVGL glue: `firmware/src/ui.cpp`
- Touch driver: `firmware/src/touch.cpp`
- MQTT integration: `firmware/src/mqtt.cpp`

Build system: PlatformIO (see `firmware/platformio.ini`).

## Web Dashboard

The dashboard provides a mobile/desktop UI to monitor and control the thermostat
over MQTT.

- Entry point: `webDashboard/src/main.jsx`
- App UI: `webDashboard/src/App.jsx`
- MQTT client: `webDashboard/src/services/mqttClient.js`

See `webDashboard/README.md` and `webDashboard/mqtt_guid.md` for frontend setup
and MQTT configuration details.

## Hardware

Core hardware used by this project:

- ESP32 Dev Module (ESP-WROOM-32)
- 3.2" ILI9341 SPI TFT display (240x320)
- XPT2046 resistive touch controller
- SHT3x temperature/humidity sensor (I2C)
- 4x relay module for heater/cooler/fan/pump control

Exact pin mappings, wiring notes, and safety constraints are defined in
`IMPLEMENTATION_GUID.md`.

## MQTT / Connectivity

The firmware and web dashboard communicate via MQTT. The device publishes
telemetry (temperature, mode, state) and subscribes to control topics. Configure
broker settings in the firmware and in `webDashboard/src/services/mqttClient.js`.
