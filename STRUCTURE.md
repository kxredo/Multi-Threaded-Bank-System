# Project Structure - Cleaned Up

## What You Have Now

### Core Application
- **`server`** - Compiled multi-threaded server binary
- **`client`** - Compiled interactive client binary
- **`src/`** - Source code (server.c, client.c, transactions.c, thread_pool.c, protocol.c, logger.c)
- **`include/`** - Header files (bank.h, thread_pool.h, protocol.h, logger.h)
- **`Makefile`** - Build system

### Demo Scripts
- **`run.sh`** - Interactive testing (opens server + client in separate terminals)
- **`demo.sh`** - Automated demo showing concurrent transfers and thread safety
- **`client1.sh`** - Setup script (creates accounts, used by demo.sh)
- **`README.md`** - Project documentation

### Other
- **`CONTEXT.md`** - Original project roadmap

---

## How to Use

### Build the Project
```bash
make clean && make all
```

### Interactive Testing (Manual)
```bash
./run.sh
```
Opens server and client in separate terminals. You manually test operations.

### Automated Demo (Recommended for Presentations)
```bash
./demo.sh
```
Automatically:
1. Starts server
2. Creates 2 accounts with initial deposits
3. Performs 3 simultaneous transfers
4. Shows final balances

**Expected Output:**
```
Account 0: $900.00  (started with $1000, transferred $300)
Account 1: $800.00  (started with $500, received $300)
```

If balances are correct → **Thread safety is working!**

---

## What Was Deleted (No Longer Needed)
- `client2.sh`, `client3.sh`, `client4.sh`, `client5.sh` - Merged into demo.sh
- `client_st.sh` - Single-threaded demo (too complex)
- `stress_test.sh` - Overly complex
- `run_demo.sh` - Redundant
- `demo_comparison.sh` - Redundant
- `server_single_threaded.c` - Not needed

---

## Key Features to Highlight

✅ **Multi-threaded**: 10 worker threads via thread pool  
✅ **Thread-safe**: Fine-grained per-account locking  
✅ **Deadlock-free**: Strict lock ordering on transfers  
✅ **Scalable I/O**: epoll reactor pattern  
✅ **Concurrent transfers**: Multiple clients operate simultaneously  

---

## Command Quick Reference

```bash
# Build
make clean && make all

# Interactive demo
./run.sh

# Automated demo (shows concurrency)
./demo.sh

# Clean up
make distclean
```
