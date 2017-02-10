#include "esp_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"

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

void task_uart(void *param)
{
	uint32 count = 0;
	while (1)
	{
		printf("%d: SDK version:%s\n", ++count, system_get_sdk_version());
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

	xTaskCreate(task_uart, "task_uart", 384, NULL, 5, NULL);
}

