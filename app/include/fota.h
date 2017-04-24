#ifndef __FOTA_H__
#define __FOTA_H__

/* Inclusion section ======================================================== */
#include "tftp.h"

/* Public macro definition section ========================================== */

/* Public type definition section =========================================== */

/* Public function prototype section ======================================== */
void fota_task(void *param);

/* Attempt to make a TFTP client connection and download the specified filename.

 'timeout' is in milliseconds, and is timeout for any UDP exchange
 _not_ the entire download.

 Returns 0 on success, LWIP err.h values for errors.

 Does not change the current firmware slot, or reboot.

 receive_cb: called repeatedly after each successful packet that
 has been written to flash and ACKed.  Can pass NULL to omit.
 */
err_t fota_download(const char *server, int port, const char *filename,
                    int timeout, int ota_slot, tftp_receive_cb receive_cb);

#endif
