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

#include "juliah_mqtt.hpp"


int main() {
    printf("\r\nStarting socket demo\r\n\r\n");

#ifdef MBED_CONF_MBED_TRACE_ENABLE
    mbed_trace_init();
#endif

    JuLIAHMQTT *example = new JuLIAHMQTT();
    MBED_ASSERT(example);

    // start with led1 off
    led1 = false;
    
    example->setup();

    char msg[100];
    sprintf(msg, "{\"timestamp\": 100000000, \"peakVolume\": 43.4, \"audio\": \"\"}");
    example->send_msg(pubTopic, msg);
    while (example->has_message()){
        example->listen_message();
    }

    example->cleanup();
    return 0;
}