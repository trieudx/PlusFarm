/* Inclusion section ======================================================== */
#include "sdk/esp_common.h"
#include "bh1750.h"

/* Private macro definition section ========================================= */
#define BH1750_ADDR_LOW         0x23
#define BH1750_ADDR_HIGH        0x5C

#define BH1750_POWER_DOWN       0x00
#define BH1750_POWER_ON         0x01
#define BH1750_RESET            0x07

#define BH1750_HRES_MT_MS       180
#define BH1750_LRES_MT_MS       30

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */

/* Private variable section ================================================= */
static I2CM_SlaveConfigType     bh1750_config;

/* Public function definition section ======================================= */
void BH1750_Init(void)
{
  /* Initialize I2C master channel */
  I2CM_Init();

  /* Configure I2C parameters for BH1750 */
  bh1750_config.start_mode = I2CM_START_MODE_I2C;
  bh1750_config.slave_addr = BH1750_ADDR_LOW;
  bh1750_config.speed_mode = I2CM_SPEED_250KHz;
  bh1750_config.access_time = 0;
  bh1750_config.reg_addr_mode = I2CM_REG_ADDR_ONE_BYTE;
  bh1750_config.reg_size = 2;
}

I2CM_ReturnType BH1750_PowerON(void)
{
  uint8_t mode = BH1750_POWER_ON;

  return I2CM_Transmit(&bh1750_config, 1, &mode);
}

I2CM_ReturnType BH1750_Sleep(void)
{
  uint8_t mode = BH1750_POWER_DOWN;

  return I2CM_Transmit(&bh1750_config, 1, &mode);
}

I2CM_ReturnType BH1750_Reset(void)
{
  I2CM_ReturnType ret;
  uint8_t mode = BH1750_RESET;

  /* Power ON sensor */
  ret = BH1750_PowerON();
  if (ret == I2CM_OK)
  {
    /* Reset sensor */
    ret = I2CM_Transmit(&bh1750_config, 1, &mode);
  }

  return ret;
}

I2CM_ReturnType BH1750_ReadAmbientLight(BH1750_OpModeType mode, uint16_t *data)
{
  I2CM_ReturnType ret;
  uint8_t uc_data[2];

  ret = I2CM_Read(&bh1750_config, mode,
                  (mode == BH1750_4LX_RES_16MS_MT) ?
                      BH1750_LRES_MT_MS : BH1750_HRES_MT_MS,
                  uc_data);

  if (ret == I2CM_OK)
  {
    /* Get ambient light value */
    *data = (((uc_data[0] << 8) | uc_data[1]) * 5) / 6;
  }

  return ret;
}

/* Private function definition section ====================================== */

/* ============================= End of file ================================ */
