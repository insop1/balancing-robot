#include "Arduino.h"
#include <WiFi.h>
#include <ArduinoOTA.h>

const char* ssid = "freedonuts";
const char* password = "omgdonuts";

// Notes: Write onStart for ArduinoOTA, have an atomic bool so we can turn off motors
// Write UDP stuff for sending and receiving
// Create a log system that replicates Serial log

void networkTask(void*) {
    while(true) {
        ArduinoOTA.handle();
        //wip
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void controlTask(void*) {
    TickType_t lastTick = xTaskGetTickCount();
    while(true) {
        // wip
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(5));
    }
}

void setup() {
    Serial.begin(115200);

    WiFi.softAP(ssid, password);
    ArduinoOTA.begin();

    xTaskCreatePinnedToCore(networkTask, "network", 8192, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(controlTask, "control", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
    vTaskDelete(nullptr);
}
