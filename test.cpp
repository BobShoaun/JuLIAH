#include "EventQueue.h"
#include "MQTTClient.h"
#include "mbed.h"
#include "wifi_helper.h"
#include "mbed-trace/mbed_trace.h"

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

    void send_msg(MQTTClient* client, const char *pubTopic, char *msg)
    {
        MQTT::Message message;
        message.qos = MQTT::QOS0;
        message.retained = false;
        message.dup = false;
        message.payload = (void*)msg;
        message.payloadlen = strlen(msg);
        int rc = client->publish(pubTopic, message);
        printf("rc from client publish is %d\r\n", rc); 
    }

int main()
{
    _net(NetworkInterface::get_default_instance())
    nsapi_size_or_error_t result = _net->connect();
    
    if (result != 0) {
            printf("Error! _net->connect() returned: %d\r\n", result);
            return;
    }
    print_network_info();
    // newest MQTT stuff
        TCPSocket socket;
        MQTTClient client(&socket);
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
        
        const char* pubTopic = "juliah/sound";
        const char* subTopic = "juliah/blink";

        // from HelloMQTT
        MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
        data.MQTTVersion = 3;
        data.clientID.cstring = "mbed-sample";
        if ((rc = client.connect(data)) != 0)
            printf("rc from client connect is %d\r\n", rc);

        rc = client.subscribe(subTopic, MQTT::QOS0, messageArrived);
        printf("rc from MQTT subscribe is %d\r\n", rc);

        char msg[100];
        sprintf(msg, "{\"timestamp\": 100000000, \"peakVolume\": 43.4, \"audio\": \"\"}");
        send_msg(&client, pubTopic, msg);
        
        while (arrivedcount < 1){
            client.yield(100);
            --arrivedcount;
        }
        printf("Demo concluded successfully \r\n");

}
