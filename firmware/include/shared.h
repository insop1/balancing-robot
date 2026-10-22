#pragma once

#include <Arduino.h>
#include <atomic>

constexpr uint8_t LOG_TEXT_SIZE = 128;
extern std::atomic<bool> otaActive;

// Because we splitted the two cores, we'll need a way for them to communicate.
// Network task might send a state to control task.
// Control task might send logs to network task to send to the server.

// Network to Control
enum CommandType : uint8_t {
    ARMED = 0,
    JOYSTICK = 1,
    CALIBRATE = 2,
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
    LOG = 0,
    ERROR = 1,
};
struct Log {
    LogType type;
    char text[LOG_TEXT_SIZE];
};
extern QueueHandle_t toNetworkQueue;
