#!/bin/bash

# Setup: Create Account 0 with $1000, Account 1 with $500
(
  sleep 0.5; echo "1"       # Create Account
  sleep 0.5; echo "2"       # Deposit
  sleep 0.5; echo "0"       # Account ID
  sleep 0.5; echo "1000"    # Amount
  sleep 0.5; echo "1"       # Create Account
  sleep 0.5; echo "2"       # Deposit
  sleep 0.5; echo "1"       # Account ID
  sleep 0.5; echo "500"     # Amount
  sleep 0.5; echo "6"       # Quit
) | ./client