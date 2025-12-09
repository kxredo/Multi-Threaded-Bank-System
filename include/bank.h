// bank.h (Revised)

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h> // for malloc/free

#define MAX_ACCOUNTS 1000

// Account Structure remains the same
typedef struct {
    int id;
    double balance;
    pthread_mutex_t lock; 
    char padding[64 - (sizeof(int) + sizeof(double) + sizeof(pthread_mutex_t))];
} Account;

// The Bank State
extern Account *bank[MAX_ACCOUNTS]; // Array of Pointers
extern pthread_mutex_t bank_state_lock; // Lock for the array/counter
extern int next_account_id; // Global counter for new unique IDs

// New Prototypes
void init_bank();
int create_account(); // Returns new account ID or -1
int deposit(int id, double amount);
int withdraw(int id, double amount);
int transfer(int from_id, int to_id, double amount);
Account* get_account(int id);