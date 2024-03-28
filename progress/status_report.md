Bob Shoaun Ng
1006568992

- So far we have wifi connectivity and connections to a MQTT broker, and microphone recording capabilities. The next challenge is to combine the two functionalities. I spent a lot of time creating a mbed project from scratch [mbed/] which contains the combined code for wifi/MQTT and mic recording.
- Abstracted wifi/mqtt connectivity functionality into a header file for easy usage [mbed/source/juliah_mqtt.hpp]
- Combined audio functionality with wifi/mqtt functionality, which required a lot of code refactoring [mbed/source/main.cpp]
- Fixed bugs, allowing for multiple records, and having the program listen to the juliah/blink topic which also listen for button presses to record. [mbed/source/main.cpp]
- Problem encountered when attempting to send entire WAV audio buffer as one MQTT message, the test broker we are using does not allow for it. Therefore I had to break down the audio buffer into FILE_CHUNK_SIZE bytes, and do some complex synchronizations with preambles for each message for the receiver to know how to piece together all the MQTT messages into one WAV audio buffer. [mbed/source/main.cpp] [client/src/routes/+page.svelte]
- Experimented a bit with FILE_CHUNK_SIZE, settled at 100 bytes, but occasionally the broker may still reject messages for reasons unknown. [mbed/source/main.cpp]

[client/src/routes/+page.svelte]

- On the frontend side, displayed a connection status pill UI to show if the client is connected to the MQTT broker.
- Figured out how to deploy the frontend to Github Pages, key takeaways is that we need a .nojekyll file + use the gh-pages script to deploy + the base path for scripts needs to include /JuLIAH, the repo name.
- In production, since the site is HTTPs, we need to also use WSS (secure web socket) for the MQTT broker connection.
- Ensure playback is working for received WAV file buffer in the UI.
- LIVE LINK: https://bobshoaun.github.io/JuLIAH/
