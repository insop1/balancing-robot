// From robot to server
export const RobotMsgType = {
  Log: 0,
  Error: 1,
} as const;

// From server to robot
export const CommandType = {
  Armed: 0,
  Joystick: 1,
  Calibrate: 2,
}

export type RobotData = {
  bootId: number, // uint32_t 0 - 3
  seq: number,  // uint32_t 4 - 7
  type: number, // uint8_t so 8
  text: string, // 9+
}

export type ServerMsg = {
  type: number,
  text: string,
}
