#pragma once

#include <Arduino.h>
#include <atomic>

constexpr uint8_t LOG_TEXT_SIZE = 128;
extern std::atomic<bool> otaActive;

// Because we splitted the two cores, we'll need a way for them to communicate.
// Network task might send a state to control task.
// Control task might send logs to network task to send to the server.

// Network to Control
enum class CommandType : uint8_t {
    Armed = 0,
    Joystick = 1,
    Calibrate = 2,
    SetGain = 3,
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
enum class MsgType : uint8_t {
    Log = 0,
    Error = 1,
    Status = 2,
};
struct Msg {
    MsgType type;
    char text[LOG_TEXT_SIZE];
};
extern QueueHandle_t toNetworkQueue;
