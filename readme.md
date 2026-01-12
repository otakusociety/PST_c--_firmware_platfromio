# AI Coding Instructions for PST Firmware

## Project Overview
**Portable Scripting Terminal (PST)** - 

The Portable Scripting Terminal (PST) is a handheld, touchscreen-based development hub for embedded systems (ESP32/MicroPython projects). It's both a file editor and a remote control dashboard for managing and interacting with projects in the field—no laptop required.

The Three Modes
Edit Mode

File browser + text editor on the PST itself (SD card storage)
Write/edit .py scripts and config files
Upload scripts to projects or download their current running code
Prototype approach: PST firmware in C++ doesn't execute code—just edits and transfers
Control Mode

Discover and connect to nearby ESP32 projects (via ESP-NOW, WiFi, serial)
Parse each project's project.json config
Auto-generate a custom UI from the config (sliders, toggles, graphs, etc.)
Send commands and receive telemetry in real-time
Persistent upload/download buttons for script management
Status Mode

Display PST health: battery, WiFi, storage, firmware version, logs
Simple info dashboard
Architecture: C++ Firmware + MicroPython Projects
PST Firmware (C++, on the PST device):

LVGL-based UI renderer
File I/O (SD card, internal storage)
Communication stack (ESP-NOW, WiFi, serial drivers)
Config parser (reads project.json)
UI generator (auto-creates controls from config)
No script execution
Project Firmware (MicroPython, on remote ESP32 boards):

Standard MicroPython code
Includes pstlib library (~200 lines)
Includes project.json config file
Simple registration calls to expose hardware/sensors to PST
pst.run_in_background() handles all PST communication non-blocking
Project Integration: pstlib + project.json

pstlib Registers (Binding hardware to PST UI)

register_action()	✅ Core	User controls (toggle, slider, joystick)
register_telemetry()	✅ Core	Sensor data with graphing/gauging
register_input()	✅ Core	Hardware state indicators (mirrors physical state)
register_event()	✅ Core	Alerts, logs, timers, conditions
register_file()	✅ Core	File upload/download/edit integration
register_text()	✅ Core	Static/dynamic labels
register_test()	✅ Core	One-shot hardware tests
render tag	✅ Smart	Per-item custom UI override without full override
trigger_event chaining	✅ Advanced	Event A → triggers Event B (log_alerts example)
mass_settings styling	✅ Good	Global theme for auto-UI
ui_mode (auto/custom/hybrid)	✅ Flexible	Choose per-project

Register Type	Purpose	Example	Generated UI

Project Integration: pstlib + project.json
pstlib Registers (Binding hardware to PST UI)
Register Type	Purpose	Example	Generated UI
register_action()	Writable control (motors, relays, LEDs)	pst.register_action("led", Pin(2, Pin.OUT))	Button/slider/joystick
register_telemetry()	Read-only sensor data (periodic polling)	pst.register_telemetry("temp", lambda: read_temp())	Live graph/gauge/number
register_input()	Hardware state indicator mirrors physical hardware input	pst.register_input("button1", lambda: Pin(14).value())	Status light/indicator
register_event()	Alert/trigger condition	pst.register_event("alert1", lambda: tank_empty())	Popup/log entry
register_file()	File access (edit/download)	pst.register_file("utils", "utils.py")	File browser link
register_text()	Dynamic label	pst.register_text("uptime", lambda: get_uptime())	Info label
register_test()	One-shot hardware test	pst.register_test("blink", run_blink_test)	Test button
Key strengths:

Zero boilerplate (3 lines = full integration)
Type-agnostic (Pin objects, lambdas, PWM all work)
Non-blocking background operation
Progressive (start with action+telemetry, add custom UI later)
project.json Schema (UI/Config Declaration)


Complete Register System Walkthrough
Here's a comprehensive example showing all registers in action with Python code, JSON definitions, and PST behavior:

1. register_action() - User Controls
What it does: Lets PST send commands to hardware (writable controls)

Python (Project Side)
from machine import Pin
from pstlib import PSTApp

# Hardware setup
led = Pin(2, Pin.OUT)
pump_motor = PWM(Pin(5), freq=1000)  # PWM motor control

# Register actions
pst = PSTApp("project.json")
pst.register_action("led", led)  # Simple on/off
pst.register_action("pump_speed", pump_motor.duty)  # Accepts value (0-1023)
pst.register_action("solenoid", lambda v: relay.set(v > 0))  # Custom callback

JSON (Project Configuration)
"action": {
  "led": { 
    "type": "toggle", 
    "label": "Main LED",
    "default": false 
  },
  "pump_speed": { 
    "type": "slider", 
    "label": "Pump Speed",
    "min": 0, 
    "max": 1023,
    "unit": "PWM",
    "default": 512
  },
  "solenoid": {
    "type": "momentary",
    "label": "Water Release",
    "duration": 2  // Hold for 2 seconds
  }
}

What Happens on PST
User taps LED toggle → PST sends: {"cmd": "led", "value": 1}
pstlib receives → calls led.on()
User drags Pump Speed slider → sends: {"cmd": "pump_speed", "value": 750}
pstlib receives → calls pump_motor.duty(750)
✅ Result: Real hardware responds to PST touches

2. register_telemetry() - Sensor Data
What it does: Continuous polling of sensor data with graphing and history

Python (Project Side)

from dht import DHT22
from machine import ADC, Pin

# Hardware
temp_sensor = DHT22(Pin(4))
humidity_sensor = DHT22(Pin(4))  # Same sensor, different read
soil_moisture = ADC(Pin(35))  # 0-4095

def read_soil():
    return soil_moisture.read() / 4095 * 100  # Convert to %

pst = PSTApp("project.json")
pst.register_telemetry("temperature", lambda: temp_sensor.temperature(), rate=2.0)  # Poll every 500ms
pst.register_telemetry("humidity", lambda: humidity_sensor.humidity(), rate=2.0)
pst.register_telemetry("soil_moisture", read_soil, rate=1.0)  # Poll every 1000ms
pst.register_telemetry("battery_voltage", lambda: battery_adc.read() * 3.3 / 4095, rate=0.1)  # Slow poll

JSON (Project Configuration)

"telemetry": {
  "temperature": {
    "type": "graph",
    "label": "Temperature",
    "unit": "°C",
    "refresh_rate": 2,
    "min": 15,
    "max": 40,
    "line_color": "#FF5733"
  },
  "humidity": {
    "type": "gauge",
    "label": "Humidity",
    "unit": "%",
    "refresh_rate": 2,
    "min": 0,
    "max": 100,
    "warning_threshold": 80
  },
  "soil_moisture": {
    "type": "number",
    "label": "Soil Moisture",
    "unit": "%",
    "precision": 1
  },
  "battery_voltage": {
    "type": "number",
    "label": "Battery",
    "unit": "V",
    "precision": 2
  }
}

What Happens on PST
PST discovers project → parses telemetry section
Every 500ms → PST polls: {"request": "temperature"}
pstlib calls temp_sensor.temperature() → returns 24.5
pstlib sends: {"telemetry": "temperature", "value": 24.5}
PST receives → plots on graph (builds history over time)
PST UI shows real-time updating graph, gauge, and number displays
✅ Result: Live sensor visualization with history

3. register_input() - Hardware State Mirroring
What it does: Mirrors physical hardware state on PST (indicators, not controls)

Python (Project Side)

from machine import Pin

# Hardware inputs (read-only from PST perspective)
button1 = Pin(14, Pin.IN)
motion_sensor = Pin(13, Pin.IN)
door_switch = Pin(12, Pin.IN)

# LED status (physical LED that might blink - mirror its state)
led_indicator = Pin(2, Pin.OUT)

pst = PSTApp("project.json")
pst.register_input("button", lambda: button1.value())  # 0 or 1
pst.register_input("motion", lambda: motion_sensor.value())  # 0 or 1
pst.register_input("door_open", lambda: door_switch.value())  # 0 or 1
pst.register_input("led_status", lambda: led_indicator.value())  # Mirror: if LED is on, show on

JSON (Project Configuration)

"input": {
  "button": {
    "type": "indicator",
    "label": "Button State",
    "states": ["released", "pressed"],
    "colors": ["green", "red"]
  },
  "motion": {
    "type": "indicator",
    "label": "Motion Detected",
    "states": ["idle", "motion"],
    "colors": ["gray", "yellow"]
  },
  "door_open": {
    "type": "indicator",
    "label": "Door",
    "states": ["closed", "open"],
    "colors": ["green", "red"]
  },
  "led_status": {
    "type": "toggle",  // Visual only
    "label": "LED Status (mirrored)",
    "readonly": true
  }
}

What Happens on PST
PST polls every 100ms: {"request": "input"}
pstlib queries all registered inputs:
button1.value() → 0
motion_sensor.value() → 1
door_switch.value() → 0
led_indicator.value() → 1
PST receives: {"input": {"button": 0, "motion": 1, "door_open": 0, "led_status": 1}}
PST updates UI:
Button indicator → green (released)
Motion indicator → yellow (motion detected!)
Door indicator → green (closed)
LED status → ON (mirrored)
✅ Result: PST shows real-time hardware state without controlling it

4. register_event() - Alerts, Logs, Triggers
What it does: Conditions that trigger alerts/logs with captured context

Python (Project Side)

from machine import Pin, ADC

# Sensor setup
tank_level = ADC(Pin(35))  # 0-4095
temp_sensor = DHT22(Pin(4))

def read_tank_percent():
    return tank_level.read() / 4095 * 100

def read_temp():
    return temp_sensor.temperature()

pst = PSTApp("project.json")

# Register multiple events (conditions that trigger)
pst.register_event("low_water", lambda: read_tank_percent() < 20)
pst.register_event("overtemp", lambda: read_temp() > 35, "Critical: Temperature too high!")
pst.register_event("motor_error", lambda: motor.error_flag)
pst.run_in_background()

JSON (Project Configuration)

"event": {
  "low_water": {
    "type": "alert",
    "message": "Water tank level is low! Refill required.",
    "priority": "high",
    "auto_dismiss": false  // User must close
  },
  
  "overtemp": {
    "type": "alert",
    "message": "Temperature critical! Shutting down pump.",
    "priority": "critical",
    "auto_dismiss": false,
    "sound": true  // Beep on alert
  },
  
  "motor_error": {
    "type": "alert",
    "message": "Motor malfunction detected.",
    "priority": "medium"
  },
  
  "log_water_alerts": {
    "type": "log",
    "trigger_event": "low_water",
    "label": "Water Level Log",
    "data_points": [
      "%time",           // HH:MM:SS
      "%date",           // YYYY-MM-DD
      "tank_level",      // Pull from telemetry
      "message",         // The alert message
      "status"           // PST system state (battery, wifi, etc.)
    ],
    "path": "/sd/logs/water_alerts.txt"
  },
  
  "log_temp_alerts": {
    "type": "log",
    "trigger_event": "overtemp",
    "label": "Temperature Log",
    "data_points": [
      "%timestamp",      // Unix timestamp
      "temperature",     // From telemetry
      "humidity",        // From telemetry
      "pump_speed",      // From action state
      "message",
      "status"
    ],
    "path": "/sd/logs/temp_alerts.txt"
  }
}

What Happens on PST
PST polls events every 100ms
pstlib evaluates: lambda: read_tank_percent() < 20 → returns True
PST detects low_water event triggered
PST displays alert popup: "Water tank level is low! Refill required."
PST simultaneously logs to /sd/logs/water_alerts.txt:

2025-01-12 14:32:45 | 12:32:45 | 2025-01-12 | tank_level=18.5 | Water tank level is low! Refill required. | battery=87% wifi=connected psram_free=2.1MB
2025-01-12 14:45:22 | 12:45:22 | 2025-01-12 | tank_level=15.2 | Water tank level is low! Refill required. | battery=85% wifi=connected psram_free=2.1MB

✅ Result: Rich alerting with contextual logging for debugging

5. register_file() - File Management
What it does: Expose files for download, edit, upload from PST. SPecify which files are allowed to be edited. 

Python (Project Side)

pst = PSTApp("project.json")

# Expose files
pst.register_file("config", "path to config.json", readonly=False)  # Edit on PST
pst.register_file("logs", "path to logs.txt", readonly=True)  # Download only
pst.register_file("main_script", "path to main.py", readonly=False)  # Edit and test

JSON (Project Configuration)

"file": {
  "config": {
    "type": "file",
    "label": "Configuration File",
    "readonly": false
  },
  "logs": {
    "type": "file",
    "label": "System Logs",
    "readonly": true
    
  },
  "main_script": {
    "type": "file",
    "label": "Main Script",
    "readonly": false
  },
  custom_ui: {
    "type": "file",
    "label": "Custom UI Script",
    "readonly": false
  }

}

What Happens on PST
PST shows File Manager tab with 3 files listed
User taps "Project Configuration":
PST downloads config.json from project
Opens in PST text editor
User modifies (e.g., changes "pump_speed": 100 → 150)
User taps Save
PST uploads modified file back to project
User taps "System Logs" (readonly):
Can view/download but cannot edit
User taps "Main Script":
Can edit, with Python syntax highlighting
Can test changes (trigger reload on project)
✅ Result: Full file management without removing device from field

6. register_text() - Labels & Status
What it does: Display static or dynamically updated text

Python (Project Side)

def get_uptime():
    # Returns uptime in hours:minutes:seconds
    return f"{hours}h {minutes}m {seconds}s"

def get_pump_status():
    if pump.is_running():
        return f"Running @ {pump.get_speed()}%"
    return "Idle"

pst = PSTApp("project.json")
pst.register_text("firmware_version", "v1.0.4-beta")  # Static
pst.register_text("uptime", get_uptime)  # Dynamic (polled)
pst.register_text("pump_status", get_pump_status)  # Dynamic
pst.register_text("last_error", lambda: error_log[-1] if error_log else "None")

JSON (Project Configuration)

"text": {
  "firmware_version": {
    "label": "Firmware Version",
    "type": "text",
    "format": "%s"
  },
  "uptime": {
    "label": "System Uptime",
    "type": "text",
    "format": "%s",
    "refresh_rate": 1  // Update every 1 second
  },
  "pump_status": {
    "label": "Pump Status",
    "type": "text",
    "format": "%s",
    "refresh_rate": 0.5  // Update twice per second
  },
  "last_error": {
    "label": "Last Error",
    "type": "text",
    "format": "%s",
    "refresh_rate": 2
  }
}

What Happens on PST
PST displays info panel with 4 text fields
Firmware Version shows: v1.0.4-beta (static, never changes)
Uptime shows: 5h 23m 47s and updates every 1s
Pump Status shows: Running @ 75% and updates 2x per second
Last Error shows: Motor overheat detected and updates every 2s
✅ Result: Real-time status display without complex graphs

7. register_test() - Hardware Tests
What it does: One-shot test functions with logging and results

Python (Project Side)

def test_led_blink():
    """Test: Blink LED 5 times"""
    for i in range(5):
        led.on()
        time.sleep(0.5)
        led.off()
        time.sleep(0.5)
    return {"status": "PASS", "message": "LED blinked 5 times successfully"}

def test_pump_pressure():
    """Test: Run pump and check pressure"""
    pump.start()
    time.sleep(2)
    pressure = pressure_sensor.read()
    pump.stop()
    
    if pressure > 50:
        return {"status": "PASS", "message": f"Pressure: {pressure} PSI (OK)"}
    else:
        return {"status": "FAIL", "message": f"Pressure: {pressure} PSI (Low)"}

def test_water_level():
    """Test: Measure water level"""
    level = tank_level.read() / 4095 * 100
    return {
        "status": "INFO",
        "message": f"Water level: {level:.1f}%",
        "data": {"level": level}
    }

pst = PSTApp("project.json")
pst.register_test("led_blink", test_led_blink, duration=5)
pst.register_test("pump_pressure", test_pump_pressure, duration=10)
pst.register_test("water_level", test_water_level, duration=3)

JSON (Project Configuration)

"tests": {
  "led_blink": {
    "label": "Blink LED Test",
    "description": "Flash the main LED 5 times to verify operation",
    "expected_duration": 5,
    "icon": "bulb"
  },
  "pump_pressure": {
    "label": "Pump Pressure Test",
    "description": "Run pump and verify pressure sensor readout",
    "expected_duration": 10,
    "icon": "gauge",
    "log_results": true  // Save results to test log
  },
  "water_level": {
    "label": "Water Level Check",
    "description": "Measure current tank water level",
    "expected_duration": 3,
    "icon": "droplet"
  }
}

What Happens on PST
PST shows Tests panel with 3 buttons
User taps "Blink LED Test":
PST sends: {"test": "led_blink", "action": "start"}
pstlib calls test_led_blink() → LED blinks 5 times
Returns: {"status": "PASS", "message": "LED blinked 5 times successfully"}
PST shows: ✅ PASS - LED blinked 5 times successfully
User taps "Pump Pressure Test":
Pump runs for 2s, pressure checked
If pressure > 50 PSI: ✅ PASS
If pressure ≤ 50 PSI: ❌ FAIL - Pressure too low
Results are logged to test history
✅ Result: Built-in diagnostic toolkit for field troubleshooting

Complete Example: Greenhouse System
Here's how all registers work together in a real scenario:

Python (Complete Project)

from machine import Pin, ADC, PWM
from pstlib import PSTApp
from dht import DHT22

# Hardware
pump = PWM(Pin(5), freq=1000)
fan = PWM(Pin(6), freq=1000)
led = Pin(2, Pin.OUT)
button = Pin(14, Pin.IN)
temp_sensor = DHT22(Pin(4))
moisture = ADC(Pin(35))
tank_level = ADC(Pin(36))

pst = PSTApp("project.json")

# Actions: User controls
pst.register_action("pump", pump.duty)
pst.register_action("fan", fan.duty)
pst.register_action("led", led)

# Telemetry: Sensor data with graphing
pst.register_telemetry("temperature", lambda: temp_sensor.temperature(), rate=2)
pst.register_telemetry("humidity", lambda: temp_sensor.humidity(), rate=2)
pst.register_telemetry("soil_moisture", lambda: moisture.read() / 4095 * 100, rate=1)
pst.register_telemetry("tank_level", lambda: tank_level.read() / 4095 * 100, rate=1)

# Inputs: Read-only hardware state
pst.register_input("button", lambda: button.value())
pst.register_input("pump_status", lambda: pump.duty() > 0)
pst.register_input("fan_status", lambda: fan.duty() > 0)

# Events: Alerts with logging
pst.register_event("low_water", lambda: tank_level.read() / 4095 * 100 < 20)
pst.register_event("overtemp", lambda: temp_sensor.temperature() > 35)
pst.register_event("dry_soil", lambda: moisture.read() / 4095 * 100 < 30)

# Files: Manage code on the device
pst.register_file("config", "config.json", readonly=False)
pst.register_file("logs", "alerts.log", readonly=True)

# Text: Status displays
pst.register_text("firmware", "Greenhouse v1.0.4")
pst.register_text("uptime", lambda: get_uptime_string())

# Tests: Diagnostics
pst.register_test("pump_test", test_pump_function, duration=5)
pst.register_test("sensor_calibration", test_sensors, duration=10)

pst.run_in_background()

# Normal project loop (unaffected by PST)
while True:
    # Your greenhouse logic here
    if temp_sensor.temperature() > 30:
        fan.duty(800)  # 80% fan speed
    time.sleep(0.1)

JSON (Project Configuration)
{
  "project_name": "Greenhouse Controller",
  "author": "DevName",
  "version": "1.0.4-beta",
  "PST_version": "1.0",
  
  "connection": { "protocol": "espnow", "fallback": ["wifi"] },
  
  "global_ui": {
    "theme": { "primary_color": "#228B22", "font": "medium" },
    "default_ui_mode": "auto"
  },
  
  "action": {
    "pump": { "type": "slider", "label": "Water Pump", "min": 0, "max": 1023, "unit": "PWM" },
    "fan": { "type": "slider", "label": "Ventilation Fan", "min": 0, "max": 1023, "unit": "PWM" },
    "led": { "type": "toggle", "label": "Status LED" }
  },
  
  "telemetry": {
    "temperature": { "type": "graph", "label": "Temperature", "unit": "°C", "refresh_rate": 2 },
    "humidity": { "type": "gauge", "label": "Humidity", "unit": "%", "refresh_rate": 2 },
    "soil_moisture": { "type": "number", "label": "Soil Moisture", "unit": "%", "refresh_rate": 1 },
    "tank_level": { "type": "number", "label": "Water Tank", "unit": "%", "refresh_rate": 1 }
  },
  
  "input": {
    "button": { "type": "indicator", "label": "Manual Button" },
    "pump_status": { "type": "indicator", "label": "Pump Running" },
    "fan_status": { "type": "indicator", "label": "Fan Running" }
  },
  
  "event": {
    "low_water": { "type": "alert", "message": "Water tank low!", "priority": "high" },
    "overtemp": { "type": "alert", "message": "Temperature critical!", "priority": "critical" },
    "dry_soil": { "type": "alert", "message": "Soil too dry!", "priority": "medium" },
    
    "log_water_alerts": {
      "type": "log",
      "trigger_event": "low_water",
      "data_points": ["%time", "%date", "tank_level", "pump", "message", "status"],
      "path": "/sd/logs/water.log"
    },
    "log_temp_alerts": {
      "type": "log",
      "trigger_event": "overtemp",
      "data_points": ["%timestamp", "temperature", "humidity", "fan", "message", "status"],
      "path": "/sd/logs/temperature.log"
    }
  },
  
  "files": {
    "config": { "label": "Configuration", "path": "config.json", "readonly": false },
    "logs": { "label": "Alert Logs", "path": "alerts.log", "readonly": true }
  },
  
  "text": {
    "firmware": { "label": "Firmware", "type": "text" },
    "uptime": { "label": "System Uptime", "type": "text", "refresh_rate": 1 }
  },
  
  "tests": {
    "pump_test": { "label": "Test Water Pump", "expected_duration": 5 },
    "sensor_cal": { "label": "Calibrate Sensors", "expected_duration": 10 }
  }
}

PST UI Generated Automatically

┌─ GREENHOUSE CONTROLLER ─────────────────┐
│ Firmware: Greenhouse v1.0.4-beta        │
│ Uptime: 5h 23m 47s                      │
├─────────────────────────────────────────┤
│ CONTROLS                                 │
│ [──●──────] Water Pump (512/1023)       │
│ [────●────] Ventilation Fan (650/1023)  │
│ [Toggle] Status LED                     │
├─────────────────────────────────────────┤
│ SENSORS                                  │
│ Temperature: 28.5°C  [Graph showing ▲]  │
│ Humidity: 65%  [Gauge: ████░░░░░░]     │
│ Soil Moisture: 42%                      │
│ Water Tank: 18%  ⚠️ LOW WATER!          │
├─────────────────────────────────────────┤
│ STATUS                                   │
│ Manual Button: Released                 │
│ Pump Running: Yes ✅                    │
│ Fan Running: No ❌                      │
├─────────────────────────────────────────┤
│ [Test Pump] [Calibrate Sensors]         │
│ [View Config] [Download Logs]           │
└─────────────────────────────────────────┘


 Everything generated from 7 register calls + 1 JSON file

Summary Table
Register	Type	Flow	Result
action	Write	User → PST → Project → Hardware	Hardware responds
telemetry	Read	Hardware → Project → PST → Graph	Live data visualization
input	Read	Hardware → Project → PST → Indicator	State mirrored on PST
event	Trigger	Condition → PST Alert + Log	Context-aware alerting
file	Manage	PST Editor ↔ Project Files	Edit code in field
text	Display	Project → PST → UI Label	Status display
test	Execute	User button → Test function → Results	Diagnostics
This is the complete ecosystem.






Steps to join the PST ecosystem:

Copy pstlib.py to project
Write project.json (declarative UI + connection config)
Add 3-6 register calls
Call run_in_background()
Done! PST discovers it, auto-generates UI, communication "just works"
Communication Protocol
Simple JSON over transport layer:

PST → Project (command):
{"cmd": "pump", "value": true}
{"cmd": "fan_speed", "value": 128}

Project → PST (telemetry/event):
{"telemetry": "temperature", "value": 23.5}
{"event": "low_water", "triggered": true}

Key Design Decisions in Your Approach
✅ Correct: PST firmware C++ = no script execution on device itself
✅ Correct: "Scripting Terminal" = scripting projects, not PST
✅ Correct: Edit mode = text editor + file manager, not Python runtime
✅ Correct: Prototype path = functional without MicroPython in PST firmware
✅ Correct: Custom UI in C++ for now (future: embedded MicroPython if needed)
✅ Correct: Projects stay 100% standalone MicroPython, pstlib is optional

# PST Firmware - Embedded Graphics Stack Documentation

### Key Hardware: JC3248W535EN
- **MCU**: ESP32-S3 (240 MHz dual-core)
- **Display**: 480x320 QSPI AXS15231B LCD with capacitive touch (320x480 after 90° rotation)
- **Memory**: 16MB QIO Flash + 8MB OPI PSRAM
- **I2C**: Touch panel on I2C_NUM_0 (GPIO 8 SCL, GPIO 4 SDA)
- **QSPI**: 4-lane parallel display interface (GPIO 21, 48, 40, 39 for data)

## Build & Development Workflow

### Build System
- **Tool**: PlatformIO with ESP-IDF framework (espressif32@6.7.0)
- **Environment**: `JC3248W535EN` defined in `platformio.ini`
- **Commands** (use PlatformIO CLI or VS Code):
  - Build: `pio run`
  - Build + Flash: `pio run -t upload`
  - Monitor Serial: `pio device monitor` (115200 baud)
  - Debug build info: `pio run -t size`
- **Board Config**: Custom JSON board definition at [boards/JC3248W535EN.json](boards/JC3248W535EN.json) - modify for new hardware variants
- **SDK Config**: [sdkconfig.defaults](sdkconfig.defaults) + [sdkconfig.JC3248W535EN](sdkconfig.JC3248W535EN) - PSRAM allocation and SPIRAM settings critical for LVGL

### Key Build Flags (platformio.ini)
```
-DCORE_DEBUG_LEVEL=3          # ESP-IDF debug verbosity
-DLV_CONF_INCLUDE_SIMPLE=1    # Use simplified LVGL config
-DBOARD_HAS_PSRAM=1           # Enable 8MB PSRAM
-DCONFIG_SPIRAM_SPEED=80m     # PSRAM speed matching CONFIG
```

## Architecture & Component Boundaries

### Startup Flow: app_main → BSP → LVGL Port
1. **app_main()** [src/app_main.c](src/app_main.c) - Entry point
   - Initializes `bsp_display_start_with_config()` with buffer rotation
   - Creates LVGL objects (buttons, labels) on main thread
   - Main loop: calls `lv_timer_handler()` every 5ms (200 FPS tick rate)
Project → PST (telemetry/event):


2. **ESP BSP Layer** [src/esp_bsp.c](src/esp_bsp.c) / [src/esp_bsp.h](src/esp_bsp.h)
   - **Hardware abstraction** for JC3248W535EN pinout - DO NOT hardcode GPIO elsewhere
   - Manages LCD initialization via SPI2_HOST (QSPI)
   - Handles touch panel I2C communication
   - Tear-effect synchronization (TE pin GPIO 38 for flicker-free display)
   - Backlight PWM control (GPIO 1, LEDC)
   - **Key functions**:
     - `bsp_display_start_with_config()` - complete LCD + LVGL setup
     - `bsp_display_lock/unlock()` - thread-safe LVGL access (mutex-based)
     - `bsp_display_backlight_on/off()` - power management

3. **LVGL Port Layer** [src/lv_port.c](src/lv_port.c) / [src/lv_port.h](src/lv_port.h)
   - **Graphics rendering engine integration**
   - Creates FreeRTOS task for LVGL timer tick (configured in `lvgl_port_cfg_t`)
   - Double-buffering with DMA-capable PSRAM allocations
   - Handles rotation at render time (`LV_DISP_ROT_90` for 90° landscape)
   - **Touch panel driver** integration (`esp_lcd_touch.c`) - reads capacitive input and passes to LVGL

4. **Display Driver** [src/esp_lcd_axs15231b.c](src/esp_lcd_axs15231b.c)
   - Vendor-specific AXS15231B LCD controller
   - Pre-initialized command sequences in `lcd_init_cmds[]` - **DO NOT modify without testing**
   - Supports RGB565 color format (16-bit little-endian)

### Error Handling Pattern
Use [src/bsp_err_check.h](src/bsp_err_check.h) macros throughout:
```c
#define BSP_ERROR_CHECK_RETURN_ERR(x)  // Graceful error return
#define BSP_NULL_CHECK(x, ret)          // Null pointer checks
```
Controlled by `CONFIG_BSP_ERROR_CHECK` in sdkconfig - enables assertions vs. silent returns.

## Key Patterns & Conventions

### 1. Pin Configuration
- **Single source of truth**: [src/pincfg.h](src/pincfg.h)
- Example: `#define TFT_SDA0 21` for QSPI data line 0
- I2C pins defined for touch controller (GPIO 8, 4)
- All GPIO enums as signed `-1` for NC (not connected) pins

### 2. LVGL Display Rotation
- Display physically rotated 90° → software compensates
- Set in `app_main()`: `rotate = LV_DISP_ROT_90`
- Touch coordinates auto-mapped by LVGL port
- Buffer size adjusted for rotated resolution (320×480 not 480×320)

### 3. PSRAM & Memory Management
- **Must use PSRAM for LVGL frame buffers** - only 80KB internal SRAM available
- Config in sdkconfig: `CONFIG_SPIRAM_USE_MALLOC=y`, `CONFIG_SPIRAM_MALLOC_ALWAYSINTERNAL=4096`
- 4KB malloc requests stay in internal RAM (stack/structures); larger allocations → PSRAM
- DMA buffers flagged with `buff_dma=1` in `lvgl_port_cfg_t` for zero-copy display transfers

### 4. Synchronization & Locking
- **Touchscreen & LVGL access**: Use `bsp_display_lock/unlock()` around shared state
- Example: Create UI elements inside `bsp_display_lock(0)` block in app_main
- FreeRTOS `SemaphoreHandle_t` manages vertical sync (TE pin) and touch interrupts

### 5. Task & Timer Management
- LVGL port spawns dedicated FreeRTOS task for rendering
- Main loop calls `lv_timer_handler()` - do NOT block this (5ms max yield)
- Async operations use `lv_timer_create()` or spawn separate FreeRTOS tasks with proper mutex locks

## Common Modifications

### Adding New GPIO/Sensor
1. Declare pin in [pincfg.h](src/pincfg.h)
2. Initialize in [esp_bsp.c](src/esp_bsp.c) via `gpio_config_t` or driver setup
3. Wrap access functions in bsp (e.g., `bsp_sensor_read()`) for abstraction

### Extending Display Functionality
- Register new LVGL widgets in app_main within `bsp_display_lock()` block
- For custom rendering: use LVGL canvas objects (`lv_canvas_*`) with PSRAM backing
- Rotation always applied transparently via LVGL port

### Serial Communication
- Monitor output: ESP_LOG macros (`ESP_LOGI`, `ESP_LOGE`)
- Set log level in platformio.ini: `-DCORE_DEBUG_LEVEL=3`
- Touch/LCD debug logs available at INFO level

## Project Dependencies
- **LVGL 8.3.11** - Graphics/UI library (from lvgl/lvgl in lib_deps)
- **ESP-IDF 5.x** - Core framework (via espressif32 platform)
- **FreeRTOS** - RTOS kernel bundled with IDF
- **Custom board definition**: [JC3248W535EN.json](boards/JC3248W535EN.json) extends PlatformIO's esp32s3 base

## Testing & Debugging Tips
- **Serial monitor** captures boot logs and runtime errors
- **LVGL demo buttons** in app_main verify LCD + touch working
- If display fails: check PSRAM init logs + QSPI pin voltages
- Touch not responding: verify I2C communication in [esp_bsp.c](src/esp_bsp.c) touch init
- Use `CONFIG_ESP_SYSTEM_PANIC_PRINT_HALT=y` (enabled) to capture panic details before reset

## File Organization
```
src/
├── app_main.c          # FreeRTOS entry point & UI demo loop
├── esp_bsp.[ch]        # Hardware abstraction & initialization
├── lv_port.[ch]        # LVGL task, rendering, memory management
├── esp_lcd_*.c         # Display controller driver (AXS15231B)
├── esp_lcd_touch.c     # Capacitive touch panel I2C driver
├── display.h           # LCD color format & resolution constants
├── pincfg.h            # GPIO pin definitions (source of truth)
├── bsp_err_check.h     # Error handling macros
├── lv_conf.h           # LVGL compile-time config
└── CMakeLists.txt      # Component source registration
```

High-Level Architecture Overview
This is a 4-layer embedded graphics stack architecture:
┌─────────────────────────────────────┐
│  app_main.c (Main Loop)             │ <- User application entry point
├─────────────────────────────────────┤
│  esp_bsp.c/h (Board Support)        │ <- Hardware abstraction layer
│  - LVGL initialization              │
│  - Display synchronization          │
│  - Touch integration                │
│  - Backlight PWM control            │
├─────────────────────────────────────┤
│  lv_port.c/h (LVGL Port)            │ <- Graphics rendering engine
│  - LVGL task management             │
│  - Buffer allocation (PSRAM)        │
│  - Rotation handling (90°/180°/270°)│
│  - Touch input driver               │
│  - Double-buffering with DMA        │
├─────────────────────────────────────┤
│  esp_lcd_*.c/h (Low-level Drivers)  │ <- Hardware-specific controllers
│  - AXS15231B LCD controller         │
│  - Touch panel I2C driver           │
│  - SPI/QSPI communication           │
└─────────────────────────────────────┘

Detailed File-by-File Breakdown
1. app_main.c (42 lines)
Purpose: FreeRTOS entry point, minimal demo application
Key Function: app_main()
Configures display with 90° rotation
Allocates LVGL buffer: 320×480 pixels = 153,600 bytes
Creates demo button + label
Main loop: calls lv_timer_handler() every 5ms (200 FPS tick rate)
Uses bsp_display_lock/unlock() for thread-safe LVGL access
Pattern: Very lean—delegates everything to BSP and LVGL layers
2. esp_bsp.c (540 lines) / esp_bsp.h (122 lines)
Purpose: Hardware abstraction + initialization orchestrator

Key Structures:

bsp_display_cfg_t: Display config (rotation, buffer size, LVGL settings)
bsp_lcd_tear_t: Tear effect sync (TE pin GPIO 38 → flicker-free rendering)
bsp_touch_int_t: Touch interrupt handling
Major Functions:

bsp_display_start_with_config(): Main entry point—initializes everything in sequence
Calls lvgl_port_init() → creates LVGL task + mutex
Calls bsp_display_brightness_init() → sets up PWM backlight (GPIO 1, LEDC)
Calls bsp_display_lcd_init() → QSPI LCD + tear sync + LVGL display driver
Calls bsp_display_indev_init() → I2C touch panel + LVGL input device
bsp_i2c_init(): Configures I2C for touch (400kHz)
bsp_display_brightness_set(): PWM control (0–100%)
bsp_display_new(): Low-level SPI2_HOST + AXS15231B panel setup
bsp_touch_new(): I2C touch controller initialization
bsp_touch_process_points_cb(): Rotation callback—adjusts touch coordinates for 90° rotation
Critical Implementation Details:

Tear Sync: Uses semaphores to synchronize LVGL rendering with display TE signal
bsp_display_tear_interrupt() (ISR) → signals TE event
bsp_display_sync_task() → delays flush until safe window
Touch Rotation: Remaps (X, Y) coordinates when display is rotated
Memory: PSRAM buffer 320×480×2 bytes = 307.2 KB (fits in 8MB PSRAM)
3. lv_port.c (623 lines) / lv_port.h (165 lines)
Purpose: LVGL library port (rendering, memory, task management)

Key Structures:

lvgl_port_ctx_t: Global context (mutex, timer, task state)
lvgl_port_display_ctx_t: Per-display (buffers, transport, DMA semaphores)
lvgl_port_touch_ctx_t: Touch driver context
Core Functions:

lvgl_port_init(): Creates LVGL task + recursive mutex (allows nested locks)
Task priority: 4, stack: 4096 bytes
Timer tick: 5ms period → 200 FPS
lvgl_port_add_disp(): Buffer allocation engine
Allocates main draw buffer from PSRAM (buff_spiram=1)
Allocates 2× transport buffers from DMA-capable RAM
Creates semaphore for DMA transfer completion
lvgl_port_flush_callback(): Complex rotation handler
Chunks frame into trans_size (1/10 screen = ~15KB slices)
Applies rotation matrices (90°, 180°, 270°, none)
Manages double-buffering: fills buffer A while DMA transfers buffer B
Handles tear sync wait callback
lvgl_port_task(): Main LVGL loop (runs on dedicated FreeRTOS task)
Locks mutex → calls lv_timer_handler() → yields based on next timer
Adaptive sleep: up to 500ms max
lvgl_port_touchpad_read(): Reads touch controller via I2C, applies transforms
Memory Architecture:

Internal SRAM (160 KB):
├── LVGL task stack (4 KB)
├── Mutex (small)
└── FreeRTOS kernel

PSRAM (8 MB):
├── Main LVGL buffer: 307 KB (full screen)
├── Transport buffer 1: ~15 KB (DMA-capable)
├── Transport buffer 2: ~15 KB (DMA-capable)
└── (future: MicroPython runtime, file buffers)

4. esp_lcd_axs15231b.c (551 lines) / esp_lcd_axs15231b.h
Purpose: AXS15231B LCD controller driver (vendor-specific)

Key Functions:

esp_lcd_new_panel_axs15231b(): Creates LCD panel object
Configures RGB565 format (16-bit, little-endian)
Sets up GPIO reset (NC—no reset pin)
Stores init command sequences
panel_axs15231b_init(): Sends 37 initialization commands to LCD
Commands: 0xBB, 0xA0, 0xA2, 0xD0... 0x22
Some commands include 200ms delays (e.g., 0x11 = sleep out, 0x29 = display on)
panel_axs15231b_draw_bitmap(): Sets LCD window + transfers color data
Sends CASET (column address) + RASET (row address)
Uses QSPI opcode 0x32 for color writes (fast parallel transfer)
esp_lcd_touch_new_i2c_axs15231b(): Configures touch IC
I2C address: 0x3B
Gesture detection, multi-touch support (max 1 point for PST)
Interrupt pin: GPIO 3 (negedge trigger)
Init Command Flow:

Sleep out (0x11) → 100ms
Memory access control (0x36) → RGB mode
Color mode (0x3A) → RGB565
Vendor-specific gamma curves (0xE0–0xE5)
Display on (0x29) → 200ms
5. esp_lcd_touch.c (267 lines) / esp_lcd_touch.h (437 lines)
Purpose: Generic touch controller abstraction
Key Functions:
esp_lcd_touch_read_data(): Reads raw touch IC data
esp_lcd_touch_get_coordinates(): Extracts (X, Y, strength), applies SW transforms
Supports: mirror_x, mirror_y, swap_xy (via bit flags or HW calls)
Calls user process_coordinates callback (used for rotation in PST)
Touch data structure: up to CONFIG_ESP_LCD_TOUCH_MAX_POINTS=1 point
6. display.h (139 lines)
Purpose: LCD display constants and configuration
Key Defines:
EXAMPLE_LCD_QSPI_H_RES = 320   // Physical width (after rotation)
EXAMPLE_LCD_QSPI_V_RES = 480   // Physical height
BSP_LCD_BITS_PER_PIXEL = 16    // RGB565
BSP_LCD_BIGENDIAN = 1          // 5-6-5 bit layout

Tear Config Macro: BSP_SYNC_TASK_CONFIG() for TE pin timing
7. pincfg.h (28 lines)
Purpose: Single source of truth for GPIO assignments
LCD Pins:
TFT_CS=45, TFT_SCK=47           // SPI clock + chip select
TFT_SDA0..3={21,48,40,39}       // QSPI data lines
TFT_DC=8, TFT_TE=38, TFT_BLK=1  // Data/clock, tear, backlight
TOUCH_PIN_NUM_I2C_SCL=8, SDA=4  // Shares GPIO 8 with LCD DC!
TOUCH_PIN_NUM_INT=3             // Interrupt
Other Peripherals: SD card, audio (I2S), battery ADC

8. lv_conf.h (771 lines)
Purpose: LVGL compile-time configuration
Critical Settings:
LV_COLOR_DEPTH = 16             // RGB565
LV_COLOR_16_SWAP = 1            // Byte swap for little-endian
LV_COLOR_SCREEN_TRANSP = 1      // Support transparency
LV_MEM_CUSTOM = 1               // Use malloc/free (→ PSRAM)
LV_DISP_DEF_REFR_PERIOD = 50ms
LV_INDEV_DEF_READ_PERIOD = 30ms
LV_DPI_DEF = 130                // DPI scaling (small touchscreen)

9. bsp_err_check.h (59 lines)
Purpose: Error handling macros
Macros:
BSP_ERROR_CHECK_RETURN_ERR(): Returns error code on fail (graceful)
BSP_NULL_CHECK(): Returns value if NULL (graceful)
Mode controlled by CONFIG_BSP_ERROR_CHECK in sdkconfig
10. CMakeLists.txt
Registers all source files as ESP-IDF component
Critical Data Flows
Display Rendering Pipeline (5ms cycle)
Main Loop (app_main)
  ↓ every 5ms
lv_timer_handler() [LVGL task]
  ↓
LVGL renders dirty regions to buffer
  ↓
lvgl_port_flush_callback()
  ↓
Chunks buffer by rotation (90°/180°/270°)
  ↓
Wait for TE signal (tear sync)
  ↓
esp_lcd_panel_draw_bitmap() [SPI2_HOST QSPI]
  ↓
AXS15231B LCD controller updates display RAM
  ↓
Physical LCD refreshes from RAM

Touch Input Pipeline (30ms polling)
LVGL task
  ↓
lvgl_port_touchpad_read()
  ↓
esp_lcd_touch_read_data() [I2C 400kHz]
  ↓
touch_axs15231b_read_data() [I2C transaction]
  ↓
I2C reads 8 bytes from AXS15231B (gesture, coordinates)
  ↓
bsp_touch_process_points_cb() [Rotation adjustment for 90°]
  ↓
esp_lcd_touch_get_coordinates() [SW transforms]
  ↓
LVGL input system processes (X, Y, state)

Key Design Patterns
Layered Abstraction: app_main → esp_bsp → lv_port → vendor drivers
Rotation Handling: Applied at 3 levels:
LVGL: LV_DISP_ROT_90 config
Buffer: pixel rearrangement in lvgl_port_flush_callback()
Touch: coordinate remapping in bsp_touch_process_points_cb()
Memory Efficiency: PSRAM used for large buffers, internal SRAM for structures
Synchronization: Tear effect + double-buffering prevents tearing
Thread Safety: All LVGL access via recursive mutex (bsp_display_lock/unlock())
Error Handling: Macro-based graceful errors (no assert crashes in production)
Critical Implementation Details to Remember
Aspect	Detail
Display Size	320×480 (rotated from native 480×320)
Color Format	RGB565, little-endian, QSPI 4-lane
Main Buffer	307 KB in PSRAM (full screen)
Transport Buffers	2× 15 KB in DMA RAM (DMA-to-LCD chunks)
Touch IC	AXS15231B @ I2C 0x3B (shares SCL with LCD DC!)
Tear Sync	GPIO 38, task-based software sync
Backlight	GPIO 1, PWM (LEDC channel 1, 5kHz)
Task Stack	LVGL: 4096 bytes, tear: 2048 bytes
Frame Rate	~200 FPS (5ms tick), but limited by display refresh (50ms)
LVGL Version	8.3.11