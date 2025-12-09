#ifndef THREAD_POOL_H
#define THREAD_POOL_H

void thread_pool_init(int num_workers);
int submit_task(int client_fd, const char *command);
void thread_pool_shutdown();

#endif // THREAD_POOL_H
