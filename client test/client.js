import mqtt from "mqtt"; // import namespace "mqtt"
import fs from "fs";

const client = mqtt.connect("mqtt://test.mosquitto.org"); // create a client
// let client = mqtt.connect("mqtt://localhost"); // create a client

client.on("connect", () => {
  console.log("connecting");

  client.subscribe("juliah/sound", (err) => {
    if (err) {
      console.log("error subbing");
    }
  });
});

let wav_file = [];

client.on("message", (topic, message) => {
  // message is Buffer
  if (topic !== "juliah/sound") return;

  // console.log(topic.toString());
  // console.log(message);
  // console.log(message.readInt8(0));

  const preamble = message.readInt8(0);

  switch (preamble) {
    case 0: // beginning
      // console.log(preamble);
      // console.log("length", message.length);
      // console.log(message);
      console.log(message.subarray(1).toString());
      wav_file = [];
      break;
    case 1: // file itself
      // console.log(preamble);
      // console.log("length", message.length);
      // console.log(message);
      // console.log(message.subarray(1).toString());
      wav_file.push(message.subarray(1));
      break;
    case 2: // end
      const wav = Buffer.concat(wav_file);
      console.log(wav);

      const filename = "./audio.wav";
      fs.writeFileSync(filename, wav);
      console.log("Written to", filename);
      break;
  }

  // client.end();
});
