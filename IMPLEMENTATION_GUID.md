# Implementation Guide

We are building a REAL, HARDWARE-READY smart thermostat using ESP32.

## PLATFORM & TOOLS

- Board: ESP32 Dev Module (esp32dev)
- SoC module: ESP-WROOM-32 (ESP32; main MCU running Arduino/LVGL/state machine)
- Framework: Arduino
- Build system: PlatformIO
- Display: ILI9341 TFT (3.2" SPI v2.0, 240x320)
  - Default orientation: landscape (width=320, height=240)
  - Note: Some sample code uses portrait (240x320); this project defaults
    to 320x240 landscape. Adjust `tft.setRotation()` if a different orientation is required for your hardware.
  - LVGL buffer guidance: use a partial/double-buffer strategy to limit RAM
    usage. Example: one buffer of ~40 rows (40\*240 pixels) is a good starting point on an ESP32 with ~320KB usable SRAM.
- Graphics: LVGL (UI designed with SquareLine)
- Touch: PRESENT (must be implemented)
- Sensor: SHT3x (Temperature + Humidity, I2C)
- Outputs: 4 relays (Heater, Cooler, Fan, Pump)

## HARDWARE (PROJECT SPECIFIC)

- ESP-WROOM-32: main ESP32 module; runs firmware, Wi-Fi, MQTT, LVGL UI.
- SHT3x: I2C temperature + humidity sensor; provides ambient data for control loops.
- 3.2" TFT SPI 240x320 v2.0 (ILI9341): main user display; LVGL renders UI to it.
- 4x SRD-05VDC-SL-C relays: active-HIGH relay boards for heater, cooler, fan, pump control.

All code must be REAL, compilable, and suitable for real hardware.
NO pseudo-code.

## PIN MAPPING (MANDATORY – DO NOT CHANGE)

--- WIRING NOTES (READ FIRST) ---

- ESP32 GPIO pins are 3.3V logic only; never connect 5V to any GPIO.
- Always share GND between ESP32 and external modules (relay board, TFT, sensors).
- Power the relay board from a separate 5V source; do not draw relay current from ESP32 3.3V.
- Use COM + NO for loads so they stay OFF by default when ESP32 is unpowered.
- Boot-strap pins (GPIO 2, 4, 5, 15) must not be forced high/low at boot; avoid strong pull-ups/downs.
- GPIO34–GPIO39 are input-only; do not use them for outputs.

| Board Label (D) | GPIO | Notes            |
| --------------- | ---- | ---------------- |
| **VP**          | 36   | Input only (ADC) |
| **VN**          | 39   | Input only (ADC) |
| **D34**         | 34   | Input only       |
| **D35**         | 35   | Input only       |
| **D32**         | 32   | Input/Output     |
| **D33**         | 33   | Input/Output     |
| **D25**         | 25   | Input/Output     |
| **D26**         | 26   | Input/Output     |
| **D27**         | 27   | Input/Output     |
| **D14**         | 14   | SPI / OK         |
| **D12**         | 12   | Boot sensitive   |
| **D13**         | 13   | SPI / OK         |
| **D23**         | 23   | MOSI             |
| **D22**         | 22   | I2C SCL          |
| **TX0**         | 1    | Serial           |
| **RX0**         | 3    | Serial           |
| **D21**         | 21   | I2C SDA          |
| **D19**         | 19   | MISO             |
| **D18**         | 18   | SCK              |
| **D5**          | 5    | CS               |
| **D17**         | 17   | IO               |
| **D16**         | 16   | IO               |
| **D4**          | 4    | OK               |
| **D2**          | 2    | Boot sensitive   |
| **D15**         | 15   | Boot sensitive   |

--- RELAYS (SRD-05VDC-SL-C, Active HIGH) ---
Pump Relay IN → GPIO 25 (D25)
Fan Low Relay IN → GPIO 14 (D14)
Fan High Relay IN → GPIO 27 (D27)
Relay board DC+ → 5V
Relay board DC- → GND
Load wiring: use COM + NO (normally open) for default-OFF behavior
Load wiring (AC mains): LIVE → COM, NO → load LIVE, load NEUTRAL → NEUTRAL
Load wiring (DC): +V → COM, NO → load +, load - → GND/-

Default boot state: ALL OFF
Pump priming in PRESTART: run for 10s, then transition to COOLING_LOW/COOLING_HIGH.
Pump must be OFF in FAN_ONLY.

--- SHT3x SENSOR (I2C) ---
VIN → 3.3V
GND → GND
SDA → GPIO 21 (D21)
SCL → GPIO 22 (D22)
ADDR/AD → GND (for I2C address 0x44) or 3.3V (for I2C address 0x45)
ALERT → optional (only needed if you want interrupt/threshold alerts; otherwise leave unconnected)
I2C address: 0x44 (default with ADDR/AD tied low)
Notes: keep I2C wires short; if unstable, add 4.7k pull-ups to 3.3V on SDA/SCL.

--- TFT ILI9341 (SPI) ---
SDO (MISO) → GPIO 19 (D19)
LED (backlight) → 3.3V (always on)
SCK → GPIO 18 (D18)
SDI (MOSI) → GPIO 23 (D23)
D/C → GPIO 2 (D2)
RESET → GPIO 4 (D4)
CS → GPIO 15 (D15)
GND → GND
VCC → 3.3V
Notes: if you use PWM on LED, drive it through a transistor/driver if it draws >10–15mA.

--- TOUCH CONTROLLER (XPT2046, Resistive, SPI) ---

T_IRQ → GPIO 33 (D33)
T_OUT/T_DO → GPIO 19 (D19, shared SPI MISO)
T_DIN → GPIO 23 (D23, shared SPI MOSI)
T_CS → GPIO 5 (D5)
T_CLK → GPIO 18 (D18, shared SPI SCK)
Notes: if touch is unstable, add a small capacitor (100nF) near touch VCC/GND.

## PROJECT STRUCTURE

firmware/
├── src/
│ ├── main.cpp
│ ├── state_machine.h
│ ├── state_machine.cpp
│ ├── sensors.h
│ ├── sensors.cpp
│ ├── outputs.h
│ ├── outputs.cpp
│ ├── ui.h
│ ├── ui.cpp
│ ├── touch.h
│ ├── touch.cpp

## SYSTEM ARCHITECTURE (IMPORTANT)

- UI NEVER changes states directly.
- UI ONLY sets request flags.
- State Machine decides everything.
- Outputs are applied ONLY based on current state.
- Sensor failure must force safe state.

## STATE MACHINE

States:

- IDLE
- PRESTART
- COOLING_LOW
- COOLING_HIGH
- FAN_ONLY
- OFF

Rules:

- GLOBAL: Power OFF overrides everything; from any state, OFF request → OFF
- IDLE → PRESTART when temp >= setPoint or setPoint changes
- IDLE + FAN_ONLY user request → FAN_ONLY
- PRESTART:
  - Pump ON for 10s priming, then:
    - If temp invalid → OFF (safe state)
    - If currentTemp >= setPoint + hysteresis → COOLING_HIGH
    - Else if currentTemp >= setPoint → COOLING_LOW
    - Otherwise → IDLE
- COOLING_LOW / COOLING_HIGH:
  - If temp invalid → OFF
  - If currentTemp <= setPoint - hysteresis → IDLE
  - If currentTemp >= setPoint + hysteresis → COOLING_HIGH
  - If currentTemp between setPoint and setPoint + hysteresis → COOLING_LOW
  - User FAN_ONLY request → FAN_ONLY (pump OFF)
- FAN_ONLY:
  - Fan LOW only (pump OFF), ignores setPoint
- OFF: stays OFF until power restored (if restart)

NEVER compare floats using ==

## HYSTERESIS (MANDATORY)

Configurable hysteresis (example: 0.5°C)

COOLING:

- Cooling HIGH when currentTemp >= setPoint + hysteresis
- Cooling LOW when currentTemp between setPoint and setPoint + hysteresis
- Cooling OFF when currentTemp <= setPoint - hysteresis

## SENSOR REQUIREMENTS (SHT3x)

- Use Adafruit_SHT31 library
- Initialize I2C manually
- Read temperature & humidity
- Handle:
  - sensor not found
  - NaN readings
- Limit read rate (≈250ms)

## OUTPUT LOGIC

- PRESTART → Pump ON for 10s priming
- COOLING_LOW → Pump ON + Fan LOW
- COOLING_HIGH → Pump ON + Fan HIGH
- FAN_ONLY → Fan LOW only (Pump OFF)
- IDLE → all OFF
- OFF → all OFF

## SAFETY CHECKS (HARDWARE)

- Relays must default OFF at boot; configure GPIO outputs before enabling any logic.
- Use COM + NO on relay outputs for default-OFF load behavior;
- Relay module DC+ must be 5V and DC- GND; do not feed 5V into ESP32 GPIO pins.
- Backlight LED pin must be driven through a suitable transistor or LED driver; do not source high current directly from ESP32.
- Be careful with boot-strap pins (GPIO 2, 4, 5, 15): keep external pull-ups/downs per ESP32 boot requirements to avoid boot failure.

## UI & TOUCH (LVGL)

- Implement LVGL input device driver for touch
- Support XPT2046
- Map touch to screen coordinates
- UI callbacks must ONLY set flags:
  - setPointChanged
  - fanOnlyRequested
  - powerOffRequested
  - modeChanged

## IMPLEMENTATION STEPS

STEP 1:

- Create file skeleton
- Enums (State, Mode)
- Global shared variables
- Empty stubs
- Project must BUILD

STEP 2 (RELAYS):

- Relay GPIO setup
- Safe startup state (all relays OFF)
- Relay ON/OFF helper functions

STEP 3 (SENSORS):

- SHT3x implementation with I2C init
- Handle missing sensor/NaN
- ~1s rate limit and brief SHT3x comment

STEP 4 (PARAMS & FLAGS):

- Decision variables
- Flags wiring
- Parameters (setPoint, hysteresis)

STEP 5 (STATE LOGIC, NO OUTPUTS):

- Full state machine transitions per rules, hysteresis-ready comparisons

STEP 6 (OUTPUT APPLICATION):

- applyOutputs()
- Pump only during HEATING

STEP 7 (HYSTERESIS):

- Add heater/cooler hysteresis control

STEP 8 (TOUCH + LVGL INPUT):

- Touch driver for XPT2046
- LVGL input mapping

STEP 9 (UI FLAGS ONLY):

- UI update loop
- Callbacks set flags only

NOTE: The UI must call the state machine flag API rather than manipulating state directly. Use the provided API functions:

- `setSetPoint(float celsius)` — update setpoint and set the setPointChanged flag
- `setHysteresis(float celsius)` — update hysteresis parameter
- `requestFanOnly()` — request FAN_ONLY (sets `gFanOnlyRequested`)
- `requestPowerOff()` — request immediate power OFF (sets `gPowerOffRequested`)
- `requestModeChange(ThermostatMode newMode)` — request a mode change (sets `gModeChanged`)

These functions must be called by UI callbacks and drivers; they must not directly change `gCurrentState`.

STEP 10 (SAFETY):

- Sensor fail-safe and OFF override

STEP 11 (MQTT/DASHBOARD):

- Implement MQTT and phone webDashboard integration

## PROJECT TODO LIST (BUGS / GAPS)

- Fix LCD touch.

- Connect to Wi-Fi and MQTT server on device.
- Check web dashboard and device connectivity via MQTT to have integrated data.

- test relays.

- Use three relays: one for pump, one for high speed, one for low speed (evaporative cooler design).(check it)
- icons
