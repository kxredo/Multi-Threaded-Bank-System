#!/bin/bash

# Interactive Demo - Opens server and client in separate terminals

echo "Starting server in new terminal..."
gnome-terminal -- bash -c "./server; sleep 5" &

sleep 2

echo "Starting client in new terminal..."
gnome-terminal -- bash -c "./client; sleep 5" &

echo "Done! Perform operations manually."
wait
