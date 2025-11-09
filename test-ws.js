const WebSocket = require("ws");

const ws = new WebSocket("ws://127.0.0.1:9624");

ws.on("open", () => {
  console.log("Connected to server");
  ws.send(JSON.stringify({ evt: "Freadall" }));
});

ws.on("message", (data) => {
  console.log("Received:", data);
  ws.close();
  process.exit(0);
});

ws.on("error", (err) => {
  console.error("Error:", err.message);
  process.exit(1);
});

setTimeout(() => {
  console.error("Timeout: no response received");
  process.exit(1);
}, 5000);
