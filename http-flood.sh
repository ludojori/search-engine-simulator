#!/bin/bash

# Usage: bash http-flood.sh <URL> <REQUEST_RATE> <METHOD> [BODY_PATH]
# Example: bash http-flood.sh 192.168.1.100:8080/path/to/source 50 GET
# Example: bash http-flood.sh 192.168.1.100:8080/path/to/source 50 POST request_body.json

# Input Parameters
URL="$1"
REQUEST_RATE="$2"
METHOD="$3"
BODY_PATH="$4"

# Validate Input
if [[ -z "$URL" || -z "$REQUEST_RATE" || -z "$METHOD" ]]; then
    echo "Usage: $0 <URL> <REQUEST_RATE> <METHOD> [BODY_PATH]"
    exit 1
fi

if [[ "$METHOD" != "GET" && "$METHOD" != "POST" ]]; then
    echo "Error: METHOD must be GET or POST."
    exit 1
fi

# Ensure curl is installed
if ! command -v curl &> /dev/null; then
    echo "Error: curl is not installed. Please install it and try again."
    exit 1
fi

# Function to perform HTTP requests
send_request() {
    if [[ "$METHOD" == "GET" ]]; then
        curl -s -o /dev/null -w "Status: %{http_code}\n" --cacert server.crt "$URL"
    elif [[ "$METHOD" == "POST" ]]; then
        if [[ -f "$BODY_PATH" ]]; then
            curl -s -o /dev/null -w "Status: %{http_code}\n" -X POST --cacert server.crt -H "Content-Type: application/json" --data @"$BODY_PATH" "$URL"
        else
            echo "Error: POST body file not found at $BODY_PATH"
            exit 1
        fi
    fi
}

# Main loop for load generation
echo "Starting load test: $METHOD requests to $URL at $REQUEST_RATE requests/second"
while true; do
    for ((i = 0; i < REQUEST_RATE; i++)); do
        send_request &
    done
    sleep 1
done
