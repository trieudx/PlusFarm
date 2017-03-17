/* Inclusion section ======================================================== */
#include "esp_common.h"
#include "onewire.h"
#include "gpio.h"
#include "FreeRTOS.h"

/* Private macro definition section ========================================= */

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */

/* Private variable section ================================================= */
static GPIO_Pin			onewire_hw_pin;

/* Public function definition section ======================================= */
void OneWire_Init(OneWire_Config *config)
{
	GPIO_Config gpio;

	/* Initialize HW pin */
	gpio.pin = config->hw_pin;
	gpio.mode = GPIO_MODE_OUT_OD;
	gpio.pull = GPIO_PULL_UP;
	GPIO_Init(&gpio);

	/* Store HW configuration */
	onewire_hw_pin = config->hw_pin;
}


/* Private function definition section ====================================== */
static OneWire_Return onewire_reset(void)
{
	bool devices_present;

	GPIO_SetLow(onewire_hw_pin);
	os_delay_us(480);
	GPIO_SetHigh(onewire_hw_pin);
	os_delay_us(70);
	devices_present = !GPIO_Read(onewire_hw_pin);
	os_delay_us(410);

	return devices_present ? ONEWIRE_OK : ONEWIRE_ERROR;
}

static void onewire_set(bool level)
{
	GPIO_SetLow(onewire_hw_pin);
	if (level == 1)
		os_delay_us(6);
	else
		os_delay_us(60);
	GPIO_SetHigh(onewire_hw_pin);
	if (level == 1)
		os_delay_us(64);
	else
		os_delay_us(10);
}

static bool onewire_read(void)
{
	bool level;

	GPIO_SetLow(onewire_hw_pin);
	os_delay_us(6);
	GPIO_SetHigh(onewire_hw_pin);
	os_delay_us(9);
	level = GPIO_Read(onewire_hw_pin);
	os_delay_us(55);

	return level;
}

/* ============================= End of file ================================ */
