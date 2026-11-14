import React from 'react';
import { send } from '../websocket.ts';
import { CommandType, RobotMsgType } from '../../../shared/messages.ts';
import { subscribe } from '../websocket.ts';

interface RobotStatus {
  armed: boolean,
  ota: boolean,
}

export function ButtonPanel() {
  const [status, setStatus] = React.useState<RobotStatus>();
  
  React.useEffect(() => {
    return subscribe((msg) => {
      if(msg.type !== RobotMsgType.Status) return;

      let json;
      try {
        json = JSON.parse(msg.text);
      } 
      catch { return; }

      setStatus({
        armed: json.armed,
        ota: json.ota,
      })
    });
  }, [])

  function armedClick() {
    if(!status) {
      return;
    }
    const armed = status.armed ? "1" : "0";
    send({ type: CommandType.Armed, text: armed });
  }

  return(
    <div className='ButtonPanel'>
      <button onClick={armedClick}>Toggle Arm</button>
    </div>
  )
}
