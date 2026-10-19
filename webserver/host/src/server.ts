import express from 'express';
import dgram from 'node:dgram';
import { WebSocketServer } from 'ws';
import { createServer } from 'node:http';
import { join } from 'node:path'
import { type RoboData, MsgType } from '../../shared/messages.ts'

let lastSeq: number;
let lastBoot: number;

const app = express();
app.use(express.static(join(import.meta.dirname, '../../browser/dist/')));

const udp = dgram.createSocket('udp4');
const http = createServer(app);
const wss = new WebSocketServer({ server: http, path: '/donuts' });

function unpack(buf: Buffer): RoboData | null {
  if (buf.length < 9) return null;
  return {
    bootId: buf.readUint32LE(0),
    seq: buf.readUint32LE(4),
    type: buf.readUint8(8),
    text: buf.toString('utf8', 9),
  }
}

function sendToBrowser(msg: object) {
  const json = JSON.stringify(msg);

  for (const client of wss.clients) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(json);
    }
  }
}

udp.on('error', (err) => {
  console.error(`udp error:\n${err.stack}`);
  udp.close();
});

udp.on('message', (buf) => {
  const data = unpack(buf);

  if(!data) return;
  console.log(JSON.stringify(data));

  // Restart everytime new bootId
  if(data.bootId != lastBoot) {
    lastBoot = data.bootId;
    lastSeq = -1;
  }

  // Only accepting newest data sequence
  if(data.seq <= lastSeq) return; 
  lastSeq = data.seq;

  sendToBrowser({
    type: data.type,
    text: data.text,
  })
});

udp.on('listening', () => {
  const address = udp.address();
  console.log(`server listening ${address.address}:${address.port}`);
});

wss.on('connection', (ws) => {
  console.log('browser connected');
  ws.on('message', (data) => console.log('browser sent: %s', data));
  ws.on('close', () => console.log('browser left'));
});

udp.bind(3103);
http.listen(3103);
