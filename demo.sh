#!/bin/bash

# Automated Demo - Shows concurrent transfers with thread safety

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}Multi-Threaded Bank System - Demo${NC}"
echo -e "${BLUE}========================================${NC}\n"

cleanup() {
    pkill -f "./server" || true
    pkill -f "./client" || true
    sleep 1
}

trap cleanup EXIT

# Start server
echo -e "${BLUE}[1/4]${NC} Starting server..."
./server > /tmp/server.log 2>&1 &
sleep 2
echo -e "${GREEN}✓ Server started${NC}\n"

# Script 1: Create Account 0 and deposit $1000
echo -e "${BLUE}[2/4]${NC} Client 1: Creating Account 0 and depositing \$1000..."
(
  sleep 2; echo "1"       # Create Account
  sleep 2; echo "2"       # Deposit
  sleep 2; echo "0"       # Account ID
  sleep 2; echo "1000"    # Amount
  sleep 2; echo "5"       # Check Balance
  sleep 2; echo "6"       # Quit
) | ./client > /dev/null 2>&1
echo -e "${GREEN}✓ Done${NC}\n"

# Script 2: Create Account 1 and deposit $500
echo -e "${BLUE}[3/4]${NC} Client 2: Creating Account 1 and depositing \$500..."
(
  sleep 2; echo "1"       # Create Account
  sleep 2; echo "2"       # Deposit
  sleep 2; echo "1"       # Account ID
  sleep 2; echo "500"     # Amount
  sleep 2; echo "5"       # Check Balance
  sleep 2; echo "6"       # Quit
) | ./client > /dev/null 2>&1
echo -e "${GREEN}✓ Done${NC}\n"

# Scripts 3, 4, 5: Simultaneous transfers $0→$1 ($100 each)
echo -e "${BLUE}[4/4]${NC} Clients 3, 4, 5: Simultaneously transferring \$100 each (0→1)..."
(
  sleep 4; echo "4"         # Transfer
  sleep 2; echo "0"         # From Account 0
  sleep 2; echo "1"         # To Account 1
  sleep 2; echo "100"       # Amount
  sleep 2; echo "5"         # Check Balance
  sleep 2; echo "6"         # Quit
) | ./client > /dev/null 2>&1 &

(
  sleep 4; echo "4"         # Transfer
  sleep 2; echo "0"         # From Account 0
  sleep 2; echo "1"         # To Account 1
  sleep 2; echo "100"       # Amount
  sleep 2; echo "5"         # Check Balance
  sleep 2; echo "6"         # Quit
) | ./client > /dev/null 2>&1 &

(
  sleep 4; echo "4"         # Transfer
  sleep 2; echo "0"         # From Account 0
  sleep 2; echo "1"         # To Account 1
  sleep 2; echo "100"       # Amount
  sleep 2; echo "5"         # Check Balance
  sleep 2; echo "6"         # Quit
) | ./client > /dev/null 2>&1

wait
echo -e "${GREEN}✓ Done${NC}\n"

# Verify results
echo -e "${BLUE}========================================${NC}"
echo -e "${YELLOW}Final Result:${NC}"
echo -e "${BLUE}========================================${NC}\n"

(
  sleep 2; echo "5"       # Check Balance
  sleep 2; echo "6"       # Quit
) | ./client 2>/dev/null | grep -A 10 "Account ID"

echo ""
echo -e "${YELLOW}Expected:${NC}"
echo "  Account 0: \$900.00 (1000 - 300)"
echo "  Account 1: \$800.00 (500 + 300)"
echo ""
echo -e "${GREEN}✓ If amounts are correct → Thread safety works!${NC}"
echo ""