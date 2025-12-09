#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>  // Add this for send()

#include "../include/bank.h"

#define THREAD_POOL_SIZE 10
#define TASK_QUEUE_SIZE 1000
#define BUFFER_SIZE 1024

// External protocol function
extern void execute_command(const char *input, char *response, size_t resp_size);

// Task structure to hold client FD and command data
typedef struct {
    int client_fd;
    char command[256];
} Task;

// Thread pool state
typedef struct {
    Task queue[TASK_QUEUE_SIZE];
    int head;
    int tail;
    int count;
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_not_empty;
    pthread_cond_t queue_not_full;
    pthread_t workers[THREAD_POOL_SIZE];
    int shutdown;
} ThreadPool;

static ThreadPool thread_pool = {0};

// Worker thread function
void* worker_thread(void *arg) {
    (void)arg; // Unused parameter
    
    while (1) {
        pthread_mutex_lock(&thread_pool.queue_lock);
        
        // Wait while queue is empty (and not shutting down)
        while (thread_pool.count == 0 && !thread_pool.shutdown) {
            pthread_cond_wait(&thread_pool.queue_not_empty, &thread_pool.queue_lock);
        }
        
        // Check if we should exit
        if (thread_pool.shutdown && thread_pool.count == 0) {
            pthread_mutex_unlock(&thread_pool.queue_lock);
            break;
        }
        
        // Fetch task from queue
        Task task = thread_pool.queue[thread_pool.head];
        thread_pool.head = (thread_pool.head + 1) % TASK_QUEUE_SIZE;
        thread_pool.count--;
        
        // Signal that queue is not full
        pthread_cond_signal(&thread_pool.queue_not_full);
        pthread_mutex_unlock(&thread_pool.queue_lock);
        
        // Process the task: execute command and send response
        char response[BUFFER_SIZE];
        execute_command(task.command, response, sizeof(response));
        
        printf("[Worker] Processing task from FD %d: %s -> %s", 
               task.client_fd, task.command, response);
        
        // Send response back to client
        if (send(task.client_fd, response, strlen(response), 0) < 0) {
            perror("send");
            printf("[Worker] Failed to send response to FD %d\n", task.client_fd);
        }
    }
    
    return NULL;
}

// Initialize the thread pool
void thread_pool_init(int num_workers) {
    if (num_workers > THREAD_POOL_SIZE) num_workers = THREAD_POOL_SIZE;
    
    thread_pool.head = 0;
    thread_pool.tail = 0;
    thread_pool.count = 0;
    thread_pool.shutdown = 0;
    
    pthread_mutex_init(&thread_pool.queue_lock, NULL);
    pthread_cond_init(&thread_pool.queue_not_empty, NULL);
    pthread_cond_init(&thread_pool.queue_not_full, NULL);
    
    for (int i = 0; i < num_workers; i++) {
        pthread_create(&thread_pool.workers[i], NULL, worker_thread, NULL);
    }
    
    printf("[ThreadPool] Initialized with %d workers\n", num_workers);
}

// Submit a task to the queue
int submit_task(int client_fd, const char *command) {
    pthread_mutex_lock(&thread_pool.queue_lock);
    
    // Wait while queue is full
    while (thread_pool.count >= TASK_QUEUE_SIZE) {
        pthread_cond_wait(&thread_pool.queue_not_full, &thread_pool.queue_lock);
    }
    
    // Add task to queue
    Task *task = &thread_pool.queue[thread_pool.tail];
    task->client_fd = client_fd;
    strncpy(task->command, command, sizeof(task->command) - 1);
    task->command[sizeof(task->command) - 1] = '\0';
    
    thread_pool.tail = (thread_pool.tail + 1) % TASK_QUEUE_SIZE;
    thread_pool.count++;
    
    // Signal that queue is not empty
    pthread_cond_signal(&thread_pool.queue_not_empty);
    pthread_mutex_unlock(&thread_pool.queue_lock);
    
    return 0;
}

// Gracefully shutdown the thread pool
void thread_pool_shutdown() {
    pthread_mutex_lock(&thread_pool.queue_lock);
    thread_pool.shutdown = 1;
    pthread_cond_broadcast(&thread_pool.queue_not_empty);
    pthread_mutex_unlock(&thread_pool.queue_lock);
    
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_join(thread_pool.workers[i], NULL);
    }
    
    pthread_mutex_destroy(&thread_pool.queue_lock);
    pthread_cond_destroy(&thread_pool.queue_not_empty);
    pthread_cond_destroy(&thread_pool.queue_not_full);
}
