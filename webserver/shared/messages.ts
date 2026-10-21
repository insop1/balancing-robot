export const MsgType = {
  Log: 1,
  Error: 2,
} as const;

export type RoboData = {
  bootId: number, // uint32_t 0 - 3
  seq: number,  // uint32_t 4 - 7
  type: number, // uint8_t so 8
  text: string, // 9+
}

export type ServerMsg = {
  type: number,
  text: string,
}
