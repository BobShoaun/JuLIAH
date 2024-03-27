<script>
  import "../app.css";
  import SoundRecording from "./SoundRecording.svelte";
  import { onMount } from "svelte";

  import mqtt from "mqtt";

  let client;
  const blinkTopic = "juliah/blink";
  const soundTopic = "juliah/sound";

  const brokerURL = "ws://test.mosquitto.org:8080";
  // const brokerURL = "ws://localhost:8080";

  let connectionState = "pending";

  let soundRecordings = [];
  let wav_file = [];

  onMount(async () => {
    try {
      client = mqtt.connect(brokerURL); // create a client
    } catch (e) {
      console.log("MQTT connection FAILED");
      connectionState = "offline";
      return;
    }

    client.on("offline", (err) => {
      connectionState = "offline";
      console.log("MQTT offline:", err);
    });

    client.on("connect", () => {
      console.log("Connected to MQTT Broker");
      connectionState = "connected";

      client.subscribe(soundTopic, (err) => {
        if (err) console.log("Error subscribing!", err);
      });
    });

    client.on("message", (topic, message) => {
      if (topic !== "juliah/sound") return;

      const preamble = message.readInt8(0);

      switch (preamble) {
        case 0: // beginning
          console.log(message.subarray(1).toString());
          const header = message.subarray(1).toString();

          let headerJSON;
          try {
            headerJSON = JSON.parse(header);
          } catch (e) {
            console.log("Invalid JSON header, discarding.", e);
            return;
          }

          const recording = {
            // timestamp: jsonMessage.timestamp,
            timestamp: Date.now(),
            // peakVolume: jsonMessage.peakVolume,
            peakVolume: Math.random() * 100,
            audio:
              headerJSON.audio ||
              "https://www2.cs.uic.edu/~i101/SoundFiles/StarWars60.wav",
          };
          soundRecordings = [recording, ...soundRecordings];

          wav_file = [];
          break;
        case 1: // file itself
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

      // let jsonMessage;
      // try {
      //   jsonMessage = JSON.parse(message.toString());
      // } catch (e) {
      //   console.log("Found invalid JSON message, discarding.", e);
      //   return;
      // }

      // console.log(topic);

      // const recording = {
      //   // timestamp: jsonMessage.timestamp,
      //   timestamp: Date.now(),
      //   // peakVolume: jsonMessage.peakVolume,
      //   peakVolume: Math.random() * 100,
      //   audio:
      //     jsonMessage.audio ||
      //     "https://www2.cs.uic.edu/~i101/SoundFiles/StarWars60.wav",
      // };

      // soundRecordings = [recording, ...soundRecordings];
      // client.end();
    });
  });

  const distract = () => {
    if (!client) return;
    console.log("Sending Blink topic");
    client.publish(blinkTopic, "blink the led now!");
  };

  const deleteSoundRecording = (index) => {
    soundRecordings = soundRecordings.filter((_, i) => i !== index);
  };

  const getConnectionColor = (connectionState) => {
    switch (connectionState) {
      case "connected":
        return "text-green-500";
      case "offline":
        return "text-red-500";
      default:
        return "text-yellow-500";
    }
  };

  const getConnectionText = (connectionState) => {
    switch (connectionState) {
      case "connected":
        return "Connected";
      case "offline":
        return "Offline";
      default:
        return "Connecting";
    }
  };
</script>

<main
  class="bg-gray-100 p-5 lg:py-14 lg:px-10 h-screen
    grid lg:grid-cols-[3fr_minmax(10em,1fr)] lg:grid-rows-[auto_1fr]
    grid-cols-1 grid-rows-[auto_1fr_auto]
    gap-x-10 gap-y-5"
>
  <header class="lg:col-span-2 grid grid-cols-subgrid items-center">
    <h1 class="text-2xl lg:text-4xl font-bold text-gray-900">
      JuLIAH <span class="text-gray-600 font-light">Just Leave It At Home</span>
    </h1>
    <button
      title={"connected to " + brokerURL}
      class="rounded-full shadow-md px-3 pl-8 py-2 relative mx-auto"
      ><span
        class={`${getConnectionColor(connectionState)} text-2xl absolute top-0 left-3`}
        >●</span
      >
      <p class="text-sm text-gray-600">{getConnectionText(connectionState)}</p>
    </button>
  </header>

  <section class="h-full flex flex-col overflow-hidden">
    <h2 class="text-xl font-medium">Sound Recordings</h2>
    <p class="mt-1 text-gray-700">
      Sounds heard beyond a certain volume will be recorded and shown here.
    </p>

    <ul
      class="mt-5 overflow-auto -ml-4 py-3 flex flex-col basis-full
    border-y-2 border-gray-300 gap-1"
    >
      {#each soundRecordings as soundRecording, i (soundRecording.timestamp)}
        <li key={soundRecording}>
          <SoundRecording
            {soundRecording}
            onDelete={() => deleteSoundRecording(i)}
          />
        </li>
      {/each}

      {#if !soundRecordings.length}
        <p class="m-auto pr-4 text-gray-500">
          No sound recordings at the moment...
        </p>
      {/if}
    </ul>
  </section>

  <section class="flex flex-col self-center">
    <button
      id="blink"
      on:click={distract}
      class="bg-red-500 rounded-xl block lg:aspect-square p-5
        shadow-red-300 shadow-2xl text-white font-bold uppercase text-xl lg:text-4xl
        hover:bg-red-600 focus:bg-red-600 active:bg-red-800 transition-all
        "
    >
      Distract
    </button>
    <p class="text-center mt-3 lg:mt-5 text-gray-700">
      Press the big red button above to blink the LED on the device and cause a
      distraction!
    </p>
  </section>
</main>
