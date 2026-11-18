import React from 'react';
import { subscribe } from '../websocket.ts';
import { RobotMsgType } from '../../../shared/messages.ts';

export function ErrorMonitor() {
  const [lines, setLines] = React.useState<string[]>([]);
  const preRef = React.useRef<HTMLPreElement>(null);
  
  React.useEffect(() => {
    return subscribe((msg) => {
      if(msg.type !== RobotMsgType.Error) return;
      setLines((prev) => [...prev, msg.text]);
    });
  }, [])

  React.useEffect(() => {
    const monitor = preRef.current;
    if(monitor) monitor.scrollTop = monitor.scrollHeight;
  }, [lines])

  return (
    <div className="error-monitor">
      <h3 className="monitor-title">Error Monitor</h3>
      <pre ref={preRef}>{ lines.join('\n') }</pre>
    </div>
  );
}
