#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>  // for usleep()

#include "../include/bank.h"

// ============================================================================
// SIMULATED PROCESSING DELAY - Makes threading difference visible
// ============================================================================
// Set to 0 to disable delay, or any value in milliseconds (e.g., 100)
// This simulates real-world latency: database access, validation, etc.
#define SIMULATED_DELAY_MS 100
// ============================================================================

// External declarations
extern Account *bank[MAX_ACCOUNTS];

// Protocol command types
typedef enum {
    CMD_CREATE,
    CMD_DEPOSIT,
    CMD_WITHDRAW,
    CMD_TRANSFER,
    CMD_BALANCE,
    CMD_SHUTDOWN,
    CMD_INVALID,
    CMD_BALANCE_ALL,
    CMD_MODE_SINGLE,
    CMD_MODE_MULTI,
    CMD_MODE_STATUS
} CommandType;

// Parsed command structure
typedef struct {
    CommandType type;
    int account_id;
    int target_id;
    double amount;
} ParsedCommand;

// Forward declaration
Account* get_account_ptr(int id);

// Trim whitespace from string
static void trim(char *str) {
    char *end = str + strlen(str) - 1;
    while (end >= str && isspace(*end)) {
        *end = '\0';
        end--;
    }
    while (*str && isspace(*str)) {
        str++;
    }
}

// Parse incoming command string
ParsedCommand parse_command(const char *input) {
    ParsedCommand cmd = {0};
    cmd.type = CMD_INVALID;
    
    if (!input || strlen(input) == 0) {
        return cmd;
    }
    
    char buffer[256];
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    // Trim newline/spaces
    trim(buffer);
    
    char cmd_name[32] = {0};
    sscanf(buffer, "%31s", cmd_name);
    
    // Convert to uppercase for comparison
    for (int i = 0; cmd_name[i]; i++) {
        cmd_name[i] = toupper(cmd_name[i]);
    }
    
    // Parse based on command type
    if (strcmp(cmd_name, "CREATE") == 0) {
        cmd.type = CMD_CREATE;
    }
    else if (strcmp(cmd_name, "DEPOSIT") == 0) {
        if (sscanf(buffer, "%*s %d %lf", &cmd.account_id, &cmd.amount) == 2) {
            cmd.type = CMD_DEPOSIT;
        }
    }
    else if (strcmp(cmd_name, "WITHDRAW") == 0) {
        if (sscanf(buffer, "%*s %d %lf", &cmd.account_id, &cmd.amount) == 2) {
            cmd.type = CMD_WITHDRAW;
        }
    }
    else if (strcmp(cmd_name, "TRANSFER") == 0) {
        if (sscanf(buffer, "%*s %d %d %lf", &cmd.account_id, &cmd.target_id, &cmd.amount) == 3) {
            cmd.type = CMD_TRANSFER;
        }
    }
    else if (strcmp(cmd_name, "BALANCE") == 0) {
        if (sscanf(buffer, "%*s %d", &cmd.account_id) == 1) {
            cmd.type = CMD_BALANCE;
        }
    }
    else if (strcmp(cmd_name, "BALANCE_ALL") == 0) {
        cmd.type = CMD_BALANCE_ALL;
    }
    else if (strcmp(cmd_name, "SHUTDOWN") == 0) {
        cmd.type = CMD_SHUTDOWN;
    }
    else if (strcmp(cmd_name, "MODE_SINGLE") == 0) {
        cmd.type = CMD_MODE_SINGLE;
    }
    else if (strcmp(cmd_name, "MODE_MULTI") == 0) {
        cmd.type = CMD_MODE_MULTI;
    }
    else if (strcmp(cmd_name, "MODE_STATUS") == 0) {
        cmd.type = CMD_MODE_STATUS;
    }
    
    return cmd;
}

// Helper function to get account pointer (declared before use)
Account* get_account_ptr(int id) {
    if (id < 0 || id >= MAX_ACCOUNTS) return NULL;
    return bank[id];
}

// Execute command and return response string
// Forward-declared server shutdown hook
extern void server_request_shutdown(void);

// External functions to control threading mode
extern void server_set_single_threaded(int enabled);
extern int server_get_single_threaded(void);

// Simulate processing delay (database access, validation, etc.)
static void simulate_processing_delay(void) {
#if SIMULATED_DELAY_MS > 0
    usleep(SIMULATED_DELAY_MS * 1000);  // Convert ms to microseconds
#endif
}

void execute_command(const char *input, char *response, size_t resp_size) {
    ParsedCommand cmd = parse_command(input);
    
    // Simulate real-world processing time for most commands
    if (cmd.type != CMD_INVALID && cmd.type != CMD_SHUTDOWN) {
        simulate_processing_delay();
    }
    
    switch (cmd.type) {
        case CMD_CREATE: {
            int new_id = create_account();
            if (new_id >= 0) {
                snprintf(response, resp_size, "SUCCESS CREATE %d\n", new_id);
            } else {
                snprintf(response, resp_size, "FAILURE CREATE -1\n");
            }
            break;
        }
        
        case CMD_DEPOSIT: {
            int result = deposit(cmd.account_id, cmd.amount);
            if (result > 0) {
                Account *acc = get_account_ptr(cmd.account_id);
                snprintf(response, resp_size, "SUCCESS DEPOSIT %.2f\n", acc->balance);
            } else {
                snprintf(response, resp_size, "FAILURE DEPOSIT -1\n");
            }
            break;
        }
        
        case CMD_WITHDRAW: {
            int result = withdraw(cmd.account_id, cmd.amount);
            if (result > 0) {
                Account *acc = get_account_ptr(cmd.account_id);
                snprintf(response, resp_size, "SUCCESS WITHDRAW %.2f\n", acc->balance);
            } else {
                snprintf(response, resp_size, "FAILURE WITHDRAW -1\n");
            }
            break;
        }
        
        case CMD_TRANSFER: {
            int result = transfer(cmd.account_id, cmd.target_id, cmd.amount);
            if (result > 0) {
                Account *from = get_account_ptr(cmd.account_id);
                snprintf(response, resp_size, "SUCCESS TRANSFER %.2f\n", from->balance);
            } else {
                snprintf(response, resp_size, "FAILURE TRANSFER -1\n");
            }
            break;
        }
        
        case CMD_BALANCE: {
            Account *acc = get_account_ptr(cmd.account_id);
            if (acc) {
                snprintf(response, resp_size, "SUCCESS BALANCE %.2f\n", acc->balance);
            } else {
                snprintf(response, resp_size, "FAILURE BALANCE -1\n");
            }
            break;
        }
        
        case CMD_BALANCE_ALL: {
            extern Account *bank[MAX_ACCOUNTS];
            snprintf(response, resp_size, "--- All Account Balances ---\n");
            int found = 0;
            for (int i = 0; i < MAX_ACCOUNTS; i++) {
                if (bank[i] != NULL) {
                    found = 1;
                    char line[128];
                    snprintf(line, sizeof(line), "Account ID %d: $%.2f\n", i, bank[i]->balance);
                    strncat(response, line, resp_size - strlen(response) - 1);
                }
            }
            if (!found) {
                snprintf(response, resp_size, "No accounts found.\n");
            }
            break;
        }
        
        case CMD_SHUTDOWN: {
            server_request_shutdown();
            snprintf(response, resp_size, "SUCCESS SHUTDOWN\n");
            break;
        }
        
        case CMD_MODE_SINGLE: {
            server_set_single_threaded(1);
            snprintf(response, resp_size, "SUCCESS MODE_SINGLE\n");
            printf("[Server] Switched to SINGLE-THREADED mode\n");
            break;
        }
        
        case CMD_MODE_MULTI: {
            server_set_single_threaded(0);
            snprintf(response, resp_size, "SUCCESS MODE_MULTI\n");
            printf("[Server] Switched to MULTI-THREADED mode\n");
            break;
        }
        
        case CMD_MODE_STATUS: {
            int is_single = server_get_single_threaded();
            snprintf(response, resp_size, "SUCCESS MODE_STATUS %s\n", 
                     is_single ? "SINGLE" : "MULTI");
            break;
        }
        
        default:
            snprintf(response, resp_size, "FAILURE INVALID -1\n");
            break;
    }
}
