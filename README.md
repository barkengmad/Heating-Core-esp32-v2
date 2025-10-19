# Heating Core ESP32 v2

ESP32-based heating control system with web interface, MQTT integration, and temperature monitoring.

## Features

- **Temperature Monitoring**: DS18B20 sensors for tanks, radiators, and underfloor heating
- **Web Interface**: Modern responsive UI for configuration and monitoring
- **MQTT Integration**: Home Assistant discovery and control
- **Dual Heat Sources**: Boiler and immersion heater control
- **Time Windows**: Configurable day/night heating periods
- **Persistent Settings**: All settings saved to flash memory

## Setup

### 1. Clone and Configure

```bash
git clone https://github.com/barkengmad/Heating-Core-esp32-v2.git
cd Heating-Core-esp32-v2
```

### 2. WiFi Configuration

Copy the template config file and add your credentials:

```bash
cp src/config.h src/config_secret.h
```

Edit `src/config_secret.h` with your WiFi credentials:

```cpp
const char* WIFI_SSID = "YourWiFiName";
const char* WIFI_PASSWORD = "YourWiFiPassword";
```

**Important**: `config_secret.h` is gitignored and will not be committed to version control.

### 3. MQTT Configuration

Update the MQTT broker settings in `src/config_secret.h`:

```cpp
const char* MQTT_HOST = "192.168.1.10";  // Your MQTT broker IP
const uint16_t MQTT_PORT = 1883;
```

### 4. Build and Upload

Using PlatformIO:

```bash
pio run --target upload
```

Or using the PlatformIO IDE.

## Hardware

- ESP32-S2-Saola-1 board
- DS18B20 temperature sensors
- Relay outputs for boiler, immersion, pumps, and valves
- 24V transformer control for zone valves

## Web Interface

Access the web interface at `http://heating.local` or the device's IP address.

### Pages

- **Main**: Temperature display, setpoint configuration, time windows
- **Config**: Hardware status, sensor enumeration, output states

## MQTT Topics

The device publishes to Home Assistant using MQTT discovery:

- Sensors: `heating-core-s2/sensor/*/state`
- Switches: `heating-core-s2/switch/*/state`
- Commands: `heating-core-s2/switch/*/set`

## Configuration

### Day/Night Windows

- Set different temperatures and heat sources for day/night periods
- Day window has priority over night window if they overlap
- Outside active windows, heating is disabled

### Heat Sources

- **Boiler**: Gas/oil boiler control
- **Immersion**: Electric immersion heater
- Each window can use either source independently

## Future Development

### Planned Features

- **Boost Controls**: Add boost buttons to run radiators for 1 hour
- **Underfloor Boost**: Add boost functionality for underfloor heating zones
- **Heat Exchange Control**: Relay control with timed boost options (15m, 30m, 1h, 2h)
- **Room Temperature Monitoring**: Add room thermometer via RS485 and local Arduino
- **Individual Radiator Control**: Per-radiator zone control

## Troubleshooting

1. **WiFi Connection**: Check credentials in `config_secret.h`
2. **MQTT Issues**: Verify broker IP and port settings
3. **Temperature Readings**: Check sensor connections and addresses in Config page
4. **Heating Not Working**: Verify time windows and heat source selection

## Development

The codebase is organized into modules:

- `main.cpp`: Setup, WiFi, web server, main loop
- `control.cpp`: Heating control logic
- `web_pages.cpp`: Web interface HTML generation
- `mqtt.cpp`: MQTT communication
- `globals.cpp`: Shared state and preferences
- `pins.h`: Hardware pin definitions
