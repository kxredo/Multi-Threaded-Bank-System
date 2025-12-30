CC = gcc
CFLAGS = -Wall -Wextra -pthread -I./include -g
LDFLAGS = -pthread

# Source files
SERVER_SOURCES = src/server.c src/transactions.c src/thread_pool.c src/protocol.c
CLIENT_SOURCES = src/client.c
STRESS_SOURCES = src/stress_client.c

# Object files
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)
STRESS_OBJECTS = $(STRESS_SOURCES:.c=.o)

# Executables
SERVER = server
CLIENT = client
STRESS_CLIENT = stress_client

# Race condition demo
RACE_DEMO = race_demo

# Default target
all: $(SERVER) $(CLIENT) $(STRESS_CLIENT)

# Build server
$(SERVER): $(SERVER_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Build client
$(CLIENT): $(CLIENT_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Build stress test client
$(STRESS_CLIENT): $(STRESS_OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(SERVER_OBJECTS) $(CLIENT_OBJECTS) $(STRESS_OBJECTS) $(SERVER) $(CLIENT) $(STRESS_CLIENT)

# Clean everything including logs
distclean: clean
	rm -f *.log bank.dat

# Run server
run_server: $(SERVER)
	./$(SERVER)

# Run client (requires server to be running)
run_client: $(CLIENT)
	./$(CLIENT)

# Run stress test (requires server to be running)
run_stress: $(STRESS_CLIENT)
	./$(STRESS_CLIENT)


# Rebuild everything
rebuild: clean all

.PHONY: all clean distclean run_server run_client run_stress run_race rebuild
