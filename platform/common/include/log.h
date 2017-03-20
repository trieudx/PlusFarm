#ifndef __LOG_H__
#define __LOG_H__

/* Inclusion section ======================================================== */

/* Public macro definition section ========================================== */
#ifdef LOG_VERBOSE
#define LOG_PRINTF(...) Log_Printf(__VA_ARGS__)
#else
#define LOG_PRINTF(...)
#endif

/* Public type definition section =========================================== */

/* Public function prototype section ======================================== */
void Log_Init(void);
#ifdef LOG_VERBOSE
void Log_Printf(const char *format, ...);
#endif

#endif
