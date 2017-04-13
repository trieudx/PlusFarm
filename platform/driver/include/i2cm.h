#ifndef __I2CM_H__
#define __I2CM_H__

/* Inclusion section ======================================================== */
#include "hal_gpio.h"

/* Public macro definition section ========================================== */
#define I2CM_SCL_PIN            HAL_GPIO_PIN_12
#define I2CM_SDA_PIN            HAL_GPIO_PIN_13

#define I2CM_TIMEOUT_MS         1000

/* Public type definition section =========================================== */
typedef enum
{
  I2CM_OK                   = 0x00,
  I2CM_TIMED_OUT            = 0x01,
  I2CM_NACK                 = 0x02
} I2CM_ReturnType;

typedef enum
{
  I2CM_SPEED_100KHz         = 0x00,
  I2CM_SPEED_250KHz         = 0x01
} I2CM_SpeedModeType;

typedef enum
{
  I2CM_START_MODE_I2C       = 0x00,
  I2CM_START_MODE_SHTXX     = 0x01
} I2CM_StartModeType;

typedef enum
{
  I2CM_REG_ADDR_ONE_BYTE    = 0x01,
  I2CM_REG_ADDR_TWO_BYTES   = 0x02
} I2CM_RegAddrModeType;

typedef struct
{
  I2CM_StartModeType        start_mode;
  uint8_t                   slave_addr;
  I2CM_SpeedModeType        speed_mode;
  uint16_t                  access_time;    /* millisecond */
  /* The below fields are used for I2CM_Write() and I2CM_Read() APIs only */
  I2CM_RegAddrModeType      reg_addr_mode;
  uint8_t                   reg_size;
} I2CM_SlaveConfigType;

/* Public function prototype section ======================================== */
void I2CM_Init(void);
I2CM_ReturnType I2CM_Transmit(I2CM_SlaveConfigType *slave_config,
                              uint8_t length, uint8_t *data);
I2CM_ReturnType I2CM_Receive(I2CM_SlaveConfigType *slave_config,
                             uint8_t length, uint8_t *data);
I2CM_ReturnType I2CM_Write(I2CM_SlaveConfigType *slave_config,
                           uint16_t reg_addr, uint16_t reg_access_time,
                           uint8_t *data);
I2CM_ReturnType I2CM_Read(I2CM_SlaveConfigType *slave_config,
                          uint16_t reg_addr, uint16_t reg_access_time,
                          uint8_t *data);

#endif
