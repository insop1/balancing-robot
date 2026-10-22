import React from 'react';
import './SerialMonitor.css';
import { subscribe } from '../websocket.ts';
import { RobotMsgType } from '../../../shared/messages.ts';

export function SerialMonitor() {
  const [lines, setLines] = React.useState<string[]>([]);
  const preRef = React.useRef<HTMLPreElement>(null);
  
  React.useEffect(() => {
    return subscribe((msg) => {
      if(msg.type !== RobotMsgType.Log) return;
      setLines((prev) => [...prev, msg.text]);
    });
  }, [])

  React.useEffect(() => {
    const monitor = preRef.current;
    if(monitor) monitor.scrollTop = monitor.scrollHeight;
  }, [lines])

  return (
    <div className="serial-monitor">
      <pre ref={preRef}>{ lines.join('\n') }</pre>
    </div>
  );
}
