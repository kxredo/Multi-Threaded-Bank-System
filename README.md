# Multi-Threaded Bank System

A high-performance banking system in C demonstrating advanced multithreading concepts.

## Quick Start

### Build
```bash
make clean && make all
```

### Run Interactive Demo (Manual Testing)
```bash
./run.sh
```
Opens server in one terminal and client in another. Manually test operations.

### Run Automated Demo (Concurrent Transfers)
```bash
./demo.sh
```
Automatically:
1. Creates 2 accounts with deposits
2. Performs 3 simultaneous transfers
3. Shows final balances (proves thread safety)

## Features

✅ **Fine-grained Locking**: Mutex per account  
✅ **Deadlock Avoidance**: Strict lock ordering  
✅ **Thread Pool**: 10 worker threads with condition variables  
✅ **Non-blocking I/O**: epoll-based reactor pattern  
✅ **Thread-Safe Transactions**: Deposit, Withdraw, Transfer  

## Architecture

- **server.c**: Multi-threaded TCP server with epoll reactor
- **thread_pool.c**: Worker thread pool with task queue
- **transactions.c**: Core banking logic with fine-grained locking
- **protocol.c**: Text-based command protocol
- **client.c**: Interactive TUI client

## Demo Output

Expected result after running `demo.sh`:
```
Account 0: $700.00  (started with $1000, transferred $300)
Account 1: $800.00  (started with $500, received $300)
```
If amounts are correct despite concurrent transfers → **Thread safety works!**
