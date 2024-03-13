#include "mbed.h"
#include "InterruptIn.h"
#include "mbed_thread.h"


DigitalOut led(LED1);


// applicationState -> 0 = not monitoring; 1 = monitoring; 2 = sending data
int applicationState = 0;


void buttonToggleHandler() {
    if (applicationState == 0) {
        applicationState = 1;
    } else {
        applicationState = 0;
    }
}

void init() {
    InterruptIn button(BUTTON1);

    button.rise(&buttonToggleHandler);
}



// Place in main.cpp
int main() {
    // main loop
    if (applicationState == 1) {
        led = 1;

        // turn on the microphone & the led
        // read the data from the microphone
        // take action based on the readings
            // if loud noise -> upload data to server
            // else -> do nothing
    }



}