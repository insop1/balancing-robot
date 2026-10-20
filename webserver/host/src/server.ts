import express from 'express';
import dgram from 'node:dgram';
import { WebSocketServer } from 'ws';
import { createServer } from 'node:http';
import { join } from 'node:path'
import { type RoboData, type ServerMsg } from '../../shared/messages.ts'

const ROBOT_PORT = 3104;
const ROBOT_IP = '192.168.4.1'

const serverBootId = crypto.getRandomValues(new Uint32Array(1))[0];
let serverSeq = 0;

let lastSeq = -1;
let lastBoot = -1;

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

function sendToRobot(msg: ServerMsg) {
  const buf = Buffer.alloc(9 + Buffer.byteLength(msg.text));

  // uint32, uint32, uint8, char* for the robot
  buf.writeUint32LE(serverBootId, 0);
  buf.writeUint32LE(serverSeq++, 4);
  buf.writeUint8(msg.type, 8);
  buf.write(msg.text, 9, 'utf8');

  udp.send(buf, ROBOT_PORT, ROBOT_IP);
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
  ws.on('message', (data: string) => {
    let json;
    try {
      json = JSON.parse(data);
    } 
    catch { return; }

    if(typeof json.type !== 'number' || typeof json.text !== 'string') return;
    if(Buffer.byteLength(json.text) >= 120) return;

    const serverMsg: ServerMsg = {
      type: json.type,
      text: json.text,
    }
    sendToRobot(serverMsg);
  });
  ws.on('close', () => console.log('browser left'));
});

udp.bind(3103);
http.listen(3103);
