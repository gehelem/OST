#!/bin/bash
#
# CI - Arch test job
# Requires: PKG_FILE_LATEST artifact from build-job-arch.
#
# libindi (a depends of ostserver/ostserver-complete, pulled in
# automatically by pacman -U) bundles indiserver + the stock simulator
# drivers in the one package on Arch - no separate indi-bin-equivalent
# needed, unlike the Debian CI image's own explicit setup.
#
set -e

pacman -Syu --noconfirm
pacman -S --needed --noconfirm nodejs npm
pacman -U --noconfirm "${PKG_FILE_LATEST}"

npm install ws

mkdir -p media

# The runner's working directory isn't guaranteed to be wiped between jobs
# (Docker executor cache/reuse) - ost.db isn't a tracked file, so a stale
# one from a previous run (built against an older DB schema) can survive
# and get opened as-is instead of freshly created, crashing the server on
# a schema it no longer expects.
rm -f ost.db

echo "Starting server..."
export QT_QPA_PLATFORM=offscreen
ostserver --webroot=media --indiserver=Y --configuration=d1 > server.log 2>&1 &
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
