#include "log.h"
#include "shared.h"

namespace {

void vLog(MsgType type, const char *fmt, va_list args) {
    Msg msg;
    msg.type = type;
    vsnprintf(msg.text, LOG_TEXT_SIZE, fmt, args);
    xQueueSend(toNetworkQueue, &msg, 0);
}

} // namespace

namespace net {

void log(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vLog(MsgType::Log, fmt, args);
    va_end(args);
}
void error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vLog(MsgType::Error, fmt, args);
    va_end(args);
}
void status(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vLog(MsgType::Status, fmt, args);
    va_end(args);
}

} // namespace
