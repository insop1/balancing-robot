import React from 'react';
import { send } from '../websocket.ts';
import './Controls.css';

export function Input() {
  const [text, setText] = React.useState('');

  function handleKeyDown(event: React.KeyboardEvent<HTMLInputElement>) {
    if (event.key === 'Enter') {
      send(text);
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
