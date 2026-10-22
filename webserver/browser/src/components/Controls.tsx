import React from 'react';
import { send } from '../websocket.ts';
import './Controls.css';
import { CommandType } from '../../../shared/messages.ts';

export function Input() {
  const [text, setText] = React.useState('');

  function handleKeyDown(event: React.KeyboardEvent<HTMLInputElement>) {
    if (event.key === 'Enter') {
      const lowerText = text.toLowerCase();
      const args: string[] = lowerText.split(' ');
      
      // currently just a temporary test
      let cType;
      switch(args[0]) {
        case "armed":
          cType = CommandType.Armed;
          break;
        case "joystick":
          cType = CommandType.Joystick;
          break;
        case "calibrate":
          cType = CommandType.Calibrate;
          break;
        default:
          return;
      }

      send({ type: cType, text: args[1] });
      setText('');
    }
  }

  return (
    <div className='Controls'>
      <label htmlFor="text-box">Type something: </label>
      <input
        id="text-box"
        value={text}
        onChange={(e) => setText(e.target.value)}
        onKeyDown={handleKeyDown}
      />
    </div>
  );
}
