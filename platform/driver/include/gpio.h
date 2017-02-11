#ifndef __GPIO_H__
#define __GPIO_H__

/* Inclusion section ======================================================== */

/* Public macro definition section ========================================== */

/* Public type definition section =========================================== */
typedef enum
{
	GPIO_PIN_0		= 0x00,
	GPIO_PIN_2		= 0x02,
	GPIO_PIN_4		= 0x04,		/* Blue LED on ESP-12F module */
	GPIO_PIN_5		= 0x05,
	GPIO_PIN_12		= 0x0C,		/* Green LED on Extended ESP-12F module */
	GPIO_PIN_13		= 0x0D,		/* Blue LED on Extended ESP-12F module */
	GPIO_PIN_14		= 0x0E,
	GPIO_PIN_15		= 0x0F,		/* Red LED on Extended ESP-12F module */
	GPIO_PIN_16		= 0x1F,
} GPIO_Pin;

typedef enum
{
	GPIO_MODE_IN		= 0x0,
	GPIO_MODE_OUT_OD	= 0x1,
	GPIO_MODE_OUT_PP	= 0x2
} GPIO_Mode;

typedef enum
{
	GPIO_NO_PULL	= 0x0,
	GPIO_PULL_UP	= 0x1,
} GPIO_Pull;

typedef struct
{
	GPIO_Pin		pin;
	GPIO_Mode		mode;
	GPIO_Pull		pull;
} GPIO_Config;

/* Public function prototype section ======================================== */
void GPIO_Init(GPIO_Config *config);
void GPIO_SetLow(GPIO_Pin pin);
void GPIO_SetHigh(GPIO_Pin pin);
void GPIO_Toggle(GPIO_Pin pin);
void GPIO_Set(GPIO_Pin pin, bool level);
bool GPIO_Read(GPIO_Pin pin);

#endif
