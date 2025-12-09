// client.c (Main TUI functions)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

static int server_sock_fd = -1;

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
    printf("Enter your choice: ");
}

// Send command to server and receive response
int send_command(const char *command) {
    if (send(server_sock_fd, command, strlen(command), 0) < 0) {
        perror("send");
        return -1;
    }
    
    char response[BUFFER_SIZE];
    int n = recv(server_sock_fd, response, sizeof(response) - 1, 0);
    if (n < 0) {
        perror("recv");
        return -1;
    }
    
    response[n] = '\0';
    
    // Parse and display error/success messages
    if (strncmp(response, "SUCCESS", 7) == 0) {
        printf("✓ Operation successful.\n");
    } else if (strncmp(response, "FAILURE", 7) == 0) {
        // Extract the error reason
        char reason[128];
        sscanf(response, "FAILURE %*s %127s", reason);
        
        if (strstr(response, "INSUFFICIENT_FUNDS")) {
            printf("✗ Error: Insufficient funds in account.\n");
        } else if (strstr(response, "ACCOUNT_NOT_FOUND")) {
            printf("✗ Error: Account not found.\n");
        } else if (strstr(response, "INVALID_AMOUNT")) {
            printf("✗ Error: Invalid amount.\n");
        } else if (strstr(response, "SAME_ACCOUNT")) {
            printf("✗ Error: Cannot transfer to the same account.\n");
        } else {
            printf("✗ Operation failed.\n");
        }
    }
    
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
    
    // If response starts with SUCCESS, account is valid
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
        
        while (getchar() != '\n'); // Clear buffer
        return id;
    }
}

void handle_deposit() {
    int id;
    double amount;
    char command[BUFFER_SIZE];
    
    id = get_valid_account_id("Enter Account ID to deposit to: ");
    
    printf("Enter Amount to deposit: $");
    while (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("Invalid amount. Please enter a positive number: $");
        while (getchar() != '\n');
    }
    
    while (getchar() != '\n'); // Clear buffer
    
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
    
    while (getchar() != '\n'); // Clear buffer
    
    snprintf(command, sizeof(command), "WITHDRAW %d %.2lf\n", id, amount);
    printf("[Sending command to Server: %s]", command);
    send_command(command);
    display_all_accounts();
}

void handle_transfer() {
    int from_id, to_id;
    double amount;
    char command[BUFFER_SIZE];
    
    from_id = get_valid_account_id("Enter Source Account ID: ");
    
    printf("Enter Destination Account ID: ");
    while (1) {
        if (scanf("%d", &to_id) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }
        
        if (!is_valid_account_id(to_id)) {
            printf("Invalid Account ID. Please try again.\n");
            continue;
        }
        
        if (from_id == to_id) {
            printf("Cannot transfer to the same account. Please try again.\n");
            continue;
        }
        
        break;
    }
    
    while (getchar() != '\n'); // Clear buffer
    
    printf("Enter Amount to transfer: $");
    while (scanf("%lf", &amount) != 1 || amount <= 0) {
        printf("Invalid amount. Please enter a positive number: $");
        while (getchar() != '\n');
    }
    
    while (getchar() != '\n'); // Clear buffer
    
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

int main() {
    int choice;
    
    printf("Attempting to connect to server...\n");
    if (setup_connection() < 0) {
        printf("Failed to connect to server. Exiting.\n");
        return 1;
    }
    
    display_welcome();
    
    while (1) {
        display_menu();
        
        // Input validation
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n');
            continue;
        }
        
        while (getchar() != '\n'); // Clear input buffer
        
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
                close(server_sock_fd);
                return 0;
            default:
                printf("\nInvalid choice. Please select from the menu (1-6).\n");
                break;
        }
    }
    
    return 0;
}