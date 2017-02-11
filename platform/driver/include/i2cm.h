#ifndef __I2CM_H__
#define __I2CM_H__

/* Inclusion section ======================================================== */
#include "gpio.h"

/* Public macro definition section ========================================== */

/* Public type definition section =========================================== */
typedef enum
{
	I2CM_OK			= 0x0,
	I2CM_ERROR		= 0x1
} I2CM_Return;

typedef enum
{
	I2CM_SPEED_100KHz	= 0x0,
	I2CM_SPEED_250KHz	= 0x1
} I2CM_SpeedMode;

typedef enum
{
	I2CM_ONE_BYTE	= 0x1,
	I2CM_TWO_BYTES	= 0x2
} I2CM_AddrMode;

typedef struct
{
	GPIO_Pin		scl_pin;
	GPIO_Pin		sda_pin;
} I2CM_HwConfig;

typedef struct
{
	uint8			slave_addr;
	I2CM_SpeedMode	speed_mode;
	I2CM_AddrMode	reg_addr_mode;
	uint8			reg_size;
} I2CM_SlaveConfig;

/* Public function prototype section ======================================== */
void I2CM_Init(I2CM_HwConfig *hw_config);
I2CM_Return I2CM_Write(I2CM_SlaveConfig *slave_config,
											uint16 reg_addr, uint8 *data);
I2CM_Return I2CM_Read(I2CM_SlaveConfig *slave_config,
											uint16 reg_addr, uint8 *data);

#endif
