#include "Arduino.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <atomic>
#include <cstdint>

WiFiUDP udp;

constexpr uint16_t UDP_PORT = 3103;
const char* SSID = "freedonuts";
const char* PASSWORD = "omgdonuts";

constexpr uint8_t UDP_BUF_SIZE = 64;
struct __attribute__((packed)) TPacket {
    uint32_t serverBootId;
    uint32_t serverSeq;
    uint8_t type;
    char text[UDP_BUF_SIZE - 9];
};

std::atomic<bool> otaActive(false);

// Future me: please split these files.

// Because we splitted the two cores, we'll need a way for them to communicate.
// Network task might send a state to control task.
// Control task might send logs to network task to send to the server.

// Network to Control
enum CommandType : uint8_t {
    KEY_ARMED,
    JOYSTICK_X,
    JOYSTICK_Y,
    CALLIBRATE,
};
struct Command {
    CommandType type;
    float value;
};
QueueHandle_t toControlQueue;

// Control to Network
enum LogType : uint8_t {
    LOG,
};
struct Log {
    LogType type;
    char text[128];
};
QueueHandle_t toNetworkQueue;
// Write UDP stuff for sending and receiving
// Create a log system that replicates Serial log

void networkTask(void*) {
    udp.begin(UDP_PORT);

    IPAddress laptopIP;
    uint16_t laptopPort = 0;

    while(true) {
        ArduinoOTA.handle();
        
        int size = udp.parsePacket();
        if(size > 0) {
            uint8_t buf[UDP_BUF_SIZE];
            int n = udp.read(buf, sizeof(buf) - 1);

            if(n >= 9) {
                laptopIP = udp.remoteIP();
                laptopPort = udp.remotePort();
                 
                // Byte offsets
                // 0: bootServerId uint32
                // 4: serverSeq uint32
                // 8: type uint8
                // 9+: text char[]

                buf[n] = '\0';
                TPacket pck;
                memcpy(&pck, buf, n + 1);

                // Write logic for serverBootId and serverSeq
                // Assume int from text based on types

                Command command = { (CommandType)pck.type, 31 /*placeholder*/};
                xQueueSend(toControlQueue, &command, 0);
            }
        }
        
        if(laptopPort) {

        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void controlTask(void*) {
    TickType_t lastTick = xTaskGetTickCount();
    while(true) {
        // wip
        // Write receiver for toControlQueue
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(5));
    }
}

void setup() {
    Serial.begin(115200);

    WiFi.softAP(SSID, PASSWORD);
    ArduinoOTA.begin();

    ArduinoOTA.onStart([] {
        otaActive = true;
        vTaskDelay(pdMS_TO_TICKS(20));
    });

    ArduinoOTA.onError([] (ota_error_t) {
        otaActive = false;
    });


    toControlQueue = xQueueCreate(16, sizeof(Command));
    toNetworkQueue = xQueueCreate(1, sizeof(Log));

    // We are splitting network and control because wifi related stuff could interrupt the robot a little.
    xTaskCreatePinnedToCore(networkTask, "network", 8192, nullptr, 1, nullptr, 0);
    xTaskCreatePinnedToCore(controlTask, "control", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
    vTaskDelete(nullptr);
}
