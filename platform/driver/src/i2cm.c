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
static void i2cm_start(void);
static void i2cm_stop(void);
static I2CM_Return i2cm_transmit(uint8 slave_addr, uint8 length, uint8 *data);
static I2CM_Return i2cm_receive(uint8 slave_addr, uint8 length, uint8 *data);
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

I2CM_Return I2CM_Write(I2CM_SlaveConfig *slave_config,
							uint16 reg_addr, uint8 access_time_ms, uint8 *data)
{
	I2CM_Return ret;
	uint8 reg_value[2];
	uint8 reg_len = 0;

	if (slave_config->reg_addr_mode == I2CM_TWO_BYTES)
		reg_value[reg_len++] = reg_addr >> 8;
	reg_value[reg_len++] = (uint8)reg_addr;

	/* Set speed mode */
	i2cm_set_speed(slave_config->speed_mode);

	/* Access to the register of slave */
	ret = i2cm_transmit(slave_config->slave_addr, reg_len, reg_value);

	if (ret == I2CM_OK)
	{
		vTaskDelay(access_time_ms / portTICK_RATE_MS);
		if (data != NULL)
		{
			/* Write data to this register */
			ret = i2cm_transmit(slave_config->slave_addr,
											slave_config->reg_size, data);
		}
	}

	return ret;
}

I2CM_Return I2CM_Read(I2CM_SlaveConfig *slave_config,
							uint16 reg_addr, uint8 access_time_ms, uint8 *data)
{
	I2CM_Return ret;
	uint8 reg_value[2];
	uint8 reg_len = 0;

	if (slave_config->reg_addr_mode == I2CM_TWO_BYTES)
		reg_value[reg_len++] = reg_addr >> 8;
	reg_value[reg_len++] = (uint8)reg_addr;

	/* Set speed mode */
	i2cm_set_speed(slave_config->speed_mode);

	/* Access to the register of slave */
	ret = i2cm_transmit(slave_config->slave_addr, reg_len, reg_value);

	if (ret == I2CM_OK)
	{
		vTaskDelay(access_time_ms / portTICK_RATE_MS);
		if (data != NULL)
		{
			/* Read data from this register */
			ret = i2cm_receive(slave_config->slave_addr,
											slave_config->reg_size, data);
		}
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

static void i2cm_start(void)
{
	GPIO_SetLow(i2cm_sda_pin);
	os_delay_us(i2cm_delay_half_cycle);
	GPIO_SetLow(i2cm_scl_pin);
	os_delay_us(i2cm_delay_quarter_cycle);
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

static I2CM_Return i2cm_transmit(uint8 slave_addr, uint8 length, uint8 *data)
{
	I2CM_Return ret;
	uint8 i;

	/* Send START condition */
	i2cm_start();

	/* Send slave address with WRITE action */
	ret = i2cm_tx_one_byte((slave_addr << 1) | I2CM_WRITE, false);

	/* Transmit data to slave */
	for (i = 0; (ret == I2CM_OK) && (i < length); i++)
	{
		ret = i2cm_tx_one_byte(data[i], (i == (length - 1)));
	}

	return ret;
}

static I2CM_Return i2cm_receive(uint8 slave_addr, uint8 length, uint8 *data)
{
	I2CM_Return ret;
	uint8 i;

	/* Send START condition */
	i2cm_start();

	/* Send slave address with READ action */
	ret = i2cm_tx_one_byte((slave_addr << 1) | I2CM_READ, false);

	/* Receive data from slave */
	for (i = 0; (ret == I2CM_OK) && (i < length); i++)
	{
		i2cm_rx_one_byte(data + i, (i == (length - 1)));
	}

	return ret;
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
