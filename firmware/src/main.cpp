#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#include "shared.h"
#include "network.h"
#include "control.h"

const char* SSID = "freedonuts";
const char* PASSWORD = "omgdonuts";

// Reminder: create a log system that replicates Serial log

void setup() {
    Serial.begin(115200);

    WiFi.softAP(SSID, PASSWORD);
    ArduinoOTA.begin();
    otaActive = false;

    ArduinoOTA.onStart([] {
        otaActive = true;
        vTaskDelay(pdMS_TO_TICKS(20));
    });

    ArduinoOTA.onError([] (ota_error_t) {
        otaActive = false;
    });

    toControlQueue = xQueueCreate(16, sizeof(Command));
    toNetworkQueue = xQueueCreate(32, sizeof(Log));

    // We are splitting network and control because wifi related stuff could interrupt the robot a little.
    xTaskCreatePinnedToCore(networkTask, "network", 8192, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(controlTask, "control", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
    vTaskDelete(nullptr);
}
