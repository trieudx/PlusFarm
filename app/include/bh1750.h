#ifndef __BH1750_H__
#define __BH1750_H__

/* Inclusion section ======================================================== */
#include "i2cm.h"

/* Public macro definition section ========================================== */

/* Public type definition section =========================================== */
typedef enum
{
  BH1750_1LX_RES_120MS_MT           = 0x20,
  BH1750_0P5LX_RES_120MS_MT         = 0x21,
  BH1750_4LX_RES_16MS_MT            = 0x23
} BH1750_OpModeType;

/* Public function prototype section ======================================== */
void BH1750_Init(void);
I2CM_ReturnType BH1750_PowerON(void);
I2CM_ReturnType BH1750_Sleep(void);
I2CM_ReturnType BH1750_Reset(void);
I2CM_ReturnType BH1750_ReadAmbientLight(BH1750_OpModeType mode, uint16_t *data);

#endif
