/* Inclusion section ======================================================== */
#include "esp_common.h"
#include "i2cm.h"
#include "gpio.h"
#include "FreeRTOS.h"

/* Private macro definition section ========================================= */
#define I2CM_WRITE						0x00
#define I2CM_READ						0x01

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */
static void i2cm_set_speed(I2CM_SpeedMode speed_mode);
static void i2cm_start(I2CM_StartMode start_mode);
static void i2cm_stop(void);
static I2CM_Return i2cm_access(I2CM_SlaveConfig *slave_config, uint16 reg_addr);
static I2CM_Return i2cm_tx_one_byte(uint8 data, bool stop);
static void i2cm_rx_one_byte(uint8 *data, bool stop);

/* Private variable section ================================================= */
static GPIO_Pin		i2cm_scl_pin;
static GPIO_Pin		i2cm_sda_pin;
static uint8		i2cm_delay_half_cycle;
static uint8		i2cm_delay_quarter_cycle;

/* Public function definition section ======================================= */
void I2CM_Init(I2CM_HwConfig *hw_config)
{
	GPIO_Config gpio_scl, gpio_sda;

	/* Initialize SCL pin */
	gpio_scl.pin = hw_config->scl_pin;
	gpio_scl.mode = GPIO_MODE_OUT_OD;
	gpio_scl.pull = GPIO_PULL_UP;
	GPIO_Init(&gpio_scl);

	/* Initialize SDA pin */
	gpio_sda.pin = hw_config->sda_pin;
	gpio_sda.mode = GPIO_MODE_OUT_OD;
	gpio_sda.pull = GPIO_PULL_UP;
	GPIO_Init(&gpio_sda);

	/* Release I2C bus */
	GPIO_SetHigh(gpio_scl.pin);
	GPIO_SetHigh(gpio_sda.pin);

	/* Store HW configuration */
	i2cm_scl_pin = hw_config->scl_pin;
	i2cm_sda_pin = hw_config->sda_pin;
}

I2CM_Return I2CM_Transmit(I2CM_SlaveConfig *slave_config,
												uint8 length, uint8 *data)
{
	I2CM_Return ret;
	uint8 i;

	/* Set speed mode */
	i2cm_set_speed(slave_config->speed_mode);

	/* Send START condition */
	i2cm_start(slave_config->start_mode);

	/* Send slave address with WRITE action */
	ret = i2cm_tx_one_byte((slave_config->slave_addr << 1) | I2CM_WRITE, false);

	if (ret == I2CM_OK)
	{
		/* Wait for completion of slave access */
		vTaskDelay(slave_config->access_time / portTICK_RATE_MS);
		/* Transmit data to slave */
		for (i = 0; (ret == I2CM_OK) && (i < length); i++)
		{
			ret = i2cm_tx_one_byte(data[i], (i == (length - 1)));
		}
	}

	return ret;
}

I2CM_Return I2CM_Receive(I2CM_SlaveConfig *slave_config,
												uint8 length, uint8 *data)
{
	I2CM_Return ret;
	uint8 i;

	/* Set speed mode */
	i2cm_set_speed(slave_config->speed_mode);

	/* Send START condition */
	i2cm_start(slave_config->start_mode);

	/* Send slave address with READ action */
	ret = i2cm_tx_one_byte((slave_config->slave_addr << 1) | I2CM_READ, false);

	if (ret == I2CM_OK)
	{
		/* Wait for completion of slave access */
		vTaskDelay(slave_config->access_time / portTICK_RATE_MS);
		/* Receive data from slave */
		for (i = 0; (ret == I2CM_OK) && (i < length); i++)
		{
			i2cm_rx_one_byte(data + i, (i == (length - 1)));
		}
	}

	return ret;
}

I2CM_Return I2CM_Write(I2CM_SlaveConfig *slave_config,
						uint16 reg_addr, uint16 reg_access_time, uint8 *data)
{
	I2CM_Return ret;

	/* Access to the register of slave */
	ret = i2cm_access(slave_config, reg_addr);

	if (ret == I2CM_OK)
	{
		/* Wait for completion of register access */
		vTaskDelay(reg_access_time / portTICK_RATE_MS);
		/* Write data to this register */
		ret = I2CM_Transmit(slave_config, slave_config->reg_size, data);
	}

	return ret;
}

I2CM_Return I2CM_Read(I2CM_SlaveConfig *slave_config,
						uint16 reg_addr, uint16 reg_access_time, uint8 *data)
{
	I2CM_Return ret;

	/* Access to the register of slave */
	ret = i2cm_access(slave_config, reg_addr);

	if (ret == I2CM_OK)
	{
		/* Wait for completion of register access */
		vTaskDelay(reg_access_time / portTICK_RATE_MS);
		/* Read data from this register */
		ret = I2CM_Receive(slave_config, slave_config->reg_size, data);
	}

	return ret;
}

/* Private function definition section ====================================== */
static void i2cm_set_speed(I2CM_SpeedMode speed_mode)
{
	if (speed_mode == I2CM_SPEED_100KHz)
	{
		i2cm_delay_half_cycle = 4;
		i2cm_delay_quarter_cycle = 3;
	}
	else
	{
		i2cm_delay_half_cycle = 2;
		i2cm_delay_quarter_cycle = 1;
	}
}

static void i2cm_start(I2CM_StartMode start_mode)
{
	if (start_mode == I2CM_START_MODE_I2C)
	{
		GPIO_SetLow(i2cm_sda_pin);
		os_delay_us(i2cm_delay_half_cycle);
		GPIO_SetLow(i2cm_scl_pin);
		os_delay_us(i2cm_delay_quarter_cycle);
	}
	else
	{
		GPIO_SetLow(i2cm_scl_pin);
		os_delay_us(i2cm_delay_quarter_cycle);
		GPIO_SetHigh(i2cm_scl_pin);
		os_delay_us(i2cm_delay_quarter_cycle);
		GPIO_SetLow(i2cm_sda_pin);
		os_delay_us(i2cm_delay_quarter_cycle);
		GPIO_SetLow(i2cm_scl_pin);
		os_delay_us(i2cm_delay_half_cycle);
		GPIO_SetHigh(i2cm_scl_pin);
		os_delay_us(i2cm_delay_quarter_cycle);
		GPIO_SetHigh(i2cm_sda_pin);
		os_delay_us(i2cm_delay_quarter_cycle);
		GPIO_SetLow(i2cm_scl_pin);
		os_delay_us(i2cm_delay_quarter_cycle);
	}
}

static void i2cm_stop(void)
{
	GPIO_SetLow(i2cm_sda_pin);
	os_delay_us(i2cm_delay_quarter_cycle);
	GPIO_SetHigh(i2cm_scl_pin);
	os_delay_us(i2cm_delay_quarter_cycle);
	GPIO_SetHigh(i2cm_sda_pin);
	os_delay_us(i2cm_delay_half_cycle);
}

static I2CM_Return i2cm_access(I2CM_SlaveConfig *slave_config, uint16 reg_addr)
{
	uint8 reg[2];
	uint8 reg_len = 0;

	if (slave_config->reg_addr_mode == I2CM_REG_ADDR_TWO_BYTES)
		reg[reg_len++] = reg_addr >> 8;
	reg[reg_len++] = (uint8)reg_addr;

	return I2CM_Transmit(slave_config, reg_len, reg);
}

static I2CM_Return i2cm_tx_one_byte(uint8 data, bool stop)
{
	int8 i;
	bool ACK;

	/* Send one byte to slave */
	for (i = 7; i >= 0; i--)
	{
		GPIO_Set(i2cm_sda_pin, (data >> i) & BIT0);
		os_delay_us(i2cm_delay_quarter_cycle);
		GPIO_SetHigh(i2cm_scl_pin);
		os_delay_us(i2cm_delay_half_cycle);
		GPIO_SetLow(i2cm_scl_pin);
		os_delay_us(i2cm_delay_quarter_cycle);
	}

	/* Read back ACK from slave */
	GPIO_SetHigh(i2cm_sda_pin);
	os_delay_us(i2cm_delay_quarter_cycle);
	GPIO_SetHigh(i2cm_scl_pin);
	os_delay_us(i2cm_delay_half_cycle);
	ACK = !GPIO_Read(i2cm_sda_pin);
	GPIO_SetLow(i2cm_scl_pin);
	os_delay_us(i2cm_delay_quarter_cycle);

	if (!ACK || stop)
	{
		/* Send STOP condition */
		i2cm_stop();
	}

	/* Return status */
	return ACK ? I2CM_OK : I2CM_ERROR;
}

static void i2cm_rx_one_byte(uint8 *data, bool stop)
{
	int8 i;

	/* Read one byte from slave */
	*data = 0;
	for (i = 7; i >= 0; i--)
	{
		os_delay_us(i2cm_delay_quarter_cycle);
		GPIO_SetHigh(i2cm_scl_pin);
		os_delay_us(i2cm_delay_half_cycle);
		*data |= GPIO_Read(i2cm_sda_pin) << i;
		GPIO_SetLow(i2cm_scl_pin);
		os_delay_us(i2cm_delay_quarter_cycle);
	}

	/* Send ACK/NACK to slave */
	if (!stop)
		GPIO_SetLow(i2cm_sda_pin);
	else
		GPIO_SetHigh(i2cm_sda_pin);
	os_delay_us(i2cm_delay_quarter_cycle);
	GPIO_SetHigh(i2cm_scl_pin);
	os_delay_us(i2cm_delay_half_cycle);
	GPIO_SetLow(i2cm_scl_pin);
	os_delay_us(i2cm_delay_quarter_cycle);
	GPIO_SetHigh(i2cm_sda_pin);

	if (stop)
	{
		/* Send STOP condition */
		i2cm_stop();
	}
}

/* ============================= End of file ================================ */
