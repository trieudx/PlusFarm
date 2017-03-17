#ifndef __ONEWIRE_H__
#define __ONEWIRE_H__

/* Inclusion section ======================================================== */
#include "gpio.h"

/* Public macro definition section ========================================== */

/* Public type definition section =========================================== */
typedef enum
{
	ONEWIRE_OK			= 0x0,
	ONEWIRE_ERROR		= 0x1
} OneWire_Return;

typedef struct
{
	GPIO_Pin			hw_pin;
} OneWire_Config;

/* Public function prototype section ======================================== */
void OneWire_Init(OneWire_Config *config);

#endif
