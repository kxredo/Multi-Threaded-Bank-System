#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/bank.h"

// External declarations
extern Account *bank[MAX_ACCOUNTS];

// Protocol command types
typedef enum {
    CMD_CREATE,
    CMD_DEPOSIT,
    CMD_WITHDRAW,
    CMD_TRANSFER,
    CMD_BALANCE,
    CMD_INVALID,
    CMD_BALANCE_ALL
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
    
    return cmd;
}

// Helper function to get account pointer (declared before use)
Account* get_account_ptr(int id) {
    if (id < 0 || id >= MAX_ACCOUNTS) return NULL;
    return bank[id];
}

// Execute command and return response string
void execute_command(const char *input, char *response, size_t resp_size) {
    ParsedCommand cmd = parse_command(input);
    
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
        
        default:
            snprintf(response, resp_size, "FAILURE INVALID -1\n");
            break;
    }
}
