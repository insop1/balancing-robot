#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <stdio.h>

#include "network.h"
#include "shared.h"

constexpr uint16_t UDP_PORT = 3103;

namespace {

constexpr uint8_t UDP_BUF_SIZE = 64;
struct __attribute__((packed)) TPacket {
    uint32_t serverBootId;
    uint32_t serverSeq;
    uint8_t type;
    char text[UDP_BUF_SIZE - 9];
};

uint32_t robotBootID;
uint32_t robotSeq = 0;

uint32_t serverLastBoot = 0;
uint32_t serverLastSeq = 0;

IPAddress serverIP;
uint16_t serverPort = 0;

WiFiUDP udp;

bool parseCommand(const TPacket &pck, Command *cmd) {
    // return true if parsed
    switch(cmd->type) {
        case JOYSTICK:
            sscanf(pck.text, "%f,%f", &cmd->a, &cmd->b);
            break;
        case CALIBRATE:
        case ARMED:
            cmd->a = atof(pck.text);
        default:
            return false;
    }
    return true;
}

void handlePacket() {
    uint8_t buf[UDP_BUF_SIZE];
    int n = udp.read(buf, sizeof(buf) - 1);
    udp.flush();

    if(n < 9) return;

    serverIP = udp.remoteIP();
    serverPort = udp.remotePort();

    // Byte offsets
    // 0: bootServerId uint32
    // 4: serverSeq uint32
    // 8: type uint8
    // 9+: text char[]

    buf[n] = '\0';
    TPacket pck;
    memcpy(&pck, buf, n + 1);

    if(pck.serverBootId != serverLastBoot) {
        // If robot restarts, serverLastBoot will be 0.
        // So we continue from the server's current seq.
        serverLastBoot = pck.serverBootId;
        serverLastSeq = pck.serverSeq;
    } 
    else if(pck.serverSeq <= serverLastSeq) {
        return;
    }
    else {
        serverLastSeq = pck.serverSeq;
    }

    Command cmd = { (CommandType)pck.type, 0.0f, 0.0f };
    if(!parseCommand(pck, &cmd)) return;

    xQueueSend(toControlQueue, &cmd, 0);
}

void sendToServer() {
    Log log;
    while(xQueueReceive(toNetworkQueue, &log, 0)) {
        uint8_t out[9 + LOG_TEXT_SIZE];
        uint8_t textLen = strlen(log.text);

        // Look at byte offsets in handlePacket. same format
        memcpy(out, &robotBootID, 4);
        memcpy(out + 4, &robotSeq, 4);
        out[8] = log.type;
        memcpy(out + 9, log.text, textLen);

        robotSeq++;

        udp.beginPacket(serverIP, serverPort);
        udp.write(out, 9 + textLen);
        udp.endPacket();
    }
}

} // namespace

void networkTask(void*) {
    robotBootID = esp_random();
    udp.begin(UDP_PORT);

    while(true) {
        ArduinoOTA.handle();
        
        int size = udp.parsePacket();
        if(size > 0) {
            handlePacket();
        }
        
        if(serverPort) {
            sendToServer();
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
