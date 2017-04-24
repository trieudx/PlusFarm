#ifndef __LOG_H__
#define __LOG_H__

/* Inclusion section ======================================================== */
#ifndef LOG_VERBOSE
#define LOG_VERBOSE             1
#endif

/* Public macro definition section ========================================== */
#if LOG_VERBOSE
#define LOG_PRINTF(...)         Log_Printf(__VA_ARGS__)
#else
#define LOG_PRINTF(...)
#endif

/* Public type definition section =========================================== */

/* Public function prototype section ======================================== */
void Log_Init(void);
#if LOG_VERBOSE
void Log_Printf(const char *format, ...);
#endif

#endif
