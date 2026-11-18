import { Controls } from './Controls.tsx';
import './App.css';
import { Monitor } from './Monitor.tsx';
import { RobotMsgType } from '../../../shared/messages.ts';

export function App() {
  return(
    <main className='layout'>
      <section className='controls' arial-label='Controls'>
        <Controls />
      </section>
      <section className='monitors' aria-label='Monitors'>
        <Monitor title="Log Monitor" type={RobotMsgType.Log} />
        <Monitor title="Error Monitor" type={RobotMsgType.Error} />
      </section>
    </main>
  )
}
