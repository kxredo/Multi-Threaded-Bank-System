// stress_client.c - Multi-Threaded Stress Test Client
// ============================================================================
// This client spawns multiple threads to simulate concurrent bank clients,
// measuring throughput to demonstrate single-threaded vs multi-threaded
// server performance difference.
// ============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

// ============================================================================
// STRESS TEST CONFIGURATION
// ============================================================================
#define NUM_CLIENTS       10     // Number of concurrent client threads
#define OPS_PER_CLIENT    20    // Operations each client performs
// ============================================================================

// Statistics
static pthread_mutex_t stats_lock = PTHREAD_MUTEX_INITIALIZER;
static int total_success = 0;
static int total_failure = 0;
static int accounts_created = 0;

// Thread argument structure
typedef struct {
    int thread_id;
    int ops_completed;
    int ops_failed;
} ClientArgs;

// Get current time in seconds (high precision)
double get_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// Connect to server
int connect_to_server(void) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_HOST, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }
    
    return sock;
}

// Send command and receive response
int send_command(int sock, const char *cmd, char *response, size_t resp_size) {
    if (send(sock, cmd, strlen(cmd), 0) < 0) {
        return -1;
    }
    
    int n = recv(sock, response, resp_size - 1, 0);
    if (n <= 0) {
        return -1;
    }
    
    response[n] = '\0';
    return (strncmp(response, "SUCCESS", 7) == 0) ? 0 : -1;
}

// Client thread function
void* client_thread(void *arg) {
    ClientArgs *args = (ClientArgs *)arg;
    char cmd[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    // Each client gets its own connection
    int sock = connect_to_server();
    if (sock < 0) {
        printf("[Client %d] Failed to connect\n", args->thread_id);
        return NULL;
    }
    
    // Create an account for this client
    snprintf(cmd, sizeof(cmd), "CREATE\n");
    if (send_command(sock, cmd, response, sizeof(response)) == 0) {
        pthread_mutex_lock(&stats_lock);
        accounts_created++;
        pthread_mutex_unlock(&stats_lock);
    }
    
    // Parse account ID from response (e.g., "SUCCESS CREATE 5")
    int my_account = -1;
    sscanf(response, "SUCCESS CREATE %d", &my_account);
    
    if (my_account < 0) {
        printf("[Client %d] Failed to create account\n", args->thread_id);
        close(sock);
        return NULL;
    }
    
    printf("[Client %2d] Created account #%d\n", args->thread_id, my_account);
    
    // Perform operations
    for (int i = 0; i < OPS_PER_CLIENT; i++) {
        int op = rand() % 3;  // 0=deposit, 1=withdraw, 2=balance
        int success = 0;
        double amount;
        const char *op_name;
        
        switch (op) {
            case 0:  // Deposit
                amount = (rand() % 1000) + 1.0;
                snprintf(cmd, sizeof(cmd), "DEPOSIT %d %.2f\n", my_account, amount);
                success = (send_command(sock, cmd, response, sizeof(response)) == 0);
                op_name = "DEPOSIT";
                break;
                
            case 1:  // Withdraw (small amount to avoid insufficient funds)
                amount = (rand() % 10) + 1.0;
                snprintf(cmd, sizeof(cmd), "WITHDRAW %d %.2f\n", my_account, amount);
                success = (send_command(sock, cmd, response, sizeof(response)) == 0);
                op_name = "WITHDRAW";
                break;
                
            case 2:  // Balance check
                amount = 0;
                snprintf(cmd, sizeof(cmd), "BALANCE %d\n", my_account);
                success = (send_command(sock, cmd, response, sizeof(response)) == 0);
                op_name = "BALANCE";
                break;
                
            default:
                op_name = "UNKNOWN";
                break;
        }
        
        // Trim newline from response for cleaner output
        char *newline = strchr(response, '\n');
        if (newline) *newline = '\0';
        
        printf("[Client %2d] Op %2d: %-8s -> %s\n", 
               args->thread_id, i + 1, op_name, response);
        
        if (success) {
            args->ops_completed++;
        } else {
            args->ops_failed++;
        }
    }
    
    close(sock);
    
    // Update global stats
    pthread_mutex_lock(&stats_lock);
    total_success += args->ops_completed;
    total_failure += args->ops_failed;
    pthread_mutex_unlock(&stats_lock);
    
    printf("[Client %d] Completed: %d ops, Failed: %d ops\n", 
           args->thread_id, args->ops_completed, args->ops_failed);
    
    return NULL;
}

int main(int argc, char *argv[]) {
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s\n", argv[0]);
            printf("\nStress test the bank server with %d clients x %d operations.\n", NUM_CLIENTS, OPS_PER_CLIENT);
            return 0;
        }
    }
    
    srand(time(NULL));
    
    printf("============================================================\n");
    printf("  BANK SERVER STRESS TEST\n");
    printf("============================================================\n");
    printf("  Clients:          %d\n", NUM_CLIENTS);
    printf("  Ops per client:   %d\n", OPS_PER_CLIENT);
    printf("  Total operations: %d\n", NUM_CLIENTS * OPS_PER_CLIENT);
    printf("============================================================\n\n");
    
    pthread_t threads[NUM_CLIENTS];
    ClientArgs args[NUM_CLIENTS];
    
    // Record start time
    double start_time = get_time_sec();
    
    // Spawn client threads
    printf("[Main] Spawning %d client threads...\n\n", NUM_CLIENTS);
    for (int i = 0; i < NUM_CLIENTS; i++) {
        args[i].thread_id = i;
        args[i].ops_completed = 0;
        args[i].ops_failed = 0;
        pthread_create(&threads[i], NULL, client_thread, &args[i]);
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Record end time
    double end_time = get_time_sec();
    double elapsed = end_time - start_time;
    
    // Print results
    int total_ops = total_success + total_failure;
    double throughput = total_ops / elapsed;
    
    printf("\n============================================================\n");
    printf("  BENCHMARK RESULTS\n");
    printf("============================================================\n");
    printf("  Accounts created: %d\n", accounts_created);
    printf("  Successful ops:   %d\n", total_success);
    printf("  Failed ops:       %d\n", total_failure);
    printf("  Total ops:        %d\n", total_ops);
    printf("------------------------------------------------------------\n");
    printf("  Total time:       %.2f seconds\n", elapsed);
    printf("  Throughput:       %.2f ops/sec\n", throughput);
    printf("============================================================\n");
    

    
    return 0;
}
