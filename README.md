# About

wip: A blank check project for Ars Vallis Ingenium (AVI) robotics club.

## Browser Commands

Use `1` and `0` to mean `true` and `false` respectively.

### Syntax example
```bash
armed 0 # disarm
armed 1 # arm

joystick 1.0,0.1 # x,y

# Sets tuning constants for robot index,value
setgain 0,25
```

### `setgain` follows in code
```cpp
// kp and kd are unused backup.
// index starts at 0
constexpr float *tuneArray[] = {&k1, &k2, &k3, &k4, &k5, &k6, &kp, &kd};
```
