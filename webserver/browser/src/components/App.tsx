import './App.css';
import { Monitor } from './Monitor.tsx';
import { RobotMsgType } from '../../../shared/messages.ts';
import { Input } from './Input.tsx';
import { ButtonPanel } from './ButtonPanel.tsx';
import { Joystick } from './Joystick.tsx';

export function App() {
  return(
    <main className='layout'>
      <section className='controls' aria-label='Controls'>
        <Input />
        <ButtonPanel />
        <Joystick />
      </section>
      <section className='monitors' aria-label='Monitors'>
        <Monitor title="Log Monitor" type={RobotMsgType.Log} />
        <Monitor title="Error Monitor" type={RobotMsgType.Error} />
      </section>
    </main>
  )
}
