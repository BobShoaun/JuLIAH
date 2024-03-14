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
