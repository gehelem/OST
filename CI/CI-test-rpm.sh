#!/bin/bash
#
# CI - Fedora RPM test job
# Requires: RPM_FILE_LATEST artifact from build-job-fedora.
#
set -e

dnf -y install "./${RPM_FILE_LATEST}" libindi nodejs npm

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

# Three diagnostics tried here and abandoned (see this file's own git
# history for each): gdb tracing only the top-level process (detaches
# from every forked child, catches nothing); scanning for a core dump
# afterwards (this runner's core_pattern redirects to the host,
# invisible from inside the container); gdb with detach-on-fork off to
# also trace forked children ("/proc/<pid>/ns/mnt: No such file or
# directory" - the GitLab Docker-executor container restricts /proc
# access for processes outside gdb's direct ptrace scope, so even this
# fails here specifically). Back to a plain run - this sandbox cannot
# give us a backtrace; reproducing on a real VM (full gdb privileges,
# same way the Arch side was debugged) is the next step, not CI.
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
