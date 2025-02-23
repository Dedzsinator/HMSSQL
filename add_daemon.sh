#!/bin/bash

# Configuration
DAEMON_NAME="hmssql_daemon"
DAEMON_PATH="/usr/local/bin/$DAEMON_NAME"
BUILD_PATH="../../build/bin/daemon"
PID_FILE="/var/run/$DAEMON_NAME.pid"

# Check if running as root
if [ "$EUID" -ne 0 ]; then 
    echo "Please run as root"
    exit 1
fi

# Stop existing daemon
if [ -f "$PID_FILE" ]; then
    echo "Stopping existing daemon..."
    OLD_PID=$(cat "$PID_FILE")
    if kill -0 "$OLD_PID" 2>/dev/null; then
        kill "$OLD_PID"
        sleep 2
        if kill -0 "$OLD_PID" 2>/dev/null; then
            kill -9 "$OLD_PID"
        fi
    fi
    rm -f "$PID_FILE"
fi

# Remove old binary
if [ -f "$DAEMON_PATH" ]; then
    echo "Removing old binary..."
    rm -f "$DAEMON_PATH"
fi

# Copy new binary
echo "Installing new binary..."
cp "$BUILD_PATH" "$DAEMON_PATH"
chmod 755 "$DAEMON_PATH"

# Start new daemon
echo "Starting new daemon..."
"$DAEMON_PATH" &
NEW_PID=$!
echo $NEW_PID > "$PID_FILE"

# Check if daemon is running
sleep 2
if kill -0 "$NEW_PID" 2>/dev/null; then
    echo "Daemon successfully started with PID: $NEW_PID"
else
    echo "Failed to start daemon"
    exit 1
fi

echo "Daemon installation complete"