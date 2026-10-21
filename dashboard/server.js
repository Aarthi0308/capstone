
const express = require("express");
const http = require("http");
const mqtt = require("mqtt");
const { Server } = require("socket.io");
const path = require("path");
require("dotenv").config();

const app = express();
const server = http.createServer(app);
const io = new Server(server);

const PORT = process.env.PORT || 3000;
const MQTT_URL =
  process.env.MQTT_URL || "mqtt://broker.hivemq.com:1883";
const MQTT_TOPIC =
  process.env.MQTT_TOPIC || "aarthi-iot/task04/temperature";

const history = [];
const MAX_HISTORY = 50;

app.use(express.static(path.join(__dirname, "public")));

const client = mqtt.connect(MQTT_URL, {
  reconnectPeriod: 3000,
  connectTimeout: 10000
});

client.on("connect", () => {
  console.log("Connected to MQTT broker");

  client.subscribe(MQTT_TOPIC, (err) => {
    if (err) {
      console.error("Subscription failed:", err.message);
    } else {
      console.log("Subscribed to:", MQTT_TOPIC);
    }
  });

  io.emit("connection-status", "connected");
});

client.on("reconnect", () => {
  console.log("Reconnecting to MQTT...");
});

client.on("offline", () => {
  console.log("MQTT client offline");
  io.emit("connection-status", "offline");
});

client.on("error", (err) => {
  console.error("MQTT error:", err.message);
});

client.on("message", (topic, payload) => {
  if (topic !== MQTT_TOPIC) return;

  try {
    const reading = JSON.parse(payload.toString());

    if (
      typeof reading.temperature !== "number" ||
      typeof reading.humidity !== "number" ||
      !Number.isFinite(reading.temperature) ||
      !Number.isFinite(reading.humidity)
    ) {
      console.log("Invalid reading ignored");
      return;
    }

    const item = {
      temperature: reading.temperature,
      humidity: reading.humidity,
      uptime_ms: Number.isFinite(reading.uptime_ms)
        ? reading.uptime_ms
        : null,
      receivedAt: new Date().toISOString()
    };

    history.push(item);

    if (history.length > MAX_HISTORY) {
      history.shift();
    }

    io.emit("reading", item);
    io.emit("history", history);

    console.log("Reading received:", item);
  } catch (err) {
    console.error("Invalid MQTT message:", err.message);
  }
});

io.on("connection", (socket) => {
  socket.emit("history", history);
  socket.emit(
    "connection-status",
    client.connected ? "connected" : "offline"
  );
});

server.listen(PORT, () => {
  console.log(`Dashboard running at http://localhost:${PORT}`);
});
