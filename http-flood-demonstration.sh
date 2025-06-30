#!/bin/bash

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <build_dir> [total_requests] [requests_per_second]"
    exit 1
fi

if [[ "$1" == "--help" ]]; then
    echo "Usage: $0 <build_dir> [total_requests] [requests_per_second]"
    echo "Example: $0 ../build/ 100 10"
    echo
    echo "Runs a demonstration of HTTP flood and IP blacklisting against the realtime-server."
    echo
    echo "Arguments:"
    echo "  <build_dir>         Path to the build directory containing realtime-server and config.ini"
    echo "  [total_requests]    Total number of requests to send in the flood (default: 50)"
    echo "  [requests_per_second] Number of requests to send per second (default: 10)"
    echo
    exit 0
fi

BUILD_DIR="$1"
BUILD_DIR="${BUILD_DIR%/}"

TOTAL_REQUESTS="${2:-50}"
REQUESTS_PER_SECOND="${3:-10}"

REALTIME_SERVER_BIN="$BUILD_DIR/realtime-server/bin/server"
if [ ! -f "$REALTIME_SERVER_BIN" ]; then
    echo "Error: Realtime server binary not found at $REALTIME_SERVER_BIN"
    exit 1
fi

REALTIME_SERVER_INI="$BUILD_DIR/realtime-server/bin/config.ini"
if [ ! -f "$REALTIME_SERVER_INI" ]; then
    echo "Error: Config file not found at $REALTIME_SERVER_INI"
    exit 1
fi

# Extract only the [mysql] section
MYSQL_SECTION=$(awk '/^\[mysql\]/ {flag=1; next} /^\[/ {flag=0} flag' "$REALTIME_SERVER_INI")
if [ -z "$MYSQL_SECTION" ]; then
    echo "Error: [mysql] section not found in $REALTIME_SERVER_INI"
    exit 1
fi

MYSQL_HOST=$(echo "$MYSQL_SECTION" | awk -F' *= *' '/^host/ {print $2}')
MYSQL_PORT=$(echo "$MYSQL_SECTION" | awk -F' *= *' '/^port/ {print $2}')
MYSQL_USER=$(echo "$MYSQL_SECTION" | awk -F' *= *' '/^username/ {print $2}')
MYSQL_PASS=$(echo "$MYSQL_SECTION" | awk -F' *= *' '/^password/ {print $2}')
MYSQL_DB=$(echo "$MYSQL_SECTION" | awk -F' *= *' '/^database/ {print $2}')

run_curl() {
    local CURL_CMD="$1"
    echo "> $CURL_CMD"
    echo
    eval "$CURL_CMD"
    echo
}

echo "Printing all contents of 'flights' table (before starting server):"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM flights;"
echo

echo "Starting realtime-server..."
"$REALTIME_SERVER_BIN" &
REALTIME_SERVER_PID=$!
sleep 1

echo "Printing all contents of 'flights' table (after starting server):"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM flights;"
echo

CURL_FLIGHTS_REQUEST="curl -X GET -H 'Content-Type: application/json' -H 'Authorization: Basic QnVsZ2FyaWFBaXI6cGFzc3dvcmQy' http://localhost:8081/flights"
echo "Sending GET request to /flights:"
run_curl "$CURL_FLIGHTS_REQUEST"

# Measure response time for a single request before the attack
echo "Measuring response time for a single request before the attack:"
time eval "$CURL_FLIGHTS_REQUEST" > /dev/null 2>&1

echo "Starting HTTP flood against /flights endpoint..."
sent=0
while [ $sent -lt $TOTAL_REQUESTS ]; do
    for i in $(seq 1 $REQUESTS_PER_SECOND); do
        if [ $sent -ge $TOTAL_REQUESTS ]; then
            break
        fi
        eval "$CURL_FLIGHTS_REQUEST" > /dev/null 2>&1 &
        sent=$((sent + 1))
    done
    echo "Sent $sent requests..."
    sleep 1
done

# Measure response time for a single request during the attack
echo "Measuring response time for a single request during the attack:"
time eval "$CURL_FLIGHTS_REQUEST" > /dev/null 2>&1



echo "Blacklisting 127.0.0.1 in realtime-server config.ini..."
if grep -q '^blacklisted_ips=' "$REALTIME_SERVER_INI"; then
    sed -i 's/^blacklisted_ips=.*/blacklisted_ips=127.0.0.1/' "$REALTIME_SERVER_INI"
else
    echo "blacklisted_ips=127.0.0.1" >> "$REALTIME_SERVER_INI"
fi

echo "Setting thread_pool_size=2 in realtime-server config.ini..."
if grep -q '^thread_pool_size=' "$REALTIME_SERVER_INI"; then
    sed -i 's/^thread_pool_size=.*/thread_pool_size=2/' "$REALTIME_SERVER_INI"
else
    sed -i '/^\[global\]/a thread_pool_size=2' "$REALTIME_SERVER_INI"
fi

echo "Restarting realtime-server to apply blacklist and new thread pool size..."
kill $REALTIME_SERVER_PID
wait $REALTIME_SERVER_PID 2>/dev/null || true
"$REALTIME_SERVER_BIN" &
REALTIME_SERVER_PID=$!
sleep 1

echo "Sending GET request to /flights from blacklisted IP (should be forbidden):"
run_curl "$CURL_FLIGHTS_REQUEST"

echo "Removing blacklist for new flood attack..."
sed -i 's/^blacklisted_ips=.*/blacklisted_ips=0/' "$REALTIME_SERVER_INI"

echo "Restarting realtime-server to remove blacklist..."
kill $REALTIME_SERVER_PID
wait $REALTIME_SERVER_PID 2>/dev/null || true
"$REALTIME_SERVER_BIN" &
REALTIME_SERVER_PID=$!
sleep 2  # Increased sleep to ensure server is ready

echo "Starting HTTP flood against /flights endpoint with thread_pool_size=2..."
sent=0
while [ $sent -lt $TOTAL_REQUESTS ]; do
    for i in $(seq 1 $REQUESTS_PER_SECOND); do
        if [ $sent -ge $TOTAL_REQUESTS ]; then
            break
        fi
        eval "$CURL_FLIGHTS_REQUEST" > /dev/null 2>&1 &
        sent=$((sent + 1))
    done
    echo "Sent $sent requests..."
    sleep 1
done

# Measure response time for a single request during the attack (thread_pool_size=2)
echo "Measuring response time for a single request during the attack (thread_pool_size=2):"
time eval "$CURL_FLIGHTS_REQUEST" > /dev/null 2>&1

echo "Stopping realtime-server (PID $REALTIME_SERVER_PID)..."
kill $REALTIME_SERVER_PID
wait $REALTIME_SERVER_PID 2>/dev/null || true
echo "Done."
