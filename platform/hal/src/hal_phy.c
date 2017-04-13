/** esp/phy.h
 *
 * PHY hardware management functions.
 *
 * Part of esp-open-rtos
 * Copyright (C) 2016 ChefSteps, Inc
 * BSD Licensed as described in the file LICENSE
 */
#include <hal_phy.h>
#include <hal_gpio.h>
#include <hal_gpio_regs.h>

void bt_coexist_configure(bt_coexist_mode_t mode, uint8_t bt_active_pin,
                          uint8_t bt_priority_pin)
{
  uint32_t new_mask = 0;
  HAL_GPIO_ConfigType gpio_config =
  {
    .mode = HAL_GPIO_MODE_IN,
    .pull = HAL_GPIO_NO_PULL,
    .sleepable = false
  };

  /* Disable coexistence entirely before changing pin assignments */
  GPIO.OUT &= ~(GPIO_OUT_BT_COEXIST_MASK);

  new_mask = VAL2FIELD_M(GPIO_OUT_BT_ACTIVE_PIN, bt_active_pin) +
  VAL2FIELD_M(GPIO_OUT_BT_PRIORITY_PIN, bt_priority_pin);

  if (mode == BT_COEXIST_USE_BT_ACTIVE || mode
      == BT_COEXIST_USE_BT_ACTIVE_PRIORITY)
  {
    gpio_config.pin = bt_active_pin,
    HAL_GPIO_Init(&gpio_config);
    new_mask |= GPIO_OUT_BT_ACTIVE_ENABLE;
  }
  if (mode == BT_COEXIST_USE_BT_ACTIVE_PRIORITY)
  {
    gpio_config.pin = bt_priority_pin,
    HAL_GPIO_Init(&gpio_config);
    new_mask |= GPIO_OUT_BT_PRIORITY_ENABLE;
  }
  GPIO.OUT |= new_mask;
}

