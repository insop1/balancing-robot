#pragma once

#include <Arduino.h>
#include <atomic>

extern std::atomic<bool> otaActive;

// Because we splitted the two cores, we'll need a way for them to communicate.
// Network task might send a state to control task.
// Control task might send logs to network task to send to the server.

// Network to Control
enum CommandType : uint8_t {
    KEY_ARMED = 0,
    JOYSTICK = 1,
    CALLIBRATE = 2,
};
struct Command {
    CommandType type;

    // We're using float because it can still represent bool with 1.0 or 0.0
    // Some data we get from server are floats
    // float b is only used for type JOYSTICK. Otherwise, it gets ignored.
    float a;
    float b;
};
extern QueueHandle_t toControlQueue;

// Control to Network
enum LogType : uint8_t {
    LOG,
};
struct Log {
    LogType type;
    char text[128];
};
extern QueueHandle_t toNetworkQueue;
