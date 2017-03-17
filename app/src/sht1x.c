/* Inclusion section ======================================================== */
#include "esp_common.h"
#include "sht1x.h"
#include "FreeRTOS.h"

/* Private macro definition section ========================================= */
#define SHT1X_TEMPERATURE		0x01
#define SHT1X_HUMIDITY			0x02

#define SHT1X_T_MT_MS			320
#define SHT1X_RH_MT_MS			80

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */
static I2CM_Return sht1x_read(uint8 mode, uint16 access_time, uint16 *data);

/* Private variable section ================================================= */
static I2CM_SlaveConfig sht1x_config;

/* Public function definition section ======================================= */
void SHT1X_Init(void)
{
	/* Initialize I2C master channel */
	I2CM_Init();

	/* Configure I2C parameters for SHT1X */
	sht1x_config.start_mode = I2CM_START_MODE_SHTXX;
	sht1x_config.speed_mode = I2CM_SPEED_100KHz;
}

I2CM_Return SHT1X_ReadTemperature(uint16 *data)
{
	return sht1x_read(SHT1X_TEMPERATURE, SHT1X_T_MT_MS, data);
}

I2CM_Return SHT1X_ReadRelativeHumidity(uint16 *data)
{
	return sht1x_read(SHT1X_HUMIDITY, SHT1X_RH_MT_MS, data);
}

/* Private function definition section ====================================== */
static I2CM_Return sht1x_read(uint8 mode, uint16 access_time, uint16 *data)
{
	I2CM_Return ret;
	uint8 uc_data[2];

	sht1x_config.slave_addr = mode;
	sht1x_config.access_time = access_time;

	ret = I2CM_Receive(&sht1x_config, 2, uc_data);
	if (ret == I2CM_OK)
	{
		/* Get temperature value */
		*data = (uc_data[0] << 8) | uc_data[1];
	}

	return ret;
}

/* ============================= End of file ================================ */
