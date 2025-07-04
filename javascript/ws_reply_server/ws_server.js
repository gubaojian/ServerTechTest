import WebSocket, { WebSocketServer } from 'ws';

const wss = new WebSocketServer({ port: 8080 });

var messageCount = 0;

wss.on('connection', function connection(ws) {
  ws.on('error', console.error);

  ws.on('message', function message(data) {
     console.log('Message %d received: %s', data);
     ws.send(`replay: ${data}`);
  });
});

console.log('WebSocket server is running on ws://localhost:8080');
