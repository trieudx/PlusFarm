/* Inclusion section ======================================================== */
#include "esp_common.h"
#include "assert.h"
#include "log.h"

/* Private macro definition section ========================================= */

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */

/* Private variable section ================================================= */

/* Public function definition section ======================================= */
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *func, uint32_t line)
{
	/* Disable interrupt */
	ETS_INTR_LOCK();
	/* Assert */
	printf("[%s: %d] %s!\r\n", func, (unsigned int)line, __FUNCTION__);
	/* Infinite loop */
	while (1);
}
#endif

/* Private function definition section ====================================== */

/* ============================= End of file ================================ */
