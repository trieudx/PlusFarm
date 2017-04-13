#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

/* Inclusion section ======================================================== */
#include "stdbool.h"

/* Public macro definition section ========================================== */

/* Public type definition section =========================================== */
typedef enum
{
  HAL_GPIO_PIN_0          = 0x00,
  HAL_GPIO_PIN_2          = 0x02,
  HAL_GPIO_PIN_4          = 0x04,     /* Blue LED on ESP-12F module */
  HAL_GPIO_PIN_5          = 0x05,
  HAL_GPIO_PIN_12         = 0x0C,     /* Green LED on Extended ESP-12F module */
  HAL_GPIO_PIN_13         = 0x0D,     /* Blue LED on Extended ESP-12F module */
  HAL_GPIO_PIN_14         = 0x0E,
  HAL_GPIO_PIN_15         = 0x0F,     /* Red LED on Extended ESP-12F module */
  HAL_GPIO_PIN_16         = 0x1F
} HAL_GPIO_PinType;

typedef enum
{
  HAL_GPIO_MODE_IN        = 0x00,
  HAL_GPIO_MODE_OUT_OD    = 0x01,
  HAL_GPIO_MODE_OUT_PP    = 0x02
} HAL_GPIO_ModeType;

typedef enum
{
  HAL_GPIO_NO_PULL        = 0x00,
  HAL_GPIO_PULL_UP        = 0x01
} HAL_GPIO_PullType;

typedef enum
{
  HAL_GPIO_INTR_FALLING_EDGE      = 0x01,
  HAL_GPIO_INTR_RISING_EDGE       = 0x02,
  HAL_GPIO_INTR_BOTH_EDGES        = 0x03
} HAL_GPIO_IntrTypeType;

typedef struct
{
  HAL_GPIO_PinType          pin;
  HAL_GPIO_ModeType         mode;
  HAL_GPIO_PullType         pull;
  bool                      sleepable;
} HAL_GPIO_ConfigType;

typedef struct
{
  HAL_GPIO_PinType          pin;
  HAL_GPIO_IntrTypeType     intr_type;
  void                      (*irq_handler)(HAL_GPIO_PinType pin);
} HAL_GPIO_IntrType;

/* Public function prototype section ======================================== */
void HAL_GPIO_Init(HAL_GPIO_ConfigType *config);
void HAL_GPIO_DeInit(HAL_GPIO_PinType pin);
void HAL_GPIO_SetLow(HAL_GPIO_PinType pin);
void HAL_GPIO_SetHigh(HAL_GPIO_PinType pin);
void HAL_GPIO_Toggle(HAL_GPIO_PinType pin);
void HAL_GPIO_Set(HAL_GPIO_PinType pin, bool level);
bool HAL_GPIO_Read(HAL_GPIO_PinType pin);
void HAL_GPIO_EnableIrq(HAL_GPIO_IntrType *intr);
void HAL_GPIO_DisableIrq(HAL_GPIO_PinType pin);

#endif

/* ============================= End of file ================================ */
