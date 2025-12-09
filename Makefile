CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./include -g
LDFLAGS = -pthread

# Source files
SERVER_SOURCES = src/server.c src/transactions.c src/thread_pool.c src/protocol.c src/logger.c
CLIENT_SOURCES = src/client.c

# Object files
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)

# Executables
SERVER = server
CLIENT = client

# Single-threaded server (for comparison)
SERVER_ST = server_st
SERVER_ST_SOURCES = src/server_single_threaded.c src/transactions.c src/protocol.c src/logger.c
SERVER_ST_OBJECTS = $(SERVER_ST_SOURCES:.c=.o)

# Default target
all: $(SERVER) $(CLIENT) $(SERVER_ST)

# Build server
$(SERVER): $(SERVER_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Build client
$(CLIENT): $(CLIENT_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Build single-threaded server
$(SERVER_ST): $(SERVER_ST_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(SERVER_ST_OBJECTS) $(SERVER) $(CLIENT) $(SERVER_ST)

# Clean everything including logs
distclean: clean
	rm -f *.log bank.dat

# Run server
run_server: $(SERVER)
	./$(SERVER)

# Run client (requires server to be running)
run_client: $(CLIENT)
	./$(CLIENT)

# Run single-threaded server
run_server_st: $(SERVER_ST)
	./$(SERVER_ST)

# Rebuild everything
rebuild: clean all

.PHONY: all clean distclean run_server run_client run_server_st rebuild
