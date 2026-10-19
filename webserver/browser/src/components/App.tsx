import React from 'react';
import { Input } from './Controls.tsx';
import { SerialMonitor } from './SerialMonitor.tsx';
import './App.css';

export function App() {
  return(
    <main className='layout'>
      <section className='controls'>
        <Input />
      </section>
      <section className='monitor'>
        <SerialMonitor />
      </section>
    </main>
  )
}
