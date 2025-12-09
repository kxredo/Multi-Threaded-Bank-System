#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <stdarg.h>

// Thread-safe logger
static pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;
static FILE *log_file = NULL;

// Initialize logger
void logger_init(const char *filename) {
    pthread_mutex_lock(&log_lock);
    
    if (log_file) {
        fclose(log_file);
    }
    
    if (filename) {
        log_file = fopen(filename, "a");
        if (!log_file) {
            fprintf(stderr, "Failed to open log file: %s\n", filename);
        }
    }
    
    pthread_mutex_unlock(&log_lock);
}

// Log a message with timestamp
void logger_log(const char *level, const char *format, ...) {
    pthread_mutex_lock(&log_lock);
    
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    va_list args;
    va_start(args, format);
    
    // Print to stderr
    fprintf(stderr, "[%s] %s: ", timestamp, level);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    // Print to file if open
    if (log_file) {
        fprintf(log_file, "[%s] %s: ", timestamp, level);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
    
    va_end(args);
    pthread_mutex_unlock(&log_lock);
}

// Convenience logging functions
void logger_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&log_lock);
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(stderr, "[%s] INFO: ", timestamp);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    if (log_file) {
        fprintf(log_file, "[%s] INFO: ", timestamp);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
    
    pthread_mutex_unlock(&log_lock);
    va_end(args);
}

void logger_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&log_lock);
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(stderr, "[%s] ERROR: ", timestamp);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    if (log_file) {
        fprintf(log_file, "[%s] ERROR: ", timestamp);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
    
    pthread_mutex_unlock(&log_lock);
    va_end(args);
}

void logger_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    
    pthread_mutex_lock(&log_lock);
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(stderr, "[%s] DEBUG: ", timestamp);
    vfprintf(stderr, format, args);
    fprintf(stderr, "\n");
    
    if (log_file) {
        fprintf(log_file, "[%s] DEBUG: ", timestamp);
        vfprintf(log_file, format, args);
        fprintf(log_file, "\n");
        fflush(log_file);
    }
    
    pthread_mutex_unlock(&log_lock);
    va_end(args);
}

// Cleanup logger
void logger_cleanup() {
    pthread_mutex_lock(&log_lock);
    
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
    
    pthread_mutex_unlock(&log_lock);
    pthread_mutex_destroy(&log_lock);
}
