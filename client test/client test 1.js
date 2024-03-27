import mqtt from "mqtt"; // import namespace "mqtt"
const client = mqtt.connect("mqtt://test.mosquitto.org"); // create a client
// let client = mqtt.connect("mqtt://localhost"); // create a client

client.on("connect", () => {
  console.log("connecting");

  client.subscribe("presence", (err) => {
    if (!err) {
      console.log("publishing");
      client.publish("presence", "Hello mqtt fdfd");
    }
  });
});

client.on("message", (topic, message) => {
  // message is Buffer
  console.log(message.toString());
  client.end();
});
