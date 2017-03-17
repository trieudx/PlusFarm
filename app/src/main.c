#include "esp_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"
#include "gpio.h"
#include "bh1750.h"
#include "sht1x.h"

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
	flash_size_map size_map = system_get_flash_size_map();
	uint32 rf_cal_sec = 0;

	switch (size_map)
	{
		case FLASH_SIZE_4M_MAP_256_256:
			rf_cal_sec = 128 - 5;
			break;
		case FLASH_SIZE_8M_MAP_512_512:
			rf_cal_sec = 256 - 5;
			break;
		case FLASH_SIZE_16M_MAP_512_512:
		case FLASH_SIZE_16M_MAP_1024_1024:
			rf_cal_sec = 512 - 5;
			break;
		case FLASH_SIZE_32M_MAP_512_512:
		case FLASH_SIZE_32M_MAP_1024_1024:
			rf_cal_sec = 1024 - 5;
			break;
		default:
			rf_cal_sec = 0;
			break;
	}

	return rf_cal_sec;
}

void task_gpio(void *param)
{
	GPIO_Config gpio_config;
	gpio_config.pin = GPIO_PIN_13;
	gpio_config.mode = GPIO_MODE_OUT_PP;
	gpio_config.pull = GPIO_NO_PULL;
	GPIO_Init(&gpio_config);

	uint32 count = 0;
	while (1)
	{
		GPIO_Toggle(gpio_config.pin);
		printf("%d: GPIO%d toggled\n", ++count, gpio_config.pin);
		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void task_i2c(void *param)
{
	I2CM_HwConfig i2cm_hwconfig;
	i2cm_hwconfig.scl_pin = GPIO_PIN_2;
	i2cm_hwconfig.sda_pin = GPIO_PIN_14;
	I2CM_Init(&i2cm_hwconfig);

	uint32 count = 0;
	uint16 data;

	while (1)
	{
		if (BH1750_ReadAmbientLight(BH1750_0P5LX_RES_120MS_MT, &data)
			== I2CM_OK)
		{
			printf("%d: Current ambient light from BH1750: 0x%04X\n",
								++count, data);
		}
		else
			printf("%d: Timeout when reading BH1750\n", ++count);

		if (SHT1X_ReadTemperature(&data) == I2CM_OK)
			printf("Current temperature: 0x%04X\n", data);
		else
			printf("Timeout when reading SHT1x\n");

		if (SHT1X_ReadRelativeHumidity(&data) == I2CM_OK)
			printf("Current RH: 0x%04X\n", data);
		else
			printf("Timeout when reading SHT1X\n");

		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void user_init(void)
{
	/* Initialize UART for printing log */
	UART_Config uart_config;
	uart_config.port_no = UART_PORT0;
	uart_config.baud_rate = UART_BAUD_RATE_460800;
	uart_config.word_length = UART_WORD_LENGTH_8b;
	uart_config.parity = UART_PARITY_NONE;
	uart_config.stop_bits = UART_STOP_BITS_1;
	uart_config.hw_flow_ctrl = UART_HW_FLOW_CTRL_NONE;
	UART_Init(&uart_config);

	/* Test GPIO */
	xTaskCreate(task_gpio, "task_gpio", 128, NULL, 5, NULL);
	/* Test I2C */
	xTaskCreate(task_i2c, "task_i2c", 128, NULL, 5, NULL);
}

