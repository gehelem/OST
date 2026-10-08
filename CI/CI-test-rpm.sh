#!/bin/bash
#
# CI - Fedora RPM test job
# Requires: RPM_FILE_LATEST artifact from build-job-fedora.
#
set -e

dnf -y install "./${RPM_FILE_LATEST}" libindi nodejs npm gdb

npm install ws

mkdir -p media

echo "Starting server..."
export QT_QPA_PLATFORM=offscreen

# Two things tried and ruled out already (see git log for this file):
# - gdb -ex run alone: only traces the top-level ostserver process and
#   detaches from every forked child, catches nothing if the crash is
#   in a driver subprocess (indi_simulator_ccd/_guide) or indiserver.
# - ulimit -c unlimited + scanning for a core file afterwards: this
#   runner's core_pattern redirects wherever the HOST's does, which a
#   container has no visibility into - confirmed, nothing ever appears.
#
# This time: gdb with detach-on-fork off, so it keeps tracing every
# forked child too (as separate "inferiors") instead of letting go of
# them - "run" then stops on a crash anywhere in the whole tree, and
# the python loop below walks every inferior and backtraces whichever
# one is actually sitting on a fatal signal.
cat > /tmp/crash.gdb << 'EOF'
set detach-on-fork off
set schedule-multiple on
run
python
import gdb
infs = gdb.inferiors()
print("=== %d inferior(s) alive when 'run' stopped ===" % len(infs))
for inf in infs:
    print("--- inferior %d, pid %s ---" % (inf.num, inf.pid))
    try:
        gdb.execute("inferior %d" % inf.num)
        gdb.execute("thread apply all bt full")
    except Exception as e:
        print("(error backtracing inferior %d: %s)" % (inf.num, e))
end
quit
EOF
gdb -batch -x /tmp/crash.gdb --args ostserver --webroot=media --indiserver=Y \
    > server.log 2>&1 &
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
