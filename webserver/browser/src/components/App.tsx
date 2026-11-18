import { Controls } from './Controls.tsx';
import { LogMonitor } from './LogMonitor.tsx';
import { ErrorMonitor } from './ErrorMonitor.tsx'
import './App.css';

export function App() {
  return(
    <main className='layout'>
      <section className='controls' arial-label='Controls'>
        <Controls />
      </section>
      <section className='monitors' aria-label='Monitors'>
        <LogMonitor />
        <ErrorMonitor />
      </section>
    </main>
  )
}
