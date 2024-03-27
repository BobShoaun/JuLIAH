# JuLIAH

Group Project for CSC385

# Mosquitto Broker

## run on WSL

mosquitto

## run using config

mosquitto -c /etc/mosquitto/mosquitto.conf

### subscribe to test topic (server)

mosquitto_sub -h localhost -t test

### publish message to test topic (client)

mosquitto_pub -h localhost -t test -m "hello world"

# Mosquitto Client (Javascript)

### run client

cd client-web
npm run dev

# Testing

### publish mqtt for sound topic

mosquitto_pub -h test.mosquitto.org -t juliah/sound -m "{ \"timestamp\":10000000, \"peakVolume\": 43.4, \"audio\": \"\" }"

### publish mqtt for blink topic 

mosquitto_pub -h test.mosquitto.org -t juliah/blink -m "BLINK MESSAGE"

### online mqtt client

https://testclient-cloud.mqtt.cool/

## UI todos

- waveform visualization for sound recordings
- use card like ui for each sound
- notifications (push)
- LED blinker UI for feedback in app
