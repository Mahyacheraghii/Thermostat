import { useEffect, useRef, useState } from "react";
import "./App.css";
import "./index.css";
import { FiDroplet, FiPower } from "react-icons/fi";
import { FaFan } from "react-icons/fa";
import ArcDisplay from "./components/ArcDisplay";
import { LuLink, LuLink2Off } from "react-icons/lu";
import { createMqttClient } from "./services/mqttClient";

export default function App() {
  const [temp, setTemp] = useState(23);
  const [humidity, setHumidity] = useState(41);
  const [setpoint, setSetpoint] = useState(23);
  const [fanSpeed, setFanSpeed] = useState("off");
  const [fanDirection, setFanDirection] = useState("up");
  const [isPumpOn, setIsPumpOn] = useState(false);
  const [isPowerOn, setIsPowerOn] = useState(true);
  const [mqttStatus, setMqttStatus] = useState("disconnected");
  const [deviceStatus, setDeviceStatus] = useState("unknown");
  const mqttRef = useRef(null);
  const hasFanSpeedTelemetry = useRef(false);
  const MQTT_URL = import.meta.env.VITE_MQTT_URL;
  const MQTT_HOST = import.meta.env.VITE_MQTT_HOST;
  const MQTT_PORT = import.meta.env.VITE_MQTT_PORT;
  const MQTT_USE_TLS = import.meta.env.VITE_MQTT_USE_TLS;
  const MQTT_PATH = import.meta.env.VITE_MQTT_PATH || "/mqtt";
  const MQTT_USER = import.meta.env.VITE_MQTT_USER;
  const MQTT_PASS = import.meta.env.VITE_MQTT_PASS;
  const BASE_TOPIC = import.meta.env.VITE_MQTT_BASE_TOPIC || "thermostat";
  const derivedUrl =
    MQTT_URL ||
    (MQTT_HOST && MQTT_PORT
      ? `${MQTT_USE_TLS === "1" || MQTT_USE_TLS === "true" ? "wss" : "ws"}://${MQTT_HOST}:${MQTT_PORT}${MQTT_PATH}`
      : "ws://localhost:9001");

  useEffect(() => {
    const telemetryPrefix = `${BASE_TOPIC}/telemetry/`;
    const mqttClient = createMqttClient({
      url: derivedUrl,
      baseTopic: BASE_TOPIC,
      username: MQTT_USER,
      password: MQTT_PASS,
      onStatus: (status) => setMqttStatus(status),
      onMessage: (topic, msg) => {
        if (!topic.startsWith(telemetryPrefix)) return;
        const key = topic.slice(telemetryPrefix.length);
        if (key === "temp_c") setTemp(Number(msg));
        else if (key === "humidity") setHumidity(Number(msg));
        else if (key === "setpoint_c") setSetpoint(Number(msg));
        else if (key === "fan_speed") {
          hasFanSpeedTelemetry.current = true;
          if (msg === "2") {
            setFanSpeed("fast");
            setFanDirection("down");
          } else if (msg === "1") {
            setFanSpeed("slow");
            setFanDirection("up");
          } else {
            setFanSpeed("off");
            setFanDirection("up");
          }
        }
        else if (key === "status") {
          setDeviceStatus(msg);
        }
        else if (key === "state") {
          setIsPowerOn(msg !== "off");
          if (!hasFanSpeedTelemetry.current) {
            if (msg === "fan_only") {
              setFanSpeed("slow");
            } else if (msg === "heating" || msg === "cooling") {
              setFanSpeed("fast");
              setFanDirection("down");
            } else {
              setFanSpeed("off");
              setFanDirection("up");
            }
          }
          if (msg === "prestart" || msg === "cooling") {
            setIsPumpOn(true);
          } else if (msg === "off") {
            setIsPumpOn(false);
          }
        } else if (key === "pump_desired") {
          setIsPumpOn(msg === "1");
        }
      },
    });
    mqttRef.current = mqttClient;

    return () => {
      mqttClient.disconnect();
    };
  }, [BASE_TOPIC, MQTT_PASS, MQTT_USER, derivedUrl]);

  const publishCommand = (cmd, payload) => {
    const client = mqttRef.current;
    if (!client) return;
    client.publishCommand(cmd, payload);
  };

  return (
    <div className=" w-full text-white flex flex-col items-center justify-between px-2 space-y-47">
      {/* wifi + settings */}
      <div className="flex items-center justify-between w-full ">
        {mqttStatus === "connected" && deviceStatus !== "offline" && (
          <LuLink size={28} color="#22c55e" />
        )}
        {mqttStatus === "connecting" && (
          <LuLink2Off size={28} color="#f59e0b" />
        )}
        {mqttStatus === "error" && <LuLink2Off size={28} color="#ef4444" />}
        {(mqttStatus === "disconnected" || deviceStatus === "offline") && (
          <LuLink2Off size={28} color="#94a3b8" />
        )}
      </div>

      {/* Arc and data */}
      <div className="flex flex-col items-center justify-center w-full">
        <ArcDisplay
          value={setpoint}
          min={15}
          max={30}
          humidity={humidity}
          currentTemp={temp}
          onChange={(value) => {
            setSetpoint(value);
            publishCommand("setpoint", value);
          }}
        />
      </div>
      {/* Controls */}
      <div className="flex flex-row items-center justify-between w-full px-7">
        <FiDroplet
          size={28}
          className={`${
            isPowerOn
              ? isPumpOn
                ? "text-blue-200 animate-pulse"
                : "text-white"
              : "text-white"
          }`}
        />
        <FaFan
          size={28}
          className={`${
            isPowerOn
              ? fanSpeed === "slow"
                ? "text-blue-200 animate-[spin_6s_linear_infinite]"
                : fanSpeed === "fast"
                  ? "text-blue-200 animate-[spin_2s_linear_infinite]"
                  : "text-white"
              : "text-white"
          }`}
        />
        <FiPower
          size={28}
          className={`${
            isPowerOn ? "text-red-400" : "text-white"
          } cursor-pointer`}
          onClick={() => {
            if (isPowerOn) {
              publishCommand("power", "0");
              setIsPowerOn(false);
            } else {
              publishCommand("power", "1");
              setIsPowerOn(true);
            }
          }}
        />
      </div>
    </div>
  );
}
