import React from 'react';
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

// status === null means it is disconnected
export function useRobotStatus() {
  const [status, setStatus] = React.useState<RobotStatus | null>(null);
  const [connected, setConnected] = React.useState(false);
  const timerRef = React.useRef<ReturnType<typeof setTimeout>>(undefined);

  const resetTimer = () => {
    if(timerRef.current) {
      clearTimeout(timerRef.current);
    }
    setConnected(true);

    timerRef.current = setTimeout(() => {
      setConnected(false);
    }, TIMEOUT_MS);
  }

  React.useEffect(() => {
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

      setStatus({
        armed: json.armed,
        ota: json.ota,
      })
    })

    return () => {
      unsub();
      clearTimeout(timerRef.current);
    }
  }, [])

  return { status, connected };
}
