You are an expert embedded systems / IoT engineer.

We are building a REAL, HARDWARE-READY smart thermostat using ESP32.

================================================
PLATFORM & TOOLS
================================================

- Board: ESP32 Dev Module (esp32dev)
- SoC module: ESP-WROOM-32 (ESP32; main MCU running Arduino/LVGL/state machine)
- Framework: Arduino
- Build system: PlatformIO
- Display: ILI9341 TFT
- Display: ILI9341 TFT (3.2" SPI v2.0, 240x320)
  - Default orientation: landscape (width=320, height=240)
  - Note: Some sample code uses portrait (240x320); this project defaults
    to 320x240 landscape. Adjust `tft.setRotation()` if a different orientation
    is required for your hardware.
  - LVGL buffer guidance: use a partial/double-buffer strategy to limit RAM
    usage. Example: one buffer of ~40 rows (40\*240 pixels) is a good starting
    point on an ESP32 with ~320KB usable SRAM.
- Graphics: LVGL (UI designed with SquareLine)
- Touch: PRESENT (must be implemented)
- Sensor: SHT31 (Temperature + Humidity, I2C)
- Outputs: 4 relays (Heater, Cooler, Fan, Pump)

================================================
HARDWARE (PROJECT SPECIFIC)
================================================

- ESP-WROOM-32: main ESP32 module; runs firmware, Wi-Fi, MQTT, LVGL UI.
- SHT3x: I2C temperature + humidity sensor; provides ambient data for control loops.
- 3.2" TFT SPI 240x320 v2.0 (ILI9341): main user display; LVGL renders UI to it.
- 4x CW-025 relays: active-HIGH relay boards for heater, cooler, fan, pump control.

All code must be REAL, compilable, and suitable for real hardware.
NO pseudo-code.

================================================
PIN MAPPING (MANDATORY – DO NOT CHANGE)
================================================

--- RELAYS (Active HIGH) ---
Heater Relay → GPIO 26
Cooler Relay → GPIO 27
Fan Relay → GPIO 14
Pump Relay → GPIO 25

Default boot state: ALL OFF
Pump priming in PRESTART: run for 10s, then transition along the COOLING path. Pump must be OFF before entering FAN_ONLY or HEATING.

--- SHT31 SENSOR (I2C) ---
VCC → 3.3V
GND → GND
SDA → GPIO 21
SCL → GPIO 22
I2C address: 0x44

--- TFT ILI9341 (SPI) ---
MOSI → GPIO 23
MISO → GPIO 19
SCK → GPIO 18
CS → GPIO 5
DC → GPIO 2
RST → GPIO 4
VCC → 3.3V
GND → GND

--- TOUCH CONTROLLER (CHOOSE ONE, SUPPORT BOTH) ---

Option A: XPT2046 (Resistive, SPI)

- T_CLK → GPIO 18 (shared SPI SCK)
- T_DIN → GPIO 23 (shared SPI MOSI)
- T_DO → GPIO 19 (shared SPI MISO)
- T_CS → GPIO 15
- T_IRQ → GPIO 33

Option B: FT6206 / FT6236 (Capacitive, I2C)

- SDA → GPIO 21 (shared I2C)
- SCL → GPIO 22 (shared I2C)
- INT → GPIO 33 (optional)

Use compile-time selection:
#define TOUCH_TYPE_XPT2046 1
#define TOUCH_TYPE_FT6X36 2
#define TOUCH_TYPE TOUCH_TYPE_XPT2046 // or TOUCH_TYPE_FT6X36

================================================
PROJECT STRUCTURE (MANDATORY)
================================================

src/
├── main.cpp
├── state_machine.h
├── state_machine.cpp
├── sensors.h
├── sensors.cpp
├── outputs.h
├── outputs.cpp
├── ui.h
├── ui.cpp
├── touch.h
├── touch.cpp

================================================
SYSTEM ARCHITECTURE (IMPORTANT)
================================================

- UI NEVER changes states directly.
- UI ONLY sets request flags.
- State Machine decides everything.
- Outputs are applied ONLY based on current state.
- Sensor failure must force safe state.

================================================
STATE MACHINE (MUST MATCH EXACTLY)
================================================

States:

- IDLE
- PRESTART
- HEATING
- COOLING
- FAN_ONLY
- OFF

Modes:

- WINTER
- SUMMER

Rules:

- GLOBAL: Power OFF overrides everything; from any state, OFF request → OFF
- GLOBAL: Mode change always → PRESTART (then branches normally)
- IDLE → PRESTART when setPoint changes
- IDLE + FAN_ONLY user request → PRESTART → FAN_ONLY (2-step transition)
- PRESTART (first enter or after mode change):
  - 10s pump priming (pump ON)
  - After 10s, check conditions:
    - WINTER & setPoint > currentTemp → HEATING (pump OFF before entering)
    - SUMMER & setPoint < currentTemp → COOLING (pump stays ON if needed)
    - Otherwise (setPoint condition not met) → IDLE (pump OFF)
- HEATING:
  - setPoint changes: check current comparison
    - If WINTER & setPoint > currentTemp: stay HEATING or keep checking
    - If NOT (WINTER & setPoint > currentTemp): → IDLE (with hysteresis)
  - User OFF request → OFF
  - Mode change → PRESTART
  - User FAN_ONLY request → FAN_ONLY (pump OFF first)
- COOLING:
  - setPoint changes: check current comparison
    - If SUMMER & setPoint < currentTemp: stay COOLING or keep checking
    - If NOT (SUMMER & setPoint < currentTemp): → IDLE (with hysteresis)
  - User OFF request → OFF
  - Mode change → PRESTART
  - User FAN_ONLY request → FAN_ONLY (pump OFF first)
- FAN_ONLY:
  - User OFF request → OFF
  - Mode change → PRESTART
  - setPoint change checked but FAN_ONLY doesn't respond to setPoint directly (remains FAN_ONLY)
- OFF: stays OFF until mode change or power restored (if restart)

NEVER compare floats using ==

================================================
HYSTERESIS (MANDATORY)
================================================

Configurable hysteresis (example: 0.5°C)

WINTER:

- Heater ON when currentTemp <= setPoint - hysteresis
- Heater OFF when currentTemp >= setPoint + hysteresis

SUMMER:

- Cooler ON when currentTemp >= setPoint + hysteresis
- Cooler OFF when currentTemp <= setPoint - hysteresis

================================================
SENSOR REQUIREMENTS (SHT31)
================================================

- Use Adafruit_SHT31 library
- Initialize I2C manually
- Read temperature & humidity
- Handle:
  - sensor not found
  - NaN readings
- Limit read rate (≈1s)
- Comment what SHT31 is and why it’s better than DHT22

================================================
OUTPUT LOGIC
================================================

- PRESTART → Pump ON for 10s (priming), then follow COOLING path
- HEATING → Heater + Fan (Pump OFF before entering HEATING)
- COOLING → Cooler + Fan
- FAN_ONLY → Fan only (Pump OFF before entering)
- IDLE → all OFF
- OFF → all OFF

================================================
UI & TOUCH (LVGL)
================================================

- Implement LVGL input device driver for touch
- Support both XPT2046 and FT6X36
- Map touch to screen coordinates
- UI callbacks must ONLY set flags:
  - setPointChanged
  - fanOnlyRequested
  - powerOffRequested
  - modeChanged

================================================
IMPLEMENTATION RULES (VERY IMPORTANT)
================================================

You MUST implement the project STEP BY STEP.
After EACH step:

1. Output only the code for that step
2. STOP
3. Ask:

"Reply OK to continue to the next step."

DO NOT continue without explicit OK.

================================================
IMPLEMENTATION STEPS (ADJUSTABLE AFTER STEP 1)
================================================

STEP 1 (DONE):

- Create file skeleton
- Enums (State, Mode)
- Global shared variables
- Empty stubs
- Project must BUILD
  STOP and wait for OK.

STEP 2 (RELAYS) (DONE):

- Relay GPIO setup
- Safe startup state (all relays OFF)
- Relay ON/OFF helper functions
  STOP and wait for OK.

STEP 3 (SENSORS) (DONE):

- SHT31 implementation with I2C init
- Handle missing sensor/NaN
- ~1s rate limit and comment vs DHT22
  STOP and wait for OK.

STEP 4 (PARAMS & FLAGS)(DONE):

- Decision variables
- Flags wiring
- Parameters (setPoint, hysteresis)
  STOP and wait for OK.

STEP 5 (STATE LOGIC, NO OUTPUTS)(DONE):

- Full state machine transitions per rules, hysteresis-ready comparisons
  STOP and wait for OK.

STEP 6 (OUTPUT APPLICATION)(DONE):

- applyOutputs()
- Pump only during HEATING
  STOP and wait for OK.

STEP 7 (HYSTERESIS) (DONE):

- Add heater/cooler hysteresis control
  STOP and wait for OK.

STEP 8 (TOUCH + LVGL INPUT)(DONE):

- Touch driver for XPT2046 and FT6X36
- LVGL input mapping
  STOP and wait for OK.

STEP 9 (UI FLAGS ONLY) (DONE):

- UI update loop
- Callbacks set flags only
  STOP and wait for OK.

NOTE: The UI must call the state machine flag API rather than manipulating state directly. Use the provided API functions:

- `setSetPoint(float celsius)` — update setpoint and set the setPointChanged flag
- `setHysteresis(float celsius)` — update hysteresis parameter
- `requestFanOnly()` — request FAN_ONLY (sets `gFanOnlyRequested`)
- `requestPowerOff()` — request immediate power OFF (sets `gPowerOffRequested`)
- `requestModeChange(ThermostatMode newMode)` — request a mode change (sets `gModeChanged`)

These functions must be called by UI callbacks and drivers; they must not directly change `gCurrentState`.

STEP 10 (SAFETY) (DONE):

- Sensor fail-safe and OFF override
  STOP and wait for OK.

STEP 11 (MQTT/DASHBOARD):

- Implement MQTT and phone webapp dashboard integration
  STOP and wait for OK.

================================================
OPEN TODOs (keep updated)
================================================

- Implement relay GPIO init + safe startup (STEP 2).
- Implement SHT31 reads with Adafruit_SHT31, I2C init, NaN/absent handling, ~1s rate, and doc comment vs DHT22 (STEP 3).
- Add decision vars, flags, setPoint/hysteresis wiring (STEP 4).
- Complete state machine transitions (no outputs) per rules and hysteresis-ready comparisons (STEP 5), including PRESTART pump-priming that feeds into COOLING after 10s.
- Implement applyOutputs with updated pump behavior (priming in PRESTART, pump OFF before HEATING/FAN_ONLY) (STEP 6).
- Add hysteresis control for heater/cooler (STEP 7).
- Implement touch drivers for XPT2046 and FT6X36 and LVGL input mapping (STEP 8).
- Wire UI callbacks to flags only; update UI task loop (STEP 9).
- Add safety/fail-safe on sensor failure (STEP 10).
- Implement MQTT + phone dashboard integration (STEP 11).

================================================
PROJECT TODO LIST (BUGS / GAPS)
================================================

- mark each item as done after fix.

1. Persist Wi-Fi credentials in NVS and auto-reconnect on boot. (DONE)
2. Add Wi-Fi connect timeout/failure feedback and cancel/clear credentials. (DONE)
3. Decide sensor-recovery behavior after OFF (auto return to IDLE or require user action): require user action. (DONE)
4. Ensure pump behavior in COOLING matches rules (pump on when needed; pump changes only in PRESTART). (DONE)
5. Keep hysteresis default consistent (one source of truth). (DONE)
6. Update UI labels from live data (current temp, humidity, setpoint) instead of static text. (DONE)
7. Add explicit setpoint arc handling (map arc value to setpoint) and wire mode change control. (DONE)
8. Add FAN_ONLY toggle behavior and UI state indication. (DONE)
9. Add sensor-fault telemetry flag to MQTT. (DONE)
10. Add MQTT auth/TLS, LWT, and reconnect backoff; move config to NVS (not compile-time only). (DONE)
11. Webapp: add setpoint controls and MQTT connection status indicator. Hysteresis is hard-coded (no user control). (DONE)
12. Reduce duplication between src/ and lib/ui/src/ to avoid divergence. (DONE)
13. Document project-specific hardware list and usage. (DONE)
14. Fix MQTT setpoint telemetry topic mismatch (device publishes `setpoint_c`, webapp listens for `setpoint`). (DONE)
15. Align MQTT broker config between device and webapp (host/port/base topic). (DONE)
16. Ensure MQTT broker WebSocket listener is enabled and matches `VITE_MQTT_URL` in webapp. (DONE - added `thermostat-web-app/.env.example`). (DONE)
17. Save valid Wi‑Fi credentials on device to allow MQTT connection. (DONE)
18. Run MQTT broker with WebSockets enabled and ensure host/port match device/webapp config.
19. Flash firmware to ESP32 and verify Wi‑Fi + MQTT connectivity on hardware.
20. Validate sensors + relay outputs on hardware (SHT3x readings, heater/cooler/fan/pump behavior, OFF fail‑safe).
21. Run touch calibration on first boot if using XPT2046.
