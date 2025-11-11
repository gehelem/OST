const WebSocket = require("ws");

const ws = new WebSocket("ws://127.0.0.1:9624");
let messageCount = 0;

ws.on("open", () => {
  console.log("Connected to server");
  ws.send(JSON.stringify({ evt: "Freadall" }));

  // Close connection after 15 seconds
  setTimeout(() => {
    console.log(`Received ${messageCount} message(s) in total`);
    ws.close();
    process.exit(0);
  }, 15000);
});

ws.on("message", (data) => {
  messageCount++;
  console.log(`Message ${messageCount}:`, data.toString());
});

ws.on("error", (err) => {
  console.error("Error:", err.message);
  process.exit(1);
});

ws.on("close", () => {
  if (messageCount === 0) {
    console.error("FAILED: No messages received from server");
    process.exit(1);
  }
  process.exit(0);
});
