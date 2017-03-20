#include "esp_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"
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
	gpio_config.pin = GPIO_PIN_15;
	gpio_config.mode = GPIO_MODE_OUT_PP;
	gpio_config.pull = GPIO_NO_PULL;
	GPIO_Init(&gpio_config);

	while (1)
	{
		GPIO_Toggle(gpio_config.pin);
		LOG_PRINTF("GPIO%d toggled", gpio_config.pin);

		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void task_bh1750(void *param)
{
	uint16 data;

	BH1750_Init();

	while (1)
	{
		if (BH1750_ReadAmbientLight(BH1750_0P5LX_RES_120MS_MT, &data)
			== I2CM_OK)
		{
			LOG_PRINTF("Current ambient light from BH1750: 0x%04X", data);
		}
		else
			LOG_PRINTF("Timeout when reading BH1750");

		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void task_sht1x(void *param)
{
	uint16 data;

	SHT1X_Init();

	while (1)
	{
		if (SHT1X_ReadTemperature(&data) == I2CM_OK)
			LOG_PRINTF("Current temperature: 0x%04X", data);
		else
			LOG_PRINTF("Timeout when reading SHT1x");

		if (SHT1X_ReadRelativeHumidity(&data) == I2CM_OK)
			LOG_PRINTF("Current RH: 0x%04X", data);
		else
			LOG_PRINTF("Timeout when reading SHT1x");

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
	/* Initialize log */
	Log_Init();

	/* Test GPIO */
	xTaskCreate(task_gpio, "task_gpio", 256, NULL, 5, NULL);
	/* Test BH1750 */
	xTaskCreate(task_bh1750, "task_bh1750", 256, NULL, 5, NULL);
	/* Test SHT1X */
	xTaskCreate(task_sht1x, "task_sht1x", 256, NULL, 5, NULL);
}

