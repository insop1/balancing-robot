import React from 'react';
import { subscribe } from '../websocket.ts';
import { RobotMsgType } from '../../../shared/messages.ts';

type Props = { 
  title: string, 
  type: RobotMsgType,
};

export function Monitor({ title, type }: Props) {
  const [lines, setLines] = React.useState<string[]>([]);
  const preRef = React.useRef<HTMLPreElement>(null);
  
  React.useEffect(() => {
    return subscribe((msg) => {
      if(msg.type !== type) return;
      setLines((prev) => [...prev, msg.text]);
    });
  }, [])

  React.useEffect(() => {
    const monitor = preRef.current;
    if(monitor) monitor.scrollTop = monitor.scrollHeight;
  }, [lines])

  return (
    <div className="monitor">
      <h3 className="monitor-title">{title}</h3>
      <pre ref={preRef}>{ lines.join('\n') }</pre>
    </div>
  );
}
