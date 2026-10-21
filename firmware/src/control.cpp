#include "Arduino.h"
#include "control.h"

void controlTask(void*) {
    TickType_t lastTick = xTaskGetTickCount();
    while(true) {
        // wip
        // Write receiver for toControlQueue
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(5));
    }
}
