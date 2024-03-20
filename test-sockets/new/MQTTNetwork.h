#ifndef _MQTTNETWORK_H_
#define _MQTTNETWORK_H_

#include "NetworkInterface.h"
// #include "../mbed-os/connectivity/netsocket/include/netsocket/Socket.h"
#include "TCPSocket.h"

class MQTTNetwork {
public:
    MQTTNetwork(NetworkInterface* aNetwork) : network(aNetwork) {
        socket = new TCPSocket();
    }

    ~MQTTNetwork() {
        delete socket;
    }

    int read(unsigned char* buffer, int len, int timeout) {
        return socket->recv(buffer, len);
    }

    int write(unsigned char* buffer, int len, int timeout) {
        return socket->send(buffer, len);
    }

    int connect(const char* hostname, int port) {
        // socket->open(network);

        // // changed
        // SocketAddress address(hostname, port);
        // return socket->connect(address);

        // New from https://github.com/ARMmbed/mbed-mqtt/blob/master/src/MQTTNetwork.h
        int ret = NSAPI_ERROR_OK;
        if ((ret = socket->open(network)) != NSAPI_ERROR_OK) {
            return ret;
        }
        SocketAddress addr;
        // if (network->gethostbyname(hostname, &addr) != NSAPI_ERROR_OK) {
        //     return NSAPI_ERROR_DNS_FAILURE;
        // }
        addr.set_ip_address(hostname);
        addr.set_port(port);
        return socket->connect(addr);
    }

    int disconnect() {
        return socket->close();
    }

private:
    NetworkInterface* network;
    TCPSocket* socket;
};

#endif // _MQTTNETWORK_H_