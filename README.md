# IoT Environment Monitoring Dashboard

## Project Overview

This project implements an end-to-end IoT monitoring system that collects temperature and humidity readings from a DHT22 sensor connected to an ESP32, transmits the readings over Wi-Fi using MQTT, and displays them on a live web dashboard.

It demonstrates embedded programming, network communication, offline buffering, reconnection handling, dashboard development, and basic power and security considerations.

## Features

- Reads temperature and humidity every 5 seconds.
- Transmits readings using MQTT over Wi-Fi.
- Displays current readings on a web dashboard.
- Shows recent readings and their received times.
- Retries Wi-Fi and MQTT connections.
- Buffers up to 20 readings in RAM during network outages.
- Publishes queued readings after reconnection.
- Keeps local configuration and credentials out of GitHub.

## Technologies Used

- ESP32
- DHT22 temperature and humidity sensor
- Arduino C++
- Wi-Fi and MQTT
- Node.js and Express.js
- Socket.IO
- HTML and CSS
- Wokwi simulator

## System Architecture

DHT22 Sensor → ESP32 → Wi-Fi → MQTT Broker → Node.js Server → Web Dashboard → User

See `system-diagram.mmd` for the system diagram.

## Hardware Connections

| DHT22 Pin | ESP32 Pin |
|---|---|
| VCC | 3V3 |
| DATA | GPIO 15 |
| GND | GND |
| NC | Not connected |

The firmware can be tested using Wokwi without physical hardware.

## Firmware Setup

1. Open [Wokwi](https://wokwi.com/).
2. Create a new ESP32 project.
3. Add a DHT22 sensor and connect it according to the table above.
4. Install the `DHT sensor library for ESPx` and `PubSubClient` libraries.
5. Copy `firmware/sketch.ino` into the Arduino editor.
6. Create a local `firmware/secrets.h` from `firmware/secrets.h.example`.
7. Start the simulation and open the Serial Monitor.

For Wokwi, use the `Wokwi-GUEST` Wi-Fi network.

## Dashboard Setup

Install Node.js, open a terminal, and run:

```bash
cd dashboard
npm install
```

Copy `.env.example` to `.env` in the dashboard directory. Check that the MQTT broker and topic match the firmware configuration.

Start the dashboard:

```bash
npm start
```

Open the dashboard in your browser:

http://localhost:3000

Keep the server running while the ESP32 simulation publishes readings.

## MQTT Configuration

- Broker: `broker.hivemq.com`
- Port: `1883`
- Topic: `aarthi-iot/task04/temperature`

The firmware and dashboard must use the same topic. Change the example topic to a unique one before testing.

The public broker is unauthenticated and uses unencrypted MQTT. It is intended for demonstration, not sensitive or production data.

## Reliability

The ESP32 periodically retries Wi-Fi and MQTT connections. When MQTT is disconnected, valid sensor readings are stored in a fixed-size RAM queue and published after reconnection.

The queue holds up to 20 readings. When it is full, the oldest reading is discarded to make room for a new one.

The dashboard server also retries its MQTT connection and keeps up to 50 recent readings in memory.

## Power Considerations

- Use a suitable USB power supply.
- Read the sensor at a reasonable interval.
- Avoid unnecessary continuous polling.
- Consider ESP32 sleep modes for battery-powered versions.
- Account for the energy consumed by Wi-Fi communication.

This version prioritizes continuous monitoring rather than battery optimization.

## Security and Secrets

- Do not upload `firmware/secrets.h`.
- Do not upload `dashboard/.env`.
- Never commit real Wi-Fi passwords or private tokens.
- Use authenticated MQTT with TLS for a real deployment.

The public demonstration broker does not provide private, authenticated communication.

## Testing Evidence

Add screenshots showing:

1. ESP32 and DHT22 wiring.
2. Sensor readings in the Serial Monitor.
3. Successful MQTT publishing.
4. Temperature and humidity on the dashboard.
5. Offline buffering and recovery after reconnection.

Save screenshots in an `evidence/` folder.

## Limitations

- The firmware queue is in RAM and is cleared after restart or power loss.
- The queue discards its oldest item when full.
- Successful MQTT publishing does not guarantee that every subscriber received the message.
- Dashboard history is lost when the server restarts.
- The public MQTT broker may be unavailable and is not intended for production use.

## Conclusion

This project demonstrates a complete IoT data pipeline from sensor readings to a human-readable dashboard, including basic network recovery, offline buffering, and power and security considerations.
