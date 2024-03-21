Bob Shoaun Ng
1006568992

- With Dian, we asked other classmates if they have managed to implement wifi connectivity. We found that WIFI is able to be implemented relatively easily, compared to what we had thought. Moreover, there is already a starting template for sockets and wifi in Mbed IDE.
- Using the starting template and abit of changes, we were able to connect to WiFi via my phone's hotspot. [test-sockets/main.cpp]
- Dian went on to figure out how to connect to a MQTT broker, and I helped her troubleshoot.
- We managed to send and receive MQTT packets to and from a public testing MQTT broker (test.mosquitto.org) [test-sockets/main.cpp]

- I started work on writing the HTML and CSS (using a library called Tailwindcss) for the web client. [+page.svelte, SoundRecording.svelte]
- Realized that Astro might not have been the best framework for our use case, it is more fitted for SSG (static site generated) or SSR (server side rendered) sites, while what we need is a single page application. Therefore I switched frameworks to Svelte. [client/]
- The user interface displays a list of sound recordings, and a big red button that says "distract". The sound recordings can be individually played back, paused, stopped, etc. The sound recordings are hooked up to the subscribed MQTT topic juliah/sound, and the big red button is hooked up to publish the juliah/blink topic. [+page.svelte]
- Made sure UI is responsive so it can be used in phones and large displays. [+page.svelte]
- Ability to remove sound recordings (delete button)
