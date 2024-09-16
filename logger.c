#ifndef LOGGER_C
#define LOGGER_C

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

// TODO(tommaso): cannot use logger with windows gui application
//                modify the logging system to write to a file

#define ANSI_COLOR_WHITE  "\x1b[0m"
#define ANSI_COLOR_RED    "\x1b[31m"
#define ANSI_COLOR_GREEN  "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"

#define LOGGER_TABLE \
    ENTRY(LOG_DEBUG,   "DEBUG",   ANSI_COLOR_WHITE)  \
    ENTRY(LOG_SUCCESS, "SUCCESS", ANSI_COLOR_GREEN)  \
    ENTRY(LOG_WARNING, "WARNING", ANSI_COLOR_YELLOW) \
    ENTRY(LOG_ERROR,   "ERROR",   ANSI_COLOR_RED)

typedef enum
{
#define ENTRY(level, type, color) level,
    LOGGER_TABLE
#undef ENTRY
    LOG_LEVEL_LEN
} LOG_LEVEL;

char *log_types[LOG_LEVEL_LEN] = {
#define ENTRY(level, type, color) type,
    LOGGER_TABLE
#undef ENTRY
};

char *colors[LOG_LEVEL_LEN] = {
#define ENTRY(level, type, color) color,
    LOGGER_TABLE
#undef ENTRY
};

time_t current_time;
struct tm *m_time; 

void GetTime() {
    time(&current_time);
    m_time = localtime(&current_time);
}

void Log(LOG_LEVEL level, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
        GetTime();
        #ifndef NO_ANSI 
            printf("%s", colors[level]);
        #endif
        
        printf(
            "[%d/%d/%d -> %d:%d:%d][%s] ",
            m_time->tm_mday,
            m_time->tm_mon,
            m_time->tm_year + 1900,
            m_time->tm_hour, 
            m_time->tm_min, 
            m_time->tm_sec,
            log_types[level]
        ); 
        vfprintf(stdout, fmt, args);
        printf("\n%s", colors[LOG_DEBUG]);
    va_end(args);
}

#endif // LOGGER_C