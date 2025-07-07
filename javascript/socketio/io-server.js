import { App } from "uWebSockets.js";
import{ Server } from "socket.io";

const app = App();
const io = new Server();

io.attachApp(app);

io.on("connection", (socket) => {
  socket.on('test_message', (msg) => {
   // console.log('message: ' + msg);
  });
});

app.listen(3000, (token) => {
  if (!token) {
    console.warn("port already in use");
  } else {
    console.log("Listening to port 3000");
  }
});