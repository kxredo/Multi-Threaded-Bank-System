#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "bank.h"

void execute_command(int client_fd, const char *input, char *response, size_t resp_size);
Account* get_account_ptr(int id);

#endif // PROTOCOL_H
