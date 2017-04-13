#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

/* Inclusion section ======================================================== */
#include "hal_gpio.h"

/* Public macro definition section ========================================== */

/* Public type definition section =========================================== */
typedef enum
{
  ONEWIRE_OK        = 0x0,
  ONEWIRE_ERROR     = 0x1
} OneWire_Return;

typedef struct
{
  HAL_GPIO_PinType    hw_pin;
} OneWire_ConfigType;

/* Public function prototype section ======================================== */
void OneWire_Init(OneWire_ConfigType *config);

#endif
