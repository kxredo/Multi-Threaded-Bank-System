#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "../include/bank.h"

#define SERVER_PORT 8080
#define MAX_EVENTS 1000
#define BUFFER_SIZE 1024

// External functions from thread_pool.c
extern void thread_pool_init(int num_workers);
extern int submit_task(int client_fd, const char *command);
extern void thread_pool_shutdown();

// External functions from transactions.c
extern void init_bank();

static int server_fd;
static int epoll_fd;
static volatile int running = 1;

// Set socket to non-blocking mode
void set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Signal handler for graceful shutdown
void signal_handler(int sig) {
    if (sig == SIGINT) {
        running = 0;
        printf("\n[Server] Shutting down...\n");
    }
}

// Initialize the server socket
int server_init() {
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }
    
    // Allow address reuse
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(SERVER_PORT);
    
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        return -1;
    }
    
    if (listen(server_fd, 128) < 0) {
        perror("listen");
        return -1;
    }
    
    set_nonblocking(server_fd);
    printf("[Server] Listening on port %d\n", SERVER_PORT);
    
    return 0;
}

// Initialize epoll
int epoll_init() {
    epoll_fd = epoll_create1(0);
    if (epoll_fd < 0) {
        perror("epoll_create1");
        return -1;
    }
    
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) < 0) {
        perror("epoll_ctl");
        return -1;
    }
    
    return 0;
}

// Handle new client connection
void handle_new_connection() {
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_fd < 0) {
        perror("accept");
        return;
    }
    
    set_nonblocking(client_fd);
    
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = client_fd;
    
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
        perror("epoll_ctl");
        close(client_fd);
        return;
    }
    
    printf("[Server] New client connected: FD %d from %s:%d\n", client_fd,
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
}

// Handle incoming data from client
void handle_client_data(int client_fd) {
    char buffer[BUFFER_SIZE];
    int n = read(client_fd, buffer, sizeof(buffer) - 1);
    
    if (n <= 0) {
        // Connection closed or error
        printf("[Server] Client FD %d disconnected\n", client_fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        close(client_fd);
        return;
    }
    
    buffer[n] = '\0';
    printf("[Server] Received from FD %d: %s\n", client_fd, buffer);
    
    // Submit task to thread pool
    submit_task(client_fd, buffer);
}

// Main reactor loop
void reactor_loop() {
    struct epoll_event events[MAX_EVENTS];
    
    while (running) {
        int nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        
        if (nfds < 0) {
            if (running) perror("epoll_wait");
            continue;
        }
        
        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == server_fd) {
                // New connection
                handle_new_connection();
            } else {
                // Data from existing client
                handle_client_data(events[i].data.fd);
            }
        }
    }
}

// Cleanup resources
void server_cleanup() {
    if (epoll_fd >= 0) close(epoll_fd);
    if (server_fd >= 0) close(server_fd);
    
    printf("[Server] Cleanup complete\n");
}

// Main server function
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    signal(SIGINT, signal_handler);
    
    // Initialize bank
    init_bank();
    printf("[Server] Bank initialized\n");
    
    // Initialize thread pool
    thread_pool_init(10);
    printf("[Server] Thread pool initialized with 10 workers\n");
    
    // Initialize server socket
    if (server_init() < 0) {
        return 1;
    }
    
    // Initialize epoll
    if (epoll_init() < 0) {
        server_cleanup();
        return 1;
    }
    
    // Run reactor loop
    printf("[Server] Starting reactor loop\n");
    reactor_loop();
    
    // Shutdown
    thread_pool_shutdown();
    server_cleanup();
    
    return 0;
}
