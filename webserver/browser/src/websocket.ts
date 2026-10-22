import { type ServerMsg } from '../../shared/messages.ts';

type Listener = (msg: ServerMsg) => void;
const listeners = new Set<Listener>();
let ws: WebSocket;

function connect() {
  ws = new WebSocket(`ws://${location.host}/donuts`);
  ws.onmessage = (e) => {
    const msg = JSON.parse(e.data);
    for(const fn of listeners) fn(msg);
  };
  ws.onclose = () => setTimeout(connect, 1000);
}
connect();

export function send(msg: object) {
  if(ws.readyState === WebSocket.OPEN) {
    const data = JSON.stringify(msg);
    ws.send(data);
  }
}

export function subscribe(fn: Listener) {
  listeners.add(fn);
  return () => { listeners.delete(fn); };
}
