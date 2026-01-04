import mqtt from "mqtt";

export function createMqttClient({
  url,
  baseTopic,
  username,
  password,
  onMessage,
  onStatus,
}) {
  const client = mqtt.connect(url, {
    clientId: "thermostat-web-" + Math.random().toString(16).slice(2, 10),
    keepalive: 30,
    reconnectPeriod: 1000,
    username,
    password,
  });

  const notifyStatus = (status) => {
    if (onStatus) onStatus(status);
  };

  notifyStatus("connecting");

  client.on("connect", () => {
    notifyStatus("connected");
    console.info("[mqtt] connected");
    client.subscribe(`${baseTopic}/telemetry/#`);
  });

  client.on("close", () => {
    notifyStatus("disconnected");
    console.info("[mqtt] disconnected");
  });
  client.on("offline", () => {
    notifyStatus("disconnected");
    console.info("[mqtt] offline");
  });
  client.on("error", (err) => {
    notifyStatus("error");
    console.error("[mqtt] error", err);
  });

  client.on("message", (topic, payload) => {
    const message = payload.toString();
    console.debug("[mqtt] recv", topic, message);
    if (onMessage) onMessage(topic, message);
  });

  const publishCommand = (cmd, payload) => {
    if (!client.connected) return;
    const topic = `${baseTopic}/cmd/${cmd}`;
    const message = String(payload);
    console.debug("[mqtt] send", topic, message);
    client.publish(topic, message, {
      retain: false,
    });
  };

  const disconnect = () => {
    client.end(true);
  };

  return { publishCommand, disconnect };
}
