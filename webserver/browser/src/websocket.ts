type Listener = (msg: any) => void;
const listeners = new Set<Listener>();
let ws: WebSocket;

function connect() {
  ws = new WebSocket(`ws://${location.host}/donuts`);
  ws.onmessage = (e) => {
    const msg = e.data;
    for(const fn of listeners) fn(msg);
  };
  ws.onclose = () => setTimeout(connect, 1000);
}
connect();

export function send(msg: string) {
  if(ws.readyState === WebSocket.OPEN) ws.send(msg);
}

export function subscribe(fn: Listener) {
  listeners.add(fn);
  return () => { listeners.delete(fn); };
}
