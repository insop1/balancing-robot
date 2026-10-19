import React from 'react';
import './SerialMonitor.css';
import { subscribe } from '../websocket.ts';

export function SerialMonitor() {
  const [lines, setLines] = React.useState<string[]>([]);
  
  React.useEffect(() => {
    return subscribe((msg) => {
      setLines((prev) => [...prev, msg]);
    });
  }, [])

  return (
    <div className="serial-monitor">
      <pre>{ lines.join('\n') }</pre>
    </div>
  );
}
