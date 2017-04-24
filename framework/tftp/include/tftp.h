#ifndef _TFTP_H_
#define _TFTP_H_

#include "lwip/lwip_err.h"
#include "lwip/lwip_api.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*tftp_receive_cb)(size_t bytes_received);

err_t tftp_receive_data(struct netconn *nc, size_t write_offs,
                        size_t limit_offs, size_t *received_len,
                        ip_addr_t *peer_addr, int peer_port,
                        tftp_receive_cb receive_cb);
err_t tftp_send_ack(struct netconn *nc, int block);
err_t tftp_send_rrq(struct netconn *nc, const char *filename);
void tftp_send_error(struct netconn *nc, int err_code, const char *err_msg);

#ifdef __cplusplus
}
#endif

#endif
