const WebSocket = require("ws");

const ws = new WebSocket("ws://127.0.0.1:9624");

function ts() {
  return new Date().toISOString();
}

// Sequence of commands to send, with optional comment
const commands = [
  // dump request
  { label: "dump request",  msg: {"DU":{"language":"en"}} },
  // login
  { label: "login",  msg: {"LO":{"user":"ADMIN","pw":"admin"}} },
  // stop all
  { label: "stop MyNavigator", msg: {"SV":{"m":{"MyNavigator":{"p":{"actions":{"e":{"abortnavigator":true}}}}}}} },
  { label: "stop MyPlanner", msg: {"SV":{"m":{"MyPlanner":{"p":{"actions":{"e":{"stop":true}}}}}}} },
  { label: "stop MyFocuser", msg: {"SV":{"m":{"MyFocuser":{"p":{"actions":{"e":{"abortfocus":true}}}}}}} },
  { label: "stop MySequencer", msg: {"SV":{"m":{"MySequencer":{"p":{"actions":{"e":{"abortsequence":true}}}}}}} },
  { label: "stop MyGuider", msg: {"SV":{"m":{"MyGuider":{"p":{"actions":{"e":{"abortguider":true}}}}}}} },

  // start planner
  { label: "start planner",  msg: {"SV":{"m":{"MyPlanner":{"p":{"actions":{"e":{"start":true}}}}}}} },
];

const DELAY_BETWEEN_COMMANDS_MS = 300;
const LISTEN_AFTER_LAST_MS = 5000;

let messageCount = 0;

async function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

async function runSequence() {
  for (let i = 0; i < commands.length; i++) {
    const { label, msg } = commands[i];
    const payload = JSON.stringify(msg);
    console.log(`[${ts()}] SEND [${i + 1}/${commands.length}] ${label}: ${payload}`);
    ws.send(payload);
    if (i < commands.length - 1) {
      await sleep(DELAY_BETWEEN_COMMANDS_MS);
    }
  }

  // Keep listening for LISTEN_AFTER_LAST_MS after the last command
  await sleep(LISTEN_AFTER_LAST_MS);
  console.log(`[${ts()}] Done. Received ${messageCount} message(s) in total.`);
  ws.close();
  process.exit(0);
}

ws.on("open", () => {
  console.log(`[${ts()}] Connected to server`);
  runSequence();
});

ws.on("message", (data) => {
  messageCount++;
  console.log(`[${ts()}] RECV [${messageCount}]: ${data.toString()}`);
});

ws.on("error", (err) => {
  console.error(`[${ts()}] Error: ${err.message}`);
  process.exit(1);
});

ws.on("close", () => {
  if (messageCount === 0) {
    console.error(`[${ts()}] FAILED: No messages received from server`);
    process.exit(1);
  }
});
