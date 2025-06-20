import WebSocket, { WebSocketServer } from 'ws';

const wss = new WebSocketServer({ port: 8080 });

var messageCount = 0;

wss.on('connection', function connection(ws) {
  ws.on('error', console.error);

  ws.on('message', function message(data) {
    messageCount++;
    if (messageCount % 10000 === 0) {
       console.log('Message %d received: %s', messageCount, data);
    }
  });
});

console.log('WebSocket server is running on ws://localhost:8080');
