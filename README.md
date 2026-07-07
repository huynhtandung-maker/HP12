# HP12 - Workspace Comfort IoT Device

HP12 is an ESP32-based IoT device for monitoring workspace comfort in real time.

The device measures temperature, humidity, heat index and relative light level, then sends telemetry to ThingsBoard for dashboard tracking. It also provides local feedback through OLED display, LEDs, buzzer and physical buttons.

## Core Purpose

HP12 helps answer one practical question:

> Is this workspace supporting or hurting deep focus right now?

## Hardware

- ESP32 / ESP32-U
- DHT22 temperature & humidity sensor
- LDR light sensor module
- OLED 0.96 inch SSD1306 I2C
- Green / Yellow / Red status LEDs
- Buzzer
- Navigation button
- Advice button

## Main Features

- Real-time temperature and humidity reading
- Heat Index / perceived temperature estimation
- Relative light level monitoring
- Focus proxy score
- OLED local dashboard
- LED and buzzer alert patterns
- ThingsBoard MQTT telemetry
- Non-blocking WiFi/MQTT reconnect
- Force first telemetry sync after connection
- Secrets separated from GitHub

## ThingsBoard Telemetry

HP12 sends:

- temperature
- humidity
- heatIndex
- lightPercent
- focusScore
- comfortState
- thermalState
- lightState
- action
- alarmProfile
- alarmMuted
- uptimeSec
- wifiRssi
- freeHeap

## File Structure

```text
HP12/
├─ HP12.ino
├─ config.h
└─ secrets.example.h
