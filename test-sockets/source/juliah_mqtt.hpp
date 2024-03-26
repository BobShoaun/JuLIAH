/* Sockets Example
 * Copyright (c) 2016-2020 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "EventQueue.h"
#include "MQTTClient.h"
#include "TCPSocket.h"
#include "mbed.h"
#include "wifi_helper.h"
#include "mbed-trace/mbed_trace.h"

// MQTT stuff

// newest MQTT stuff
#include <MQTTClientMbedOs.h>

// led stuff
DigitalOut led1(LED1);

#if MBED_CONF_APP_USE_TLS_SOCKET
#include "root_ca_cert.h"

#ifndef DEVICE_TRNG
#error "mbed-os-example-tls-socket requires a device which supports TRNG"
#endif
#endif // MBED_CONF_APP_USE_TLS_SOCKET

const char* pubTopic = "juliah/sound";
const char* subTopic = "juliah/blink";

// Helperfunc for blinking
void blink()
{
    led1 = !led1;
}

// MQTT helperfunc
int arrivedcount = 0;

static void messageArrived(MQTT::MessageData& md)
{
    ++arrivedcount;
    MQTT::Message &message = md.message;
    printf("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);

    // blink led1 every 1s for 10 times
    EventQueue queue;
    queue.call_every(150ms, blink);
    queue.dispatch_for(5s);
}


class JuLIAHMQTT {
    static constexpr size_t MAX_NUMBER_OF_ACCESS_POINTS = 10;
    static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;

#if MBED_CONF_APP_USE_TLS_SOCKET
    static constexpr size_t REMOTE_PORT = 443; // tls port
#else
    static constexpr size_t REMOTE_PORT = 80; // standard HTTP port
#endif // MBED_CONF_APP_USE_TLS_SOCKET

private:
    NetworkInterface *_net;
    TCPSocket socket;
    MQTTClient client;

public:
    JuLIAHMQTT() : _net(NetworkInterface::get_default_instance()), client(&socket)
    {
    }

    ~JuLIAHMQTT()
    {
        if (_net) 
            _net->disconnect();
    }


    void setup()
    {
        if (!_net) {
            printf("Error! No network interface found.\r\n");
            return;
        }

        /* if we're using a wifi interface run a quick scan */
        if (_net->wifiInterface()) {
            /* the scan is not required to connect and only serves to show visible access points */
            wifi_scan();

            /* in this example we use credentials configured at compile time which are used by
             * NetworkInterface::connect() but it's possible to do this at runtime by using the
             * WiFiInterface::connect() which takes these parameters as arguments */
        }

        /* connect will perform the action appropriate to the interface type to connect to the network */

        printf("Connecting to the network...\r\n");

        nsapi_size_or_error_t result = _net->connect();
        if (result != 0) {
            printf("Error! _net->connect() returned: %d\r\n", result);
            return;
        }

        print_network_info();

        socket.open(_net);
        const char* hostname = "test.mosquitto.org";
        int port = 1883;
        SocketAddress addr;
        // addr.set_ip_address(hostname);
        if (_net->gethostbyname(hostname, &addr) != NSAPI_ERROR_OK) {
            printf("hostname error");
        }
        addr.set_port(port);

        int rc = socket.connect(addr); 
        if (rc != 0)
            printf("rc from MQTT TCP connect is %d\r\n", rc);
        else
            printf("MQTT socket connected \n");
        
        // from HelloMQTT
        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        data.MQTTVersion = 3;
        data.clientID.cstring = "mbed-sample";
        if ((rc = client.connect(data)) != 0)
            printf("rc from client connect is %d\r\n", rc);

        rc = client.subscribe(subTopic, MQTT::QOS0, messageArrived);
        printf("rc from MQTT subscribe is %d\r\n", rc);
        printf("Setup concluded successfully \r\n");       
    }

    void send_msg(const char *pubTopic, char *msg)
    {
        MQTT::Message message;
        message.qos = MQTT::QOS0;
        message.retained = false;
        message.dup = false;
        message.payload = (void*)msg;
        message.payloadlen = strlen(msg);
        int rc = client.publish(pubTopic, message);
        printf("rc from client publish is %d\r\n", rc); 
    }

    void listen_message() {
        client.yield(100);
        --arrivedcount;
    }

    bool has_message() {
        return arrivedcount < 1;
    }

    void cleanup() {
        // clean up
        int rc = client.unsubscribe(subTopic);
        printf("rc from unsubscribe was %d\r\n", rc);

        rc = client.disconnect();
        printf("rc from client disconnect was %d\r\n", rc);

        _net->disconnect();
        printf("disconnected from wifi");
    }

private:



    bool resolve_hostname(SocketAddress &address)
    {
        const char hostname[] = MBED_CONF_APP_HOSTNAME;

        /* get the host address */
        printf("\nResolve hostname %s\r\n", hostname);
        nsapi_size_or_error_t result = _net->gethostbyname(hostname, &address);
        if (result != 0) {
            printf("Error! gethostbyname(%s) returned: %d\r\n", hostname, result);
            return false;
        }

        printf("%s address is %s\r\n", hostname, (address.get_ip_address() ? address.get_ip_address() : "None") );

        return true;
    }

    void wifi_scan()
    {
        WiFiInterface *wifi = _net->wifiInterface();

        WiFiAccessPoint ap[MAX_NUMBER_OF_ACCESS_POINTS];

        /* scan call returns number of access points found */
        int result = wifi->scan(ap, MAX_NUMBER_OF_ACCESS_POINTS);

        if (result <= 0) {
            printf("WiFiInterface::scan() failed with return value: %d\r\n", result);
            return;
        }

        printf("%d networks available:\r\n", result);

        for (int i = 0; i < result; i++) {
            printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\r\n",
                   ap[i].get_ssid(), get_security_string(ap[i].get_security()),
                   ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
                   ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5],
                   ap[i].get_rssi(), ap[i].get_channel());
        }
        printf("\r\n");
    }

    void print_network_info()
    {
        /* print the network info */
        SocketAddress a;
        _net->get_ip_address(&a);
        printf("IP address: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_netmask(&a);
        printf("Netmask: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
        _net->get_gateway(&a);
        printf("Gateway: %s\r\n", a.get_ip_address() ? a.get_ip_address() : "None");
    }


};
