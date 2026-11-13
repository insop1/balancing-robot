import React from 'react';
import { send } from '../websocket.ts';
import './Input.css';
import { CommandType } from '../../../shared/messages.ts';

const commandMap: Record<string, CommandType | undefined> = {
  armed: CommandType.Armed,
  joystick: CommandType.Joystick,
  calibrate: CommandType.Calibrate,
  setgain: CommandType.SetGain,
};

export function Input() {
  const [text, setText] = React.useState('');

  function handleKeyDown(event: React.KeyboardEvent<HTMLInputElement>) {
    if (event.key === 'Enter') {
      const lowerText = text.toLowerCase();
      const args: string[] = lowerText.split(' ');
      
      const cType = commandMap[args[0]];
      setText('');

      if (cType === undefined) return;
      send({ type: cType, text: args[1] });
    }
  }

  return (
    <div className='Controls'>
      <label htmlFor="text-box">Command: </label>
      <input
        id="text-box"
        value={text}
        onChange={(e) => setText(e.target.value)}
        onKeyDown={handleKeyDown}
      />
    </div>
  );
}
