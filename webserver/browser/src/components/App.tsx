import { Controls } from './Controls.tsx';
import { SerialMonitor } from './SerialMonitor.tsx';
import './App.css';

export function App() {
  return(
    <main className='layout'>
      <section className='controls'>
        <Controls />
      </section>
      <section className='monitor'>
        <SerialMonitor />
      </section>
    </main>
  )
}
