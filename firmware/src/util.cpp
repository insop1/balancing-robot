#include "util.h"

namespace util {

float clamp(float v, float min, float max) {
    float result = v;
    if(v > max) {
        result = max;
    }
    else if(v < min) {
        result = min;
    }
    return result;
}

int sign(float n) {
    if(n >= 0) {
        return 1;
    } else {
        return -1;
    }
}

} // namespace
