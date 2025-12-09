#ifndef LOGGER_H
#define LOGGER_H

void logger_init(const char *filename);
void logger_log(const char *level, const char *format, ...);
void logger_info(const char *format, ...);
void logger_error(const char *format, ...);
void logger_debug(const char *format, ...);
void logger_cleanup();

#endif // LOGGER_H
