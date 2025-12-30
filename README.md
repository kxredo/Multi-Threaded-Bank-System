# Multi-Threaded Bank System

A client-server banking system in C that demonstrates the performance difference between single-threaded and multi-threaded request processing. The server handles concurrent clients using a thread pool and non-blocking I/O, while the client provides an interactive interface with built-in benchmarking.

The project serves as a practical example of systems programming concepts: socket communication, synchronization primitives, and high-performance server architecture.

## Features

- **Runtime mode switching** — Toggle between single-threaded and multi-threaded processing without restarting the server
- **Built-in stress testing** — Spawn concurrent clients to measure throughput directly from the interactive client
- **Thread pool with task queue** — Workers process requests in parallel using a producer-consumer pattern
- **Per-account locking** — Fine-grained mutex locks prevent contention while allowing concurrent access to different accounts
- **Deadlock avoidance** — Transfer operations acquire locks in a strict order based on account ID

## Techniques

### Non-blocking I/O with epoll
The server uses [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html) to monitor multiple client connections without blocking. This reactor pattern handles thousands of connections efficiently by multiplexing I/O events in a single thread, then dispatching work to the thread pool.

### Condition Variables for Thread Coordination
The thread pool uses [pthread condition variables](https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html) to put workers to sleep when the queue is empty. This avoids busy-waiting and allows efficient CPU utilization compared to polling.

### Lock Ordering for Deadlock Prevention
When transferring between two accounts, the code always acquires locks in ascending order by account ID. This prevents circular wait conditions that cause deadlocks in concurrent systems.

### Simulated Processing Delay
A configurable delay in [src/protocol.c](src/protocol.c) simulates real-world latency (database access, validation). This makes the threading performance difference visible during benchmarks.

## Technologies

| Technology | Purpose |
|------------|---------|
| [POSIX Threads](https://man7.org/linux/man-pages/man7/pthreads.7.html) | Thread creation, mutexes, condition variables |
| [epoll](https://man7.org/linux/man-pages/man7/epoll.7.html) | Scalable I/O event notification |
| [BSD Sockets](https://man7.org/linux/man-pages/man7/socket.7.html) | TCP client-server communication |
| [clock_gettime](https://man7.org/linux/man-pages/man3/clock_gettime.3.html) | High-resolution timing for benchmarks |

## Project Structure

```
├── Makefile
├── README.md
├── include/
│   ├── bank.h
│   ├── logger.h
│   ├── protocol.h
│   └── thread_pool.h
└── src/
    ├── client.c
    ├── logger.c
    ├── protocol.c
    ├── server.c
    ├── stress_client.c
    ├── thread_pool.c
    └── transactions.c
```

- **[include/](include/)** — Header files defining data structures for accounts, protocol commands, and thread pool interface
- **[src/server.c](src/server.c)** — Main server with epoll reactor and threading mode toggle
- **[src/client.c](src/client.c)** — Interactive TUI client with built-in stress testing
- **[src/thread_pool.c](src/thread_pool.c)** — Worker threads and task queue implementation
- **[src/transactions.c](src/transactions.c)** — Banking operations with mutex-protected accounts
- **[src/protocol.c](src/protocol.c)** — Command parsing and execution
- **[src/stress_client.c](src/stress_client.c)** — Standalone benchmark utility

## Building

```bash
make clean && make
```

## Running

```bash
# Terminal 1: Start server (multi-threaded by default)
./server

# Terminal 2: Start interactive client
./client
```

Use menu options 7 and 8 to switch between single-threaded and multi-threaded modes. Each switch runs an automatic stress test showing the throughput difference.
