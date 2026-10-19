import React from 'react';
import './SerialMonitor.css';
import { subscribe } from '../websocket.ts';
import { MsgType, type ServerMsg } from '../../../shared/messages.ts';

export function SerialMonitor() {
  const [lines, setLines] = React.useState<string[]>([]);
  
  React.useEffect(() => {
    return subscribe((msg) => {
      if(msg.type !== MsgType.Log) return;
      setLines((prev) => [...prev, msg.text]);
    });
  }, [])

  return (
    <div className="serial-monitor">
      <pre>{ lines.join('\n') }</pre>
    </div>
  );
}
