import { io } from "socket.io-client";

const socket = io("ws://127.0.0.1:3000/", {
  transports: ["websocket"],
  reconnectionAttempts: 5,
  reconnectionDelay: 1000,
  timeout: 2000,
  autoConnect: true,
  withCredentials: true,
  extraHeaders: {
    "X-Custom-Header": "value",
  }});

socket.on("connect", () => {
  socket.emit("test_message", "Hello, server!");
});
socket.on("disconnect", (reason) => {
  console.log("Disconnected:", reason);
});

// 防止Node.js事件循环退出的关键代码
socket.on("connect_error", (err) => {
  console.error("Connection error:", err);
});

socket.on("test_message", (msg) => {
  console.log("Received message:", msg);
});

 socket.emit("test_message", "Hello, server!");

 const message = "Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!Hello, server!";
 for(var i=0; i<1000*1000*2; i++) {
    socket.emit("test_message",  message);
 }