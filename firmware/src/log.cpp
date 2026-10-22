
#include "log.h"
#include "shared.h"

namespace net {

void log(const char *fmt, ...) {
    Log msg;
    msg.type = LOG;
    
    va_list args;
    va_start(args, fmt);
    vsnprintf(msg.text, sizeof(msg.text), fmt, args);
    va_end(args);

    xQueueSend(toNetworkQueue, &msg, 0);
}

}
