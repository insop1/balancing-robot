import React from 'react';
import { subscribe } from '../websocket.ts';
import { RobotMsgType } from '../../../shared/messages.ts';
import './Monitor.css';

type Props = { 
  title: string, 
  type: RobotMsgType,
};

const MAX_LINES = 500;

export function Monitor({ title, type }: Props) {
  const [lines, setLines] = React.useState<string[]>([]);
  const preRef = React.useRef<HTMLPreElement>(null);
  
  React.useEffect(() => {
    return subscribe((msg) => {
      if(msg.type !== type) return;
      setLines((prev) => {
        const next = [...prev, msg.text];
        if(next.length > MAX_LINES) {
          next.shift();
        }
        return next;
      });
    });
  }, [])

  React.useLayoutEffect(() => {
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
