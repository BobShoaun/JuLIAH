<script>
  import "../app.css";
  import SoundRecording from "./SoundRecording.svelte";
  import { onMount } from "svelte";

  import mqtt from "mqtt";

  let soundRecordings = [];

  let client;
  const blinkTopic = "juliah/blink";
  const soundTopic = "juliah/sound";

  onMount(() => {
    // const client = mqtt.connect("ws://localhost:8080"); // create a client
    client = mqtt.connect("ws://test.mosquitto.org:8080"); // create a client

    client.on("connect", () => {
      console.log("Connected to MQTT Broker");

      client.subscribe(soundTopic, (err) => {
        if (err) console.log("Error subscribing!", err);
      });
    });

    client.on("message", (topic, message) => {
      // message is Buffer
      console.log(message.toString());

      let jsonMessage;
      try {
        jsonMessage = JSON.parse(message.toString());
      } catch (e) {
        console.log("Found invalid JSON message, discarding.");
        return;
      }

      console.log(topic);

      // { \"timestamp\": 100000000, \"peakVolume\": 43.4, \"audio\": \"\" }
      const recording = {
        // timestamp: jsonMessage.timestamp,
        timestamp: Date.now(),
        // peakVolume: jsonMessage.peakVolume,
        peakVolume: Math.random() * 100,
        audio:
          jsonMessage.audio ||
          "https://www2.cs.uic.edu/~i101/SoundFiles/StarWars60.wav",
      };

      soundRecordings = [recording, ...soundRecordings];
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
</script>

<main
  class="bg-gray-100 p-5 lg:py-14 lg:px-10 h-screen
    grid lg:grid-cols-[3fr_minmax(10em,1fr)] lg:grid-rows-[auto_1fr]
    grid-cols-1 grid-rows-[auto_auto]
    gap-x-10 gap-y-5"
>
  <h1 class="text-2xl lg:text-5xl font-bold lg:col-span-2 text-gray-900">
    JuLIAH <span class="text-gray-600 font-light">Just Leave It At Home</span>
  </h1>

  <section class="h-full flex flex-col overflow-hidden">
    <h2 class="text-xl font-medium">Sound Recordings</h2>
    <p class="mt-1 text-gray-700">
      Sounds heard beyond a certain volume will be recorded and shown here.
    </p>

    <ul
      class="mt-5 overflow-auto -ml-4 py-3 flex flex-col basis-full
    border-y-2 border-gray-300"
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
    <p class="text-center mt-2 lg:mt-5 text-gray-700">
      Press the big red button above to blink the LED on the device and cause a
      distraction!
    </p>
  </section>
</main>
