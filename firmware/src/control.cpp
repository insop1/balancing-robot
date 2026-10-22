#include "Arduino.h"
#include "control.h"
#include "shared.h"
#include "log.h"

namespace {



} // namespace

void controlTask(void*) {
    TickType_t lastTick = xTaskGetTickCount();
    while(true) {
        // wip

        Command command;
        while(xQueueReceive(toControlQueue, &command, 0)) {
            net::log("ESP32 received { type: %d, a: %f, b: %f }", command.type, command.a, command.b);
        }
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(5));
    }
}
