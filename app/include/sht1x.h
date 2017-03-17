#ifndef __SHT1X_H__
#define __SHT1X_H__

/* Inclusion section ======================================================== */
#include "i2cm.h"

/* Public macro definition section ========================================== */

/* Public type definition section =========================================== */

/* Public function prototype section ======================================== */
void SHT1X_Init(void);
I2CM_Return SHT1X_ReadTemperature(uint16 *data);
I2CM_Return SHT1X_ReadRelativeHumidity(uint16 *data);

#endif
