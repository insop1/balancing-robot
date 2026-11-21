# About

wip: A blank check project for Ars Vallis Ingenium (AVI) robotics club.

## Setup

### Uploading Firmware

First upload (over USB):
```bash
cd firmware
pio run -e wire -t upload
```

Every upload after (over OTA):
```bash
cd firmware
pio run -e ota -t upload
```

### Browser Setup
Note: server's device must be the first device connected to the robot.
1.  Build the browser UI:
    ```bash
    cd webserver/browser
    npm ci
    npm run build
    ```
2. Start the server:
    ```bash
    cd ../host
    npm ci
    npm run start
    ```
3. Connect to the robot's `freedonuts` WiFi. Password: `omgdonuts`.
4. Open `http://localhost:3103/`

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
// kp and kd are unused backups.
// index starts at 0
constexpr float *tuneArray[] = {&k1, &k2, &k3, &k4, &k5, &k6, &kp, &kd};
```

