🗺️ C Multi-Threaded Bank Project Roadmap
This detailed, eight-phase plan outlines the development of a high-performance, multi-threaded banking system in C. It focuses on advanced system programming concepts to ensure the project is scalable, robust, and stands out.

Phase 0: Foundation & Core Data Structures (bank.h)
Goal: Define the atomic, self-protecting data structure and global state.

Task 0.1 - Account Structure: Define the Account struct including id, balance, and pthread_mutex_t lock. (Stand Out Feature: Fine-Grained Locking - Mutex per account)

Task 0.2 - Alignment: Add padding to the Account struct to ensure it aligns with CPU cache lines, preventing false sharing. (Stand Out Feature: Cache Line Alignment Micro-optimization)

Task 0.3 - Global State: Declare Account *bank[MAX_ACCOUNTS], pthread_mutex_t bank_state_lock, and next_account_id. The global lock protects the array/counter access.

Task 0.4 - Initialization: Implement init_bank() to set all array pointers to NULL and initialize all global locks.

Phase 1: Deadlock-Proof Transaction Logic (transactions.c)
Goal: Implement the business logic for money movement with guaranteed thread safety.

Task 1.1 - Account Creation: Implement create_account(). This function must acquire the bank_state_lock, malloc a new Account struct, initialize its internal mutex, and store the pointer in the global bank array. (Stand Out Feature: Dynamic Account Creation)

Task 1.2 - Deposit / Withdraw: Implement deposit() and withdraw(). These operations only require locking the target account's mutex before the balance change and unlocking afterward.

Task 1.3 - Transfer: Implement transfer(). This function is critical: it must determine the two accounts and lock them based on a strict ordering rule (e.g., ascending id or memory address) to prevent circular dependencies. (Stand Out Feature: Deadlock Avoidance via Strict Lock Ordering)

Task 1.4 - Error Codes: Ensure all transaction functions return clear error codes (e.g., -1 for failure, 1 for success) for protocol use.

Phase 2: Concurrency Engine (threadpool.c)
Goal: Implement the worker thread architecture for scalable, CPU-efficient request processing.

Task 2.1 - Structures: Define the Task struct (to hold client FD and command data) and a fixed-size circular array for the task queue.

Task 2.2 - Synchronization: Initialize queue_lock (mutex), queue_not_empty (condition variable), and queue_not_full (condition variable) to manage queue access.

Task 2.3 - Worker Logic: Implement worker_thread(). The worker should loop indefinitely, calling fetch_task(). It uses pthread_cond_wait() when the queue is empty. (Stand Out Feature: CPU-Efficient Wait using Condition Variables)

Task 2.4 - Submission: Implement submit_task(): Pushes a task to the queue and uses pthread_cond_signal() to instantly wake up a sleeping worker.

Phase 3: Network Server (The Reactor) (server.c)
Goal: Create the high-performance core that handles many connections using non-blocking I/O.

Task 3.1 - Setup: Initialize the TCP Socket, bind to the server port (e.g., 8080), and listen for connections.

Task 3.2 - epoll Initialization: Create an epoll instance using epoll_create1(). Add the listening socket's File Descriptor (FD) to epoll for monitoring connection requests. (Stand Out Feature: Non-blocking I/O using epoll)

Task 3.3 - Main Loop: Implement the Reactor loop: Call epoll_wait() to block until a network event occurs.

Task 3.4 - New Connection: If an event is a new connection, accept() the FD, set the new client FD to non-blocking mode, and add it to the epoll instance.

Task 3.5 - Data Event: If data arrives on a client FD, read the full request and call submit_task() (from Phase 2), passing the client FD to the Thread Pool. (Stand Out Feature: Decoupling I/O from Processing)

Phase 4: Communication Protocol & Parsing (protocol.c)
Goal: Define and implement the message language for client-server interaction.

Task 4.1 - Protocol Definition: Define the simple text-based protocol (e.g., CREATE, DEPOSIT <ID> <AMOUNT>).

Task 4.2 - Parsing Function: Implement a robust function to parse the incoming command string (e.g., using strtok or sscanf) and validate arguments.

Task 4.3 - Request Execution: Modify the worker_thread to use the parser, map the command to the correct transaction function, and execute it.

Task 4.4 - Response: Format the execution result (e.g., SUCCESS <NEW_BALANCE> or FAILURE <ERROR_CODE>) and send it back to the client FD.

Phase 5: TUI Client Implementation (client.c)
Goal: Implement the user-friendly command-line interface.

Task 5.1 - TUI: Implement display_welcome() and display_menu() functions for a clean interface as requested.

Task 5.2 - Network Connect: Implement setup_connection() using standard Client Sockets to establish a connection with the server.

Task 5.3 - Command Handlers: Implement handle_create_account(), handle_deposit(), etc. These functions gather user input and format the protocol string (e.g., using sprintf).

Task 5.4 - Network I/O: Use send() to transmit the formatted command to the server and recv() to wait for and display the server's response.

Phase 6: Data Persistence & Reliability (persistence.c)
Goal: Ensure the bank state is saved efficiently and can recover from crashes.

Task 6.1 - mmap Save/Load: Implement functions to save and load the entire bank structure to a binary file using Memory Mapped Files (mmap). (Stand Out Feature: High-Performance Persistence with mmap)

Task 6.2 - Journaling Logic: Create a simple Write-Ahead Log (WAL) structure. Before any multi-step transaction (like transfer), write the intended operation to a log file. (Stand Out Feature: Simplified Transaction Atomicity / WAL)

Task 6.3 - Crash Recovery: On server startup, implement logic to check the WAL file. If pending operations are found, execute them to restore data consistency, then clear the log.

Phase 7: Finalization, Testing, & Documentation
Goal: Demonstrate the system's performance and ensure professional presentation.

Task 7.1 - Makefile: Create a robust Makefile to compile all components (server, client, transactions, etc.) with appropriate flags (-Wall, -Wextra, -pthread).

Task 7.2 - Stress Client: Develop a simple utility that spawns numerous threads to bombard the server with simultaneous transfers and transactions. (Stand Out Feature: Proof of Concurrency/Stability under Stress)

Task 7.3 - Benchmarking: Integrate timing functions (gettimeofday or clock_gettime) to measure and report the system's average transaction throughput (transactions/second).

Task 7.4 - Cleanup: Implement graceful shutdown logic: catch SIGINT (Ctrl+C), close the epoll FD, join all worker threads, save the persistent data, and exit cleanly. (Stand Out Feature: Graceful Shutdown and Signal Handling)