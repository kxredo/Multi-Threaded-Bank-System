// client.c - Multi-Threaded Bank Client
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdint.h>
#include <time.h>

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

// Stress test configuration
#define STRESS_NUM_CLIENTS    10
#define STRESS_OPS_PER_CLIENT 10

static int server_sock_fd = -1;
static pthread_mutex_t socket_mutex = PTHREAD_MUTEX_INITIALIZER;

void display_welcome() {
    printf("===========================================\n");
    printf(" WELCOME TO SHARAF BANK \n");
    printf("===========================================\n");
}

void display_menu() {
    printf("\n--- Main Menu ---\n");
    printf("1. Create Account\n");
    printf("2. Deposit\n");
    printf("3. Withdraw\n");
    printf("4. Transfer\n");
    printf("5. Check Balance\n");
    printf("6. Quit\n");
    printf("--- Simulation ---\n");
    printf("7. Run Single Threaded\n");
    printf("8. Run Multi Threaded\n");
    printf("Enter option [#]: ");
}

// Send command to server and receive response (thread-safe with mutex)
int send_command(const char *command) {
    pthread_mutex_lock(&socket_mutex);
    
    if (send(server_sock_fd, command, strlen(command), 0) < 0) {
        perror("send");
        pthread_mutex_unlock(&socket_mutex);
        return -1;
    }
    
    char response[BUFFER_SIZE];
    int n = recv(server_sock_fd, response, sizeof(response) - 1, 0);
    if (n < 0) {
        perror("recv");
        pthread_mutex_unlock(&socket_mutex);
        return -1;
    }
    
    response[n] = '\0';
    
    // Parse and display error/success messages
    if (strncmp(response, "SUCCESS", 7) == 0) {
        printf("Operation successful.\n");
    } else if (strncmp(response, "FAILURE", 7) == 0) {
        if (strstr(response, "INSUFFICIENT_FUNDS")) {
            printf("Error: Insufficient funds.\n");
        } else if (strstr(response, "ACCOUNT_NOT_FOUND")) {
            printf("Error: Account not found.\n");
        } else if (strstr(response, "INVALID_AMOUNT")) {
            printf("Error: Invalid amount.\n");
        } else if (strstr(response, "SAME_ACCOUNT")) {
            printf("Error: Cannot transfer to the same account.\n");
        } else {
            printf("Operation failed.\n");
        }
    }
    
    pthread_mutex_unlock(&socket_mutex);
    return 0;
}

// Display all accounts and their balances
void display_all_accounts() {
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "BALANCE_ALL\n");
    
    if (send(server_sock_fd, command, strlen(command), 0) < 0) {
        perror("send");
        return;
    }
    
    char response[BUFFER_SIZE * 5];
    memset(response, 0, sizeof(response));
    int n = recv(server_sock_fd, response, sizeof(response) - 1, 0);
    if (n < 0) {
        perror("recv");
        return;
    }
    
    response[n] = '\0';
    printf("\n%s\n", response);
}

void handle_create_account() {
    printf("\n[Creating new account...]\n");
    send_command("CREATE\n");
    display_all_accounts();
}

// Query server to check if account ID is valid
int is_valid_account_id(int id) {
    char command[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    
    snprintf(command, sizeof(command), "BALANCE %d\n", id);
    
    if (send(server_sock_fd, command, strlen(command), 0) < 0) {
        return 0;
    }
    
    int n = recv(server_sock_fd, response, sizeof(response) - 1, 0);
    if (n < 0) {
        return 0;
    }
    
    response[n] = '\0';
    return (strncmp(response, "SUCCESS", 7) == 0);
}

// Get valid account ID from user with validation
int get_valid_account_id(const char *prompt) {
    int id;
    while (1) {
        printf("%s", prompt);
        if (scanf("%d", &id) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }
        
        if (!is_valid_account_id(id)) {
            printf("Invalid Account ID. Please try again.\n");
            continue;
        }
        
        while (getchar() != '\n');
        return id;
    }
}

// Back-aware input helpers (prototypes) to avoid implicit declarations
static int read_line(char *buf, size_t sz);
static int is_back_token(const char *s);
static int prompt_account_id_or_back(const char *prompt, int *out_id);
static int prompt_amount_or_back(const char *prompt, double *out_amount);

void handle_deposit() {
    int id;
    double amount;
    char command[BUFFER_SIZE];

    if (!prompt_account_id_or_back("Enter Account ID to deposit to (or 'b' to go back): ", &id)) {
        printf("↩ Back to main menu.\n");
        return;
    }
    if (!prompt_amount_or_back("Enter Amount to deposit ($, or 'b' to go back): ", &amount)) {
        printf("↩ Back to main menu.\n");
        return;
    }

    snprintf(command, sizeof(command), "DEPOSIT %d %.2lf\n", id, amount);
    printf("[Sending command to Server: %s]", command);
    send_command(command);
    display_all_accounts();
}

void handle_withdraw() {
    int id;
    double amount;
    char command[BUFFER_SIZE];
    
    id = get_valid_account_id("Enter Account ID to withdraw from: ");
    
    printf("Enter Amount to withdraw: $");
    while (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("Invalid amount. Please enter a positive number: $");
        while (getchar() != '\n');
    }
    
    while (getchar() != '\n');
    
    snprintf(command, sizeof(command), "WITHDRAW %d %.2lf\n", id, amount);
    printf("[Sending command to Server: %s]", command);
    send_command(command);
    display_all_accounts();
}

void handle_transfer() {
    int from_id, to_id;
    double amount;
    char command[BUFFER_SIZE];

    if (!prompt_account_id_or_back("Enter Source Account ID (or 'b' to go back): ", &from_id)) {
        printf("↩ Back to main menu.\n");
        return;
    }

    while (1) {
        if (!prompt_account_id_or_back("Enter Destination Account ID (or 'b' to go back): ", &to_id)) {
            printf("↩ Back to main menu.\n");
            return;
        }
        if (from_id == to_id) {
            printf("Cannot transfer to the same account. Please choose a different destination.\n");
            continue;
        }
        break;
    }

    if (!prompt_amount_or_back("Enter Amount to transfer ($, or 'b' to go back): ", &amount)) {
        printf("↩ Back to main menu.\n");
        return;
    }

    snprintf(command, sizeof(command), "TRANSFER %d %d %.2lf\n", from_id, to_id, amount);
    printf("[Sending command to Server: %s]", command);
    send_command(command);
    display_all_accounts();
}

void handle_balance() {
    printf("\n--- All Account Balances ---\n");
    display_all_accounts();
}

// Setup connection to the server
int setup_connection() {
    server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock_fd < 0) {
        perror("socket");
        return -1;
    }
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    if (inet_pton(AF_INET, SERVER_HOST, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(server_sock_fd);
        return -1;
    }
    
    if (connect(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(server_sock_fd);
        return -1;
    }
    
    printf("[Client] Connected to server at %s:%d\n", SERVER_HOST, SERVER_PORT);
    return 0;
}

// Simple line reader and "back" detector
static int read_line(char *buf, size_t sz) {
    if (!fgets(buf, (int)sz, stdin)) return 0;
    size_t len = strlen(buf);
    while (len && (buf[len-1] == '\n' || buf[len-1] == '\r')) buf[--len] = '\0';
    return 1;
}

static int is_back_token(const char *s) {
    // Accept 'b' or 'B' as "back"
    return s && s[0] && (s[0] == 'b' || s[0] == 'B') && s[1] == '\0';
}

static int prompt_account_id_or_back(const char *prompt, int *out_id) {
    char line[64];
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!read_line(line, sizeof line)) return 0;  // EOF or read error → go back
        if (is_back_token(line)) return 0;            // user chose back
        int id;
        if (sscanf(line, "%d", &id) == 1) {
            if (is_valid_account_id(id)) { *out_id = id; return 1; }
            printf("Invalid Account ID. Try again or enter 'b' to go back.\n");
        } else {
            printf("Invalid input. Enter a number or 'b' to go back.\n");
        }
    }
}

static int prompt_amount_or_back(const char *prompt, double *out_amount) {
    char line[64];
    while (1) {
        printf("%s", prompt);
        fflush(stdout);
        if (!read_line(line, sizeof line)) return 0;
        if (is_back_token(line)) return 0;
        double amt;
        if (sscanf(line, "%lf", &amt) == 1 && amt > 0.0) { *out_amount = amt; return 1; }
        printf("Invalid amount. Enter a positive number or 'b' to go back.\n");
    }
}

// ============================================================================
// BUILT-IN STRESS TEST
// ============================================================================
typedef struct {
    int thread_id;
    int ops_completed;
    int ops_failed;
} StressClientArgs;

static pthread_mutex_t stress_stats_lock = PTHREAD_MUTEX_INITIALIZER;
static int stress_total_success = 0;
static int stress_total_failure = 0;

static double get_time_sec(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

static int stress_send_command(int sock, const char *cmd, char *response, size_t resp_size) {
    if (send(sock, cmd, strlen(cmd), 0) < 0) return -1;
    int n = recv(sock, response, resp_size - 1, 0);
    if (n <= 0) return -1;
    response[n] = '\0';
    return (strncmp(response, "SUCCESS", 7) == 0) ? 0 : -1;
}

static void* stress_client_thread(void *arg) {
    StressClientArgs *args = (StressClientArgs *)arg;
    char cmd[BUFFER_SIZE], response[BUFFER_SIZE];
    
    // Connect to server
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return NULL;
    
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_HOST, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return NULL;
    }
    
    // Create account
    snprintf(cmd, sizeof(cmd), "CREATE\n");
    stress_send_command(sock, cmd, response, sizeof(response));
    int my_account = -1;
    sscanf(response, "SUCCESS CREATE %d", &my_account);
    
    if (my_account < 0) {
        close(sock);
        return NULL;
    }
    
    printf("[Client %2d] Created account #%d\n", args->thread_id, my_account);
    
    // Perform operations
    for (int i = 0; i < STRESS_OPS_PER_CLIENT; i++) {
        int op = rand() % 3;
        int success = 0;
        const char *op_name;
        
        switch (op) {
            case 0:
                snprintf(cmd, sizeof(cmd), "DEPOSIT %d %.2f\n", my_account, (rand() % 1000) + 1.0);
                success = (stress_send_command(sock, cmd, response, sizeof(response)) == 0);
                op_name = "DEPOSIT";
                break;
            case 1:
                snprintf(cmd, sizeof(cmd), "WITHDRAW %d %.2f\n", my_account, (rand() % 10) + 1.0);
                success = (stress_send_command(sock, cmd, response, sizeof(response)) == 0);
                op_name = "WITHDRAW";
                break;
            case 2:
                snprintf(cmd, sizeof(cmd), "BALANCE %d\n", my_account);
                success = (stress_send_command(sock, cmd, response, sizeof(response)) == 0);
                op_name = "BALANCE";
                break;
            default:
                op_name = "UNKNOWN";
        }
        
        char *nl = strchr(response, '\n');
        if (nl) *nl = '\0';
        printf("[Client %2d] Op %2d: %-8s -> %s\n", args->thread_id, i + 1, op_name, response);
        
        if (success) args->ops_completed++;
        else args->ops_failed++;
    }
    
    close(sock);
    
    pthread_mutex_lock(&stress_stats_lock);
    stress_total_success += args->ops_completed;
    stress_total_failure += args->ops_failed;
    pthread_mutex_unlock(&stress_stats_lock);
    
    return NULL;
}

void run_stress_test(const char *mode_name, int num_clients) {
    printf("\n============================================================\n");
    printf("  STRESS TEST - %s MODE\n", mode_name);
    printf("============================================================\n");
    printf("  Clients: %d, Ops per client: %d, Total: %d ops\n", 
           num_clients, STRESS_OPS_PER_CLIENT, 
           num_clients * STRESS_OPS_PER_CLIENT);
    printf("============================================================\n\n");
    
    stress_total_success = 0;
    stress_total_failure = 0;
    
    pthread_t threads[STRESS_NUM_CLIENTS];
    StressClientArgs args[STRESS_NUM_CLIENTS];
    
    double start = get_time_sec();
    
    for (int i = 0; i < num_clients; i++) {
        args[i].thread_id = i;
        args[i].ops_completed = 0;
        args[i].ops_failed = 0;
        pthread_create(&threads[i], NULL, stress_client_thread, &args[i]);
    }
    
    for (int i = 0; i < num_clients; i++) {
        pthread_join(threads[i], NULL);
    }
    
    double elapsed = get_time_sec() - start;
    int total = stress_total_success + stress_total_failure;
    
    printf("\n============================================================\n");
    printf("  RESULTS - %s MODE\n", mode_name);
    printf("============================================================\n");
    printf("  Successful: %d, Failed: %d, Total: %d\n", 
           stress_total_success, stress_total_failure, total);
    printf("  Time: %.2f seconds\n", elapsed);
    printf("  Throughput: %.2f ops/sec\n", total / elapsed);
    printf("============================================================\n");
}

// Main program
int main() {
    if (setup_connection() < 0) {
        printf("Failed to connect to the bank server.\n");
        return 1;
    }
    
    display_welcome();
    
    int choice;
    while (1) {
        display_menu();
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please try again.\n");
            while (getchar() != '\n');
            continue;
        }
        
        while (getchar() != '\n');
        
        switch (choice) {
            case 1:
                handle_create_account();
                break;
            case 2:
                handle_deposit();
                break;
            case 3:
                handle_withdraw();
                break;
            case 4:
                handle_transfer();
                break;
            case 5:
                handle_balance();
                break;
            case 6:
                printf("\nThank you for using the Sharaf Bank system. Goodbye!\n");
                // Request server shutdown before closing
                send_command("SHUTDOWN\n");
                close(server_sock_fd);
                return 0;
            case 7:
                printf("\n[Run in Single Threaded Mode]\n");
                send_command("MODE_SINGLE\n");
                run_stress_test("SINGLE-THREADED", STRESS_NUM_CLIENTS);  // 10 clients
                break;
            case 8:
                printf("\n[Run in Multi-Threaded Mode]\n");
                send_command("MODE_MULTI\n");
                run_stress_test("MULTI-THREADED", STRESS_NUM_CLIENTS);  // 10 clients
                break;
            default:
                printf("\nInvalid choice. Please select from the menu (1-8).\n");
                break;
        }
    }
    
    return 0;
}
