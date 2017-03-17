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
	I2CM_START_MODE_I2C		= 0x0,
	I2CM_START_MODE_SHTXX	= 0x1
} I2CM_StartMode;

typedef enum
{
	I2CM_REG_ADDR_ONE_BYTE	= 0x1,
	I2CM_REG_ADDR_TWO_BYTES	= 0x2
} I2CM_RegAddrMode;

typedef struct
{
	GPIO_Pin		scl_pin;
	GPIO_Pin		sda_pin;
} I2CM_HwConfig;

typedef struct
{
	I2CM_StartMode		start_mode;
	uint8				slave_addr;
	I2CM_SpeedMode		speed_mode;
	uint16				access_time;	/* millisecond */
	/* The below fields are used for I2CM_Write() and I2CM_Read() APIs only */
	I2CM_RegAddrMode	reg_addr_mode;
	uint8				reg_size;
} I2CM_SlaveConfig;

/* Public function prototype section ======================================== */
void I2CM_Init(I2CM_HwConfig *hw_config);
I2CM_Return I2CM_Transmit(I2CM_SlaveConfig *slave_config,
												uint8 length, uint8 *data);
I2CM_Return I2CM_Receive(I2CM_SlaveConfig *slave_config,
												uint8 length, uint8 *data);
I2CM_Return I2CM_Write(I2CM_SlaveConfig *slave_config,
						uint16 reg_addr, uint16 reg_access_time, uint8 *data);
I2CM_Return I2CM_Read(I2CM_SlaveConfig *slave_config,
						uint16 reg_addr, uint16 reg_access_time, uint8 *data);

#endif
