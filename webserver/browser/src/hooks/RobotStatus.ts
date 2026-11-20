import { create } from 'zustand';
import { subscribe, emit } from '../websocket.ts';
import { RobotMsgType } from '../../../shared/messages';

// Must match statusInterval in firmware/src/control.cpp
// Robot sends every 500ms. We give it 3 pings + 100ms of slack before timeout.
const STATUS_INTERVAL_MS = 500;
const TIMEOUT_MS = STATUS_INTERVAL_MS * 3 + 100;

export type RobotStatus = {
  armed: boolean,
  ota: boolean,
}

export type RobotState = {
  status: RobotStatus | null,
  connected: boolean,
}

export const useRobotStatus = create<RobotState>(() => ({ status: null, connected: false }));

let timer: ReturnType<typeof setTimeout> | undefined;

const resetTimer = () => {
  clearTimeout(timer);
  useRobotStatus.setState({ connected: true })

  timer = setTimeout(() => {
    useRobotStatus.setState({ connected: false })
  }, TIMEOUT_MS);
}

const unsub = subscribe((msg) => {
  if(msg.type !== RobotMsgType.Status) {
    return;
  }
  resetTimer();

  // Robot will send a json string for type Status
  let json;
  try {
    json = JSON.parse(msg.text);
  }
  catch { 
    emit({ type: RobotMsgType.Error, text: `Bad status JSON: ${msg.text}`});
    return; 
  }

  const status = {
    armed: json.armed,
    ota: json.ota,
  }

  useRobotStatus.setState({ status: status })
})

import.meta.hot?.dispose(() => { 
  unsub(); clearTimeout(timer); 
});
