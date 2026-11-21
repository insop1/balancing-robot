import React from 'react';
import { send } from '../websocket.ts';
import { CommandType } from '../../../shared/messages.ts';
import { useRobotStatus } from '../hooks/RobotStatus.ts';

export function ButtonPanel() {
  const { status, connected } = useRobotStatus();

  function armedClick() {
    if(!connected || !status) {
      return;
    }
    const armed = status.armed ? "0" : "1";
    send({ type: CommandType.Armed, text: armed });
  }

  function restartClick() {
    if(!connected || !status) {
      return;
    }
    send({ type: CommandType.Restart, text: "" })
  }

  return(
    <div className='ButtonPanel'>
      <button onClick={armedClick} disabled={!connected}>Toggle Arm</button>
      <button onClick={restartClick} disabled={!connected}>Restart ESP32</button>
    </div>
  )
}
