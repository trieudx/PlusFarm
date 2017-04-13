/* Inclusion section ======================================================== */
#include "sdk/esp_common.h"
#include "onewire.h"
#include "hal_gpio.h"
#include "freertos.h"

/* Private macro definition section ========================================= */

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */

/* Private variable section ================================================= */
static HAL_GPIO_PinType onewire_hw_pin;

/* Public function definition section ======================================= */
void OneWire_Init(OneWire_ConfigType *config)
{
  HAL_GPIO_ConfigType gpio;

  /* Initialize HW pin */
  gpio.pin = config->hw_pin;
  gpio.mode = HAL_GPIO_MODE_OUT_OD;
  gpio.pull = HAL_GPIO_PULL_UP;
  gpio.sleepable = false;
  HAL_GPIO_Init(&gpio);

  /* Store HW configuration */
  onewire_hw_pin = config->hw_pin;
}

/* Private function definition section ====================================== */
OneWire_Return onewire_reset(void)
{
  bool devices_present;

  HAL_GPIO_SetLow(onewire_hw_pin);
  sdk_os_delay_us(480);
  HAL_GPIO_SetHigh(onewire_hw_pin);
  sdk_os_delay_us(70);
  devices_present = !HAL_GPIO_Read(onewire_hw_pin);
  sdk_os_delay_us(410);

  return devices_present ? ONEWIRE_OK : ONEWIRE_ERROR;
}

void onewire_set(bool level)
{
  HAL_GPIO_SetLow(onewire_hw_pin);
  if (level == 1)
    sdk_os_delay_us(6);
  else
    sdk_os_delay_us(60);
  HAL_GPIO_SetHigh(onewire_hw_pin);
  if (level == 1)
    sdk_os_delay_us(64);
  else
    sdk_os_delay_us(10);
}

bool onewire_read(void)
{
  bool level;

  HAL_GPIO_SetLow(onewire_hw_pin);
  sdk_os_delay_us(6);
  HAL_GPIO_SetHigh(onewire_hw_pin);
  sdk_os_delay_us(9);
  level = HAL_GPIO_Read(onewire_hw_pin);
  sdk_os_delay_us(55);

  return level;
}

/* ============================= End of file ================================ */
