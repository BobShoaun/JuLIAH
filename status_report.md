Bob Shoaun Ng
1006568992

For this milestone, my goal was to set up the Mosquitto MQTT broker. With the help of this article [https://medium.com/gravio-edge-iot-platform/how-to-set-up-a-mosquitto-mqtt-broker-securely-using-client-certificates-82b2aaaef9c8] I was able to get Mosquitto downloaded and running on my Ubuntu WSL instance on Windows. However, there were many difficulties and road blocks I faced when trying to get things working.

After downloading the Mosquitto broker, most tutorials use `systemctl` to run the broker as a daemon. However I did not need that and my use case was to just run and test it locally for development, and deploy it on a server towards the end. Moreover, systemctl is not available on WSL Ubuntu for some reason. The way to run it was just using the command `mosquitto`, with that running I can run `mosquitto_sub -h localhost -t test` to subscribe to the topic "test" on another terminal, then run `mosquitto_pub -h localhost -t test -m "hello world"` to send a hello world to the subscriber on yet another terminal. This all works, but my attempts to communicate with the broker running in WSL with Windows was not working. I have made sure the ports were forwarded and correct, but trying to communicate with the broker via a web client was giving the wrong protocol for some reason.

Nevertheless, I started working on the frontend (using Astro), which uses a javascript MQTT client library to communicate with the broker. Surprisingly, it works, I was able to subscribe and publish to the broker using the MQTT protocol running in node.js. However, I needed it to run not in node.js but in the browser's runtime, which is incompatible with the MQTT protocol, therefore I had to use the web socket protocol instead.

Getting Mosquitto to expose a web socket port is simple in theory, it just needed a few lines added to the mosquitto.conf file. But the problem was when running the broker via the `mosquitto` command, it does not use the mosquitto.conf file. I have to specify it like this: `mosquitto -c /etc/mosquitto/mosquitto.conf`. But then there posses another issue where there was a permissions error when editing the pid file. After hours of debugging, I realized I can just comment the reference to that file out in the config file, and that makes it work (hopefully this doesn't bite me in the ass next time).

So that was most of the tinkering. The web client created using Astro has a "Blink LED" button to which when pressed sends a MQTT message with the topic "blink" to the broker via web sockets on port 8080. Then, a subscriber subscribed to the "blink" topic will receive the message.

Here are the notable files I have contributed to:

- mosquitto.conf (Mosquitto config file)
- index.astro (frontend page with html, css, js)
- README.md (documents how to run broker and client)

Do note that there are many other files that I have indirectly contributed to or is scafolded by Astro.
