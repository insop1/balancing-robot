import React from 'react';
import nipplejs from 'nipplejs';
import { send } from '../websocket.ts';
import { CommandType } from '../../../shared/messages.ts';

export function Joystick() {
  const zoneRef = React.useRef<HTMLDivElement>(null);
  const stick = React.useRef({ x: 0, y: 0 });

  React.useEffect(() => {
    const size = 150;
    const manager = nipplejs.create({
      zone: zoneRef.current!,
      mode: 'static',
      position: { left: '50%', top: '50%' },
      color: {
        front: 'white',
        back: 'rgba(100, 100, 100, 0.2)',
      },
      size,
    });
    
    manager.on('move', (evt) => {
      const data = evt.data;
      const mag = Math.min(data.distance / (size / 2), 1);
      stick.current = {
        x: Math.cos(data.angle.radian) * mag,
        y: Math.sin(data.angle.radian) * mag,
      }
    });
    manager.on('end', () => {
      stick.current = { x: 0, y: 0 };
    });

    const timer = setInterval(() => {
      const { x, y } = stick.current;
      send({ type: CommandType.Joystick, text: `${x.toFixed(3)},${y.toFixed(3)}` });
    }, 33);

    return () => {
      clearInterval(timer);
      manager.destroy();
    };
  }, []);

  return <div ref={zoneRef} style={{ position: 'relative', width: 200, height: 200 }}/>;
}
