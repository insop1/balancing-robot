#include "Arduino.h"
#include "control.h"
#include "esp32-hal.h"
#include "shared.h"
#include "log.h"
#include "gyro.h"
#include "motors.h"

namespace {

uint32_t logPrev = 0;
uint32_t logInterval = 200;

} // namespace

void controlTask(void*) {
    TickType_t lastTick = xTaskGetTickCount();

    if(!gyro::init()) {
        net::log("Could not initialize imu");
        while(true) delay(100000);
    }

    motors::init();
    while(true) {
        Command cmd;
        while(xQueueReceive(toControlQueue, &cmd, 0)) {
            if(cmd.type == JOYSTICK) {
                motors::turn(cmd.a, cmd.b);
            }
        }
        vTaskDelayUntil(&lastTick, pdMS_TO_TICKS(5));
    }
}
