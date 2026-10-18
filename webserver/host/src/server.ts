import express from 'express';
import dgram from 'node:dgram';
import { WebSocketServer } from 'ws';
import { createServer } from 'node:http';
import { join } from 'node:path'

const app = express();
app.use(express.static(join(import.meta.dirname, '../../browser/dist')));

const udp = dgram.createSocket('udp4');
const http = createServer(app);
const wss = new WebSocketServer({ server: http, path: '/donuts' });

udp.on('error', (err) => {
  console.error('udp error:\n${err.stack}');
  udp.close();
});

udp.on('message', (msg, rinfo) => {
  console.log('server got: ${msg} from ${rinfo.address}:${rinfo.port}');
  for (const client of wss.clients) {
    if (client.readyState === WebSocket.OPEN) client.send(msg.toString());
  }
});

udp.on('listening', () => {
  const address = udp.address();
  console.log('server listening ${address.address}:${address.port}');
});

wss.on('connection', (ws) => {
  console.log('browser connected');
  ws.on('message', (data) => console.log('browser sent: %s', data));
  ws.on('close', () => console.log('browser left'));
});

udp.bind(3103);
http.listen(3103);
