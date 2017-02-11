/* Inclusion section ======================================================== */
#include "esp_common.h"
#include "gpio.h"

/* Private macro definition section ========================================= */
#define PERIPHS_GPIO_BASEADDR				0x60000300

#define GPIO_OUT_ADDRESS					0x00
#define GPIO_OUT_W1TS_ADDRESS				0x04
#define GPIO_OUT_W1TC_ADDRESS				0x08

#define GPIO_ENABLE_ADDRESS					0x0C
#define GPIO_ENABLE_W1TS_ADDRESS			0x10
#define GPIO_ENABLE_W1TC_ADDRESS			0x14

#define GPIO_IN_ADDRESS						0x18

#define GPIO_PIN_INT_TYPE_MSB				9
#define GPIO_PIN_INT_TYPE_LSB				7
#define GPIO_PIN_INT_TYPE_MASK				(0x00000007<<GPIO_PIN_INT_TYPE_LSB)

#define GPIO_PAD_DRIVER_ENABLE				1
#define GPIO_PAD_DRIVER_DISABLE				(~GPIO_PAD_DRIVER_ENABLE)
#define GPIO_PIN_DRIVER_MSB					2
#define GPIO_PIN_DRIVER_LSB					2
#define GPIO_PIN_DRIVER_MASK				(0x00000001 << GPIO_PIN_DRIVER_LSB)

#define GPIO_PIN0_ADDRESS					0x28

#define GPIO_REG_READ(reg)	READ_PERI_REG(PERIPHS_GPIO_BASEADDR + reg)
#define GPIO_REG_WRITE(reg, val)	\
							WRITE_PERI_REG(PERIPHS_GPIO_BASEADDR + reg, val)

#define GPIO_PIN_REG(i)		(i == 0) ? PERIPHS_IO_MUX_GPIO0_U:		\
							(i == 1) ? PERIPHS_IO_MUX_U0TXD_U:		\
							(i == 2) ? PERIPHS_IO_MUX_GPIO2_U:		\
							(i == 3) ? PERIPHS_IO_MUX_U0RXD_U:		\
							(i == 4) ? PERIPHS_IO_MUX_GPIO4_U:		\
							(i == 5) ? PERIPHS_IO_MUX_GPIO5_U:		\
							(i == 6) ? PERIPHS_IO_MUX_SD_CLK_U:		\
							(i == 7) ? PERIPHS_IO_MUX_SD_DATA0_U:	\
							(i == 8) ? PERIPHS_IO_MUX_SD_DATA1_U:	\
							(i == 9) ? PERIPHS_IO_MUX_SD_DATA2_U:	\
							(i == 10)? PERIPHS_IO_MUX_SD_DATA3_U:	\
							(i == 11)? PERIPHS_IO_MUX_SD_CMD_U:		\
							(i == 12)? PERIPHS_IO_MUX_MTDI_U:		\
							(i == 13)? PERIPHS_IO_MUX_MTCK_U:		\
							(i == 14)? PERIPHS_IO_MUX_MTMS_U:		\
							PERIPHS_IO_MUX_MTDO_U

#define GPIO_PIN_ADDR(i)	(GPIO_PIN0_ADDRESS + i * 4)

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */

/* Public function definition section ======================================= */
void GPIO_Init(GPIO_Config *config)
{
	if (config->pin != GPIO_PIN_16)
	{
		uint32 reg_addr = GPIO_PIN_REG(config->pin);
		uint32 reg_val;

		/* Select GPIO functionality */
		if ((1 << config->pin) & (BIT0 | BIT2 | BIT4 | BIT5))
			PIN_FUNC_SELECT(reg_addr, 0);
		else
			PIN_FUNC_SELECT(reg_addr, 3);

		/* Set mode */
		if (config->mode == GPIO_MODE_IN)
			GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, 1 << config->pin);
		else
		{
			GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 1 << config->pin);

			if (config->mode == GPIO_MODE_OUT_OD)
			{
				reg_val = GPIO_REG_READ(GPIO_PIN_ADDR(config->pin));
				reg_val &= (~GPIO_PIN_DRIVER_MASK);
				reg_val |= (GPIO_PAD_DRIVER_ENABLE << GPIO_PIN_DRIVER_LSB);
				GPIO_REG_WRITE(GPIO_PIN_ADDR(config->pin), reg_val);
			}
		}

		/* Configure internal pull-up resistor */
		if (config->pull == GPIO_PULL_UP)
			PIN_PULLUP_EN(reg_addr);
		else
			PIN_PULLUP_DIS(reg_addr);

		/* Disable interrupt */
		reg_val = GPIO_REG_READ(GPIO_PIN_ADDR(config->pin));
		reg_val &= (~GPIO_PIN_INT_TYPE_MASK);
		GPIO_REG_WRITE(GPIO_PIN_ADDR(config->pin), reg_val);
	}
	else
	{
		/* Select GPIO functionality */
		WRITE_PERI_REG(PAD_XPD_DCDC_CONF,
						(READ_PERI_REG(PAD_XPD_DCDC_CONF) & 0xFFFFFFBC) | BIT0);
		WRITE_PERI_REG(RTC_GPIO_CONF,
						(READ_PERI_REG(RTC_GPIO_CONF) & (uint32)0xFFFFFFFE));

		/* Set mode */
		if (config->mode == GPIO_MODE_IN)
		{
			WRITE_PERI_REG(RTC_GPIO_ENABLE,
				READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xFFFFFFFE);
		}
		else
		{
			WRITE_PERI_REG(RTC_GPIO_ENABLE,
				(READ_PERI_REG(RTC_GPIO_ENABLE) & (uint32)0xFFFFFFFE) | BIT0);
		}
	}

}

void GPIO_SetLow(GPIO_Pin pin)
{
	GPIO_Set(pin, 0);
}

void GPIO_SetHigh(GPIO_Pin pin)
{
	GPIO_Set(pin, 1);
}

void GPIO_Toggle(GPIO_Pin pin)
{
	if (pin != GPIO_PIN_16)
		GPIO_Set(pin, !((GPIO_REG_READ(GPIO_OUT_ADDRESS) >> pin) & BIT0));
}

void GPIO_Set(GPIO_Pin pin, bool level)
{
	if (pin != GPIO_PIN_16)
	{
		if (level == 0)
			GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1 << pin);
		else
			GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1 << pin);
	}
	else
	{
		WRITE_PERI_REG(RTC_GPIO_OUT,
					(READ_PERI_REG(RTC_GPIO_OUT) & (uint32)0xFFFFFFFE) | level);
	}
}

bool GPIO_Read(GPIO_Pin pin)
{
	if (pin != GPIO_PIN_16)
		return ((GPIO_REG_READ(GPIO_IN_ADDRESS) >> pin) & BIT0);
	else
		return (READ_PERI_REG(RTC_GPIO_IN_DATA) & BIT0);
}

/* Private function definition section ====================================== */

/* ============================= End of file ================================ */
