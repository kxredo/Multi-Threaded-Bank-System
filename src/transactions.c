#include "../include/bank.h"
#include <stdio.h>
#include <stdlib.h>

// Global state
Account *bank[MAX_ACCOUNTS];
pthread_mutex_t bank_state_lock;
int next_account_id = 0;

void init_bank() {
    pthread_mutex_init(&bank_state_lock, NULL);
    for (int i = 0; i < MAX_ACCOUNTS; i++) {
        bank[i] = NULL;
    }
    next_account_id = 0;
}

// Helper function to get account safely
Account* get_account(int id) {
    if (id < 0 || id >= MAX_ACCOUNTS) return NULL;
    return bank[id]; 
}

// Create a new account with a unique ID
int create_account() {
    pthread_mutex_lock(&bank_state_lock);
    
    if (next_account_id >= MAX_ACCOUNTS) {
        pthread_mutex_unlock(&bank_state_lock);
        return -1; // Out of space
    }
    
    int new_id = next_account_id++;
    
    // Allocate and initialize new account
    Account *acc = (Account *)malloc(sizeof(Account));
    if (!acc) {
        pthread_mutex_unlock(&bank_state_lock);
        return -1;
    }
    
    acc->id = new_id;
    acc->balance = 0.0;
    pthread_mutex_init(&acc->lock, NULL);
    
    bank[new_id] = acc;
    pthread_mutex_unlock(&bank_state_lock);
    
    return new_id;
}

// Deposit funds into an account
int deposit(int id, double amount) {
    Account *acc = get_account(id);
    if (!acc || amount <= 0) return -1;

    pthread_mutex_lock(&acc->lock);
    acc->balance += amount;
    pthread_mutex_unlock(&acc->lock);
    
    return 1;
}

// Withdraw funds from an account
int withdraw(int id, double amount) {
    Account *acc = get_account(id);
    if (!acc || amount <= 0) return -1;

    pthread_mutex_lock(&acc->lock);
    
    int success = 0;
    if (acc->balance >= amount) {
        acc->balance -= amount;
        success = 1;
    }

    pthread_mutex_unlock(&acc->lock);
    return success;
}

// Transfer from one account to another with deadlock avoidance
int transfer(int from_id, int to_id, double amount) {
    if (from_id == to_id || amount <= 0) return -1;
    
    Account *from = get_account(from_id);
    Account *to = get_account(to_id);
    
    if (!from || !to) return -1;
    
    // Lock in strict ordering (by ID) to prevent deadlock
    int first_id = (from_id < to_id) ? from_id : to_id;
    int second_id = (from_id < to_id) ? to_id : from_id;
    
    Account *first = (first_id == from_id) ? from : to;
    Account *second = (second_id == from_id) ? from : to;
    
    pthread_mutex_lock(&first->lock);
    pthread_mutex_lock(&second->lock);
    
    int success = 0;
    if (from->balance >= amount) {
        from->balance -= amount;
        to->balance += amount;
        success = 1;
    }
    
    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
    
    return success;
}

// Get account balance
double get_balance(int id) {
    Account *acc = get_account(id);
    if (!acc) return -1.0;
    
    pthread_mutex_lock(&acc->lock);
    double balance = acc->balance;
    pthread_mutex_unlock(&acc->lock);
    
    return balance;
}