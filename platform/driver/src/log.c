/* Inclusion section ======================================================== */
#include "sdk/esp_common.h"
#include "stdarg.h"
#include "log.h"
#include "hal_uart.h"
#include "freertos.h"
#include "freertos_queue.h"
#include "freertos_task.h"
#include "stdio.h"

/* Private macro definition section ========================================= */
#define LOG_QUEUE_SIZE              50

/* Private type definition section ========================================== */
typedef struct
{
  uint16_t  millisecond;
  uint8_t   second;
  uint8_t   minute;
  uint8_t   hour;
} log_time_t;

typedef struct
{
  char      *content;
  uint32_t  cur_time_ms;
} log_msg_t;

/* Private function prototype section ======================================= */
#ifdef LOG_VERBOSE
static void task_log(void *param);
static void log_get_time(log_time_t *time, uint32_t ms);
#endif

/* Private variable section ================================================= */
#ifdef LOG_VERBOSE
static QueueHandle_t         log_queue;
#endif

/* Public function definition section ======================================= */
void Log_Init(void)
{
  /* Initialize UART for printing log */
  uart_set_baud(0, 460800);

#ifdef LOG_VERBOSE
  /* Initialize log queue */
  log_queue = xQueueCreate(LOG_QUEUE_SIZE, sizeof(log_msg_t));
  /* Create log task */
  xTaskCreate(task_log, "task_log", 256, NULL, 5, NULL);
#endif
}

#ifdef LOG_VERBOSE
void Log_Printf(const char *format, ...)
{
  log_msg_t msg;
  va_list argptr;
  uint16_t msg_len;

  /* Get current time point */
  msg.cur_time_ms = sdk_system_get_time() / 1000;

  va_start(argptr, format);
  msg_len = vsnprintf(NULL, 0, format, argptr) + 1;
  va_end(argptr);
  msg.content = pvPortMalloc(msg_len);
  va_start(argptr, format);
  vsnprintf(msg.content, msg_len, format, argptr);
  va_end(argptr);

  xQueueSend(log_queue, &msg, portMAX_DELAY);
}
#endif

/* Private function definition section ====================================== */
#ifdef LOG_VERBOSE
static void task_log(void *param)
{
  log_time_t time_info;
  log_msg_t msg;

  while(1)
  {
    xQueueReceive(log_queue, &msg, portMAX_DELAY);

    /* Get time information */
    log_get_time(&time_info, msg.cur_time_ms);
    /* Print log */
    printf("    %02d:%02d:%02d.%03d    %s\n",
        time_info.hour, time_info.minute,
        time_info.second, time_info.millisecond,
        msg.content);
    vPortFree(msg.content);
  }
}

static void log_get_time(log_time_t *time, uint32_t ms)
{
  uint32_t second = ms / 1000;
  uint32_t minute = second / 60;
  uint32_t hour = minute / 60;

  time->millisecond = ms % 1000;
  time->second = second % 60;
  time->minute = minute % 60;
  time->hour = hour % 24;
}
#endif

/* ============================= End of file ================================ */
