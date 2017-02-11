/* Inclusion section ======================================================== */
#include "esp_common.h"
#include "bh1750.h"
#include "FreeRTOS.h"

/* Private macro definition section ========================================= */
#define BH1750_ADDR_LOW			0x23
#define BH1750_ADDR_HIGH		0x5C

#define BH1750_ADDR				BH1750_ADDR_LOW
#define BH1750_SPEED_MODE		I2CM_SPEED_100KHz

#define BH1750_POWER_DOWN		0x00
#define BH1750_POWER_ON			0x01
#define BH1750_RESET			0x07

#define BH1750_MT_HRES_MS		180
#define BH1750_MT_LRES_MS		30

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */

/* Private variable section ================================================= */

/* Public function definition section ======================================= */
I2CM_Return BH1750_PowerON(void)
{
	uint8 mode = BH1750_POWER_ON;

	return I2CM_Transmit(BH1750_ADDR, BH1750_SPEED_MODE, 1, &mode);
}

I2CM_Return BH1750_Sleep(void)
{
	uint8 mode = BH1750_POWER_DOWN;

	return I2CM_Transmit(BH1750_ADDR, BH1750_SPEED_MODE, 1, &mode);
}

I2CM_Return BH1750_Reset(void)
{
	I2CM_Return ret;
	uint8 mode = BH1750_RESET;

	/* Power ON sensor */
	ret = BH1750_PowerON();
	if (ret == I2CM_OK)
	{
		/* Reset sensor */
		ret = I2CM_Transmit(BH1750_ADDR, BH1750_SPEED_MODE, 1, &mode);
	}

	return ret;
}

I2CM_Return BH1750_ReadAmbientLight(BH1750_OpMode mode, uint16 *data)
{
	I2CM_Return ret;

	/* Set measurement mode */
	ret = I2CM_Transmit(BH1750_ADDR, BH1750_SPEED_MODE, 1, (uint8 *)&mode);
	if (ret == I2CM_OK)
	{
		uint8 uc_data[2];

		/* Wait for measurement completion */
		if (mode == BH1750_4LX_RES_16MS_MT)
			vTaskDelay(BH1750_MT_LRES_MS / portTICK_RATE_MS);
		else
			vTaskDelay(BH1750_MT_HRES_MS / portTICK_RATE_MS);
		/* Read data */
		ret = I2CM_Receive(BH1750_ADDR, BH1750_SPEED_MODE, 2, uc_data);
		if (ret == I2CM_OK)
		{
			/* Get ambient light */
			*data = (((uc_data[0] << 8) | uc_data[1]) * 6) / 5;
		}
	}

	return ret;
}

/* Private function definition section ====================================== */

/* ============================= End of file ================================ */
