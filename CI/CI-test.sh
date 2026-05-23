#!/bin/bash
#
# CI - test job
# Requires: DEB_FILE_LATEST artifact from build-job
#
set -e

apt-get update -qq
apt-get install -y software-properties-common nodejs npm
apt-add-repository ppa:mutlaqja/ppa
apt-get update -qq
dpkg -i "${DEB_FILE_LATEST}" || true
apt-get install -f -y

npm install ws

mkdir -p media

echo "Starting server..."
export QT_QPA_PLATFORM=offscreen
ostserver --webroot=media > server.log 2>&1 &
SERVER_PID=$!

sleep 7

echo "Checking server process..."
if ! ps -p $SERVER_PID > /dev/null 2>&1; then
    echo "ERROR: server crashed at startup!"
    cat server.log
    exit 1
fi

MEM_KB=$(ps -o rss= -p $SERVER_PID 2>/dev/null || echo 0)
echo "Server RSS memory: ${MEM_KB} KB"
if [ "${MEM_KB}" -gt 524288 ]; then
    echo "ERROR: server memory usage too high (${MEM_KB} KB > 512 MB) - possible OOM regression"
    cat server.log
    kill $SERVER_PID || true
    exit 1
fi

node test-ws.js 2>&1 | tee ws.log

echo "Validating WebSocket messages..."
MSG_COUNT=$(grep -c "^Message" ws.log || true)
if [ "${MSG_COUNT}" -lt 2 ]; then
    echo "ERROR: expected multiple messages, got ${MSG_COUNT}"
    cat ws.log
    kill $SERVER_PID || true
    exit 1
fi
if ! grep -q "{" ws.log || ! grep -q ":" ws.log; then
    echo "ERROR: messages do not look like JSON"
    cat ws.log
    kill $SERVER_PID || true
    exit 1
fi
echo "OK: received ${MSG_COUNT} JSON message(s)"

echo "Server logs:"
tail -50 server.log

kill $SERVER_PID || true
