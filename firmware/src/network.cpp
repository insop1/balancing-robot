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

uint32_t lastBoot = 0;
uint32_t lastSeq = 0;

IPAddress laptopIP;
uint16_t laptopPort = 0;

WiFiUDP udp;

bool parseCommand(const TPacket &pck, Command &command) {
    // return true if parsed
    switch(command.type) {
        case JOYSTICK:
            sscanf(pck.text, "%f,%f", &command.a, &command.b);
            break;
        case CALLIBRATE:
            command.a = atof(pck.text);
            break;
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

    if(pck.serverBootId != lastBoot) {
        lastBoot = pck.serverBootId;
        lastSeq = pck.serverSeq;
    } 
    else if(pck.serverSeq <= lastSeq) {
        return;
    }
    else {
        lastSeq = pck.serverSeq;
    }

    Command command = { (CommandType)pck.type, 0.0f, 0.0f };
    if(!parseCommand(pck, command)) return;

    xQueueSend(toControlQueue, &command, 0);
}

} // namespace

void networkTask(void*) {
    udp.begin(UDP_PORT);

    while(true) {
        ArduinoOTA.handle();
        
        int size = udp.parsePacket();
        if(size > 0) {
            handlePacket();
        }
        
        if(laptopPort) {
            // wip
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
