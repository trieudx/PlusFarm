#ifndef __ASSERT_H__
#define __ASSERT_H__

/* Inclusion section ======================================================== */

/* Public macro definition section ========================================== */
#ifndef USE_FULL_ASSERT
#define USE_FULL_ASSERT
#endif

#ifdef USE_FULL_ASSERT
#define assert_param(prt) ((prt) ? (void)0 : \
						assert_failed((uint8_t *)__FUNCTION__, __LINE__))
#else
#define assert_param(prt)
#endif

/* Public type definition section =========================================== */

/* Public function prototype section ======================================== */
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *func, uint32_t line);
#endif

#endif
