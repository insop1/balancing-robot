#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "shared.h"
#include "network.h"
#include "control.h"


std::atomic<bool> otaActive(false);
QueueHandle_t toControlQueue;
QueueHandle_t toNetworkQueue;

void setup() {
    Serial.begin(115200);

    toControlQueue = xQueueCreate(16, sizeof(Command));
    toNetworkQueue = xQueueCreate(32, sizeof(Log));

    // We are splitting network and control because wifi related stuff could interrupt the robot a little.
    xTaskCreatePinnedToCore(networkTask, "network", 8192, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(controlTask, "control", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
    vTaskDelete(nullptr);
}
