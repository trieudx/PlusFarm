/* Inclusion section ======================================================== */
#include "hal_gpio.h"
#include "hal_gpio_regs.h"
#include "hal_rtc_regs.h"
#include "hal_iomux.h"
#include "hal_iomux_regs.h"
#include "hal_interrupts.h"
#include "common_macros.h"

/* Private macro definition section ========================================= */

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */
static void IRAM gpio_isr(void);

/* Private variable section ================================================= */
static void (*irq_handlers[16])(HAL_GPIO_PinType pin);

/* Public function definition section ======================================= */
void HAL_GPIO_Init(HAL_GPIO_ConfigType *config)
{
  if (config->pin != HAL_GPIO_PIN_16)
  {
    /* Set mode */
    if (config->mode == HAL_GPIO_MODE_IN)
    {
      iomux_set_gpio_function(config->pin, false);
      GPIO.ENABLE_OUT_CLEAR = BIT(config->pin);
    }
    else
    {
      iomux_set_gpio_function(config->pin, true);
      GPIO.ENABLE_OUT_SET = BIT(config->pin);

      if (config->mode == HAL_GPIO_MODE_OUT_PP)
        GPIO.CONF[config->pin] &= ~GPIO_CONF_OPEN_DRAIN;
      else
      {
        GPIO.CONF[config->pin] |= GPIO_CONF_OPEN_DRAIN;

        /* Configure internal pull-up resistor */
        if (config->pull == HAL_GPIO_PULL_UP)
          iomux_set_pullup_flags(gpio_to_iomux(config->pin), IOMUX_PIN_PULLUP);
      }
    }
  }
  else
  {
    /* Select GPIO functionality */
    RTC.GPIO_CFG[3] = (RTC.GPIO_CFG[3] & 0xffffffbc) | 1;
    RTC.GPIO_CONF = (RTC.GPIO_CONF & 0xfffffffe) | 0;
    /* Set mode */
    if (config->mode == HAL_GPIO_MODE_IN)
    {
      RTC.GPIO_ENABLE = (RTC.GPIO_OUT & 0xfffffffe);
    }
    else
    {
      RTC.GPIO_ENABLE = (RTC.GPIO_OUT & 0xfffffffe) | 1;
    }
  }
}

void HAL_GPIO_DeInit(HAL_GPIO_PinType pin)
{
  GPIO.ENABLE_OUT_CLEAR = BIT(pin);
  *gpio_iomux_reg(pin) &= ~IOMUX_PIN_OUTPUT_ENABLE;
}

void HAL_GPIO_SetLow(HAL_GPIO_PinType pin)
{
  HAL_GPIO_Set(pin, 0);
}

void HAL_GPIO_SetHigh(HAL_GPIO_PinType pin)
{
  HAL_GPIO_Set(pin, 1);
}

void HAL_GPIO_Toggle(HAL_GPIO_PinType pin)
{
  if (pin != HAL_GPIO_PIN_16)
    HAL_GPIO_Set(pin, !((GPIO.OUT >> pin) & BIT(0)));
}

void HAL_GPIO_Set(HAL_GPIO_PinType pin, bool level)
{
  if (pin != 16)
  {
    if (level == 1)
      GPIO.OUT_SET = 1 << pin;
    else
      GPIO.OUT_CLEAR = 1 << pin;
  }
  else
  {
    RTC.GPIO_OUT = (RTC.GPIO_OUT & 0xfffffffe) | level;
  }
}

bool HAL_GPIO_Read(HAL_GPIO_PinType pin)
{
  if (pin != 16)
    return (GPIO.IN >> pin) & BIT(0);
  else
    return RTC.GPIO_IN & BIT(0);
}

void HAL_GPIO_EnableIrq(HAL_GPIO_IntrType *intr)
{
  GPIO.CONF[intr->pin] = SET_FIELD(GPIO.CONF[intr->pin], GPIO_CONF_INTTYPE,
                                   intr->intr_type);
  _xt_isr_attach(INUM_GPIO, gpio_isr);
  _xt_isr_unmask(1 << INUM_GPIO);

  irq_handlers[intr->pin] = intr->irq_handler;
}

void HAL_GPIO_DisableIrq(HAL_GPIO_PinType pin)
{

}

/* Private function definition section ====================================== */
static void IRAM gpio_isr(void)
{
  uint32_t status_reg = GPIO.STATUS;
  GPIO.STATUS_CLEAR = status_reg;

  uint8_t gpio_idx;
  while((gpio_idx = __builtin_ffs(status_reg)))
  {
    gpio_idx--;
    status_reg &= ~BIT(gpio_idx);
    if(FIELD2VAL(GPIO_CONF_INTTYPE, GPIO.CONF[gpio_idx]))
    {
      if (irq_handlers[gpio_idx])
      {
        irq_handlers[gpio_idx](gpio_idx);
      }
    }
  }
}

/* ============================= End of file ================================ */
