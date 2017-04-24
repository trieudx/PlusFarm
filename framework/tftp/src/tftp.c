/* TFTP Server OTA support
 *
 * For details of use see ota-tftp.h
 *
 * Part of esp-open-rtos
 * Copyright (C) 2015 Superhouse Automation Pty Ltd
 * BSD Licensed as described in the file LICENSE
 */
#include <freertos.h>
#include <string.h>
#include <strings.h>

#include "lwip/lwip_err.h"
#include "lwip/lwip_api.h"
#include "lwip/lwip_sys.h"
#include "lwip/lwip_netdb.h"
#include "lwip/lwip_dns.h"
#include "lwip/lwip_mem.h"

#include <lwip_netbuf_helpers.h>
#include <sdk/spi_flash.h>
#include <sdk/esp_system.h>

#include "tftp.h"
#include "rboot.h"
#include "log.h"

#define TFTP_OCTET_MODE "octet" /* non-case-sensitive */

#define TFTP_OP_RRQ 1
#define TFTP_OP_WRQ 2
#define TFTP_OP_DATA 3
#define TFTP_OP_ACK 4
#define TFTP_OP_ERROR 5
#define TFTP_OP_OACK 6

#define TFTP_ERR_FILENOTFOUND 1
#define TFTP_ERR_FULL 3
#define TFTP_ERR_ILLEGAL 4
#define TFTP_ERR_BADID 5

#define TFTP_TIMEOUT_RETRANSMITS 10

err_t tftp_receive_data(struct netconn *nc, size_t write_offs,
                        size_t limit_offs, size_t *received_len,
                        ip_addr_t *peer_addr, int peer_port,
                        tftp_receive_cb receive_cb)
{
  *received_len = 0;
  const int DATA_PACKET_SZ = 512 + 4; /*( packet size plus header */
  uint32_t start_offs = write_offs;
  int block = 1;

  struct netbuf *netbuf = 0;
  int retries = TFTP_TIMEOUT_RETRANSMITS;

  while (1)
  {
    if (peer_addr)
    {
      netconn_disconnect(nc);
    }

    err_t err = netconn_recv(nc, &netbuf);

    if (peer_addr)
    {
      if (netbuf)
      {
        /* For TFTP server, the UDP connection is already established. But for client,
         we don't know what port the server is using until we see the first data
         packet - so we connect here.
         */
        netconn_connect(nc, netbuf_fromaddr(netbuf), netbuf_fromport(netbuf));
        peer_addr = 0;
      }
      else
      {
        /* Otherwise, temporarily re-connect so we can send errors */
        netconn_connect(nc, peer_addr, peer_port);
      }
    }

    if (err == ERR_TIMEOUT)
    {
      if (retries-- > 0 && block > 1)
      {
        /* Retransmit the last ACK, wait for repeat data block.

         This doesn't work for the first block, have to time out and start again. */
        tftp_send_ack(nc, block - 1);
        continue;
      }
      tftp_send_error(nc, TFTP_ERR_ILLEGAL, "Timeout");
      return ERR_TIMEOUT;
    }
    else if (err != ERR_OK)
    {
      tftp_send_error(nc, TFTP_ERR_ILLEGAL, "Failed to receive packet");
      return err;
    }

    uint16_t opcode = netbuf_read_u16_n(netbuf, 0);
    if (opcode != TFTP_OP_DATA)
    {
      tftp_send_error(nc, TFTP_ERR_ILLEGAL, "Unknown opcode");
      netbuf_delete(netbuf);
      return ERR_VAL;
    }

    uint16_t client_block = netbuf_read_u16_n(netbuf, 2);
    if (client_block != block)
    {
      netbuf_delete(netbuf);
      if (client_block == block - 1)
      {
        /* duplicate block, means our ack got lost */
        tftp_send_ack(nc, block - 1);
        continue;
      }
      else
      {
        tftp_send_error(nc, TFTP_ERR_ILLEGAL, "Block# out of order");
        return ERR_VAL;
      }
    }

    /* Reset retry count if we got valid data */
    retries = TFTP_TIMEOUT_RETRANSMITS;

    if (write_offs % SECTOR_SIZE == 0)
    {
      sdk_spi_flash_erase_sector(write_offs / SECTOR_SIZE);
    }

    /* One UDP packet can be more than one netbuf segment, so iterate all the
     segments in the netbuf and write them to flash
     */
    int offset = 0;
    int len = netbuf_len(netbuf);

    if (write_offs + len >= limit_offs)
    {
      tftp_send_error(nc, TFTP_ERR_FULL, "Image too large");
      return ERR_VAL;
    }

    bool first_chunk = true;
    do
    {
      uint16_t chunk_len;
      uint32_t *chunk;
      netbuf_data(netbuf, (void **)&chunk, &chunk_len);
      if (first_chunk)
      {
        chunk++; /* skip the 4 byte TFTP header */
        chunk_len -= 4; /* assuming this netbuf chunk is at least 4 bytes! */
        first_chunk = false;
      }
      if (chunk_len && ((uint32_t)chunk % 4))
      {
        /* sdk_spi_flash_write requires a word aligned
         buffer, so if the UDP payload is unaligned
         (common) then we copy the first word to the stack
         and write that to flash, then move the rest of the
         buffer internally to sit on an aligned offset.

         Assuming chunk_len is always a multiple of 4 bytes.
         */
        uint32_t first_word;
        memcpy(&first_word, chunk, 4);
        sdk_spi_flash_write(write_offs + offset, &first_word, 4);
        memmove(LWIP_MEM_ALIGN(chunk), &chunk[1], chunk_len - 4);
        chunk = LWIP_MEM_ALIGN(chunk);
        offset += 4;
        chunk_len -= 4;
      }
      sdk_spi_flash_write(write_offs + offset, chunk, chunk_len);
      offset += chunk_len;
    } while (netbuf_next(netbuf) >= 0);

    netbuf_delete(netbuf);

    *received_len += len - 4;

    if (len < DATA_PACKET_SZ)
    {
      /* This was the last block, but verify the image before we ACK
       it so the client gets an indication if things were successful.
       */
      const char *err = "Unknown validation error";
      uint32_t image_length;
      if (!rboot_verify_image(start_offs, &image_length, &err) || image_length
          != *received_len)
      {
        tftp_send_error(nc, TFTP_ERR_ILLEGAL, err);
        return ERR_VAL;
      }
    }

    err_t ack_err = tftp_send_ack(nc, block);
    if (ack_err != ERR_OK)
    {
      LOG_PRINTF("OTA TFTP failed to send ACK.");
      return ack_err;
    }

    // Make sure ack was successful before calling callback.
    if (receive_cb)
    {
      receive_cb(*received_len);
    }

    if (len < DATA_PACKET_SZ)
    {
      return ERR_OK;
    }

    block++;
    write_offs += 512;
  }
}

err_t tftp_send_ack(struct netconn *nc, int block)
{
  /* Send ACK */
  struct netbuf *resp = netbuf_new();
  uint16_t *ack_buf = (uint16_t *)netbuf_alloc(resp, 4);
  ack_buf[0] = htons(TFTP_OP_ACK);
  ack_buf[1] = htons(block);
  err_t ack_err = netconn_send(nc, resp);
  netbuf_delete(resp);
  return ack_err;
}

void tftp_send_error(struct netconn *nc, int err_code, const char *err_msg)
{
  LOG_PRINTF("OTA TFTP Error: %s", err_msg);
  struct netbuf *err = netbuf_new();
  uint16_t *err_buf = (uint16_t *)netbuf_alloc(err, 4 + strlen(err_msg) + 1);
  err_buf[0] = htons(TFTP_OP_ERROR);
  err_buf[1] = htons(err_code);
  strcpy((char *)&err_buf[2], err_msg);
  netconn_send(nc, err);
  netbuf_delete(err);
}

err_t tftp_send_rrq(struct netconn *nc, const char *filename)
{
  struct netbuf *rrqbuf = netbuf_new();
  uint16_t *rrqdata = (uint16_t *)netbuf_alloc(
      rrqbuf, 4 + strlen(filename) + strlen(TFTP_OCTET_MODE));
  rrqdata[0] = htons(TFTP_OP_RRQ);
  char *rrq_filename = (char *)&rrqdata[1];
  strcpy(rrq_filename, filename);
  strcpy(rrq_filename + strlen(filename) + 1, TFTP_OCTET_MODE);

  err_t err = netconn_send(nc, rrqbuf);
  netbuf_delete(rrqbuf);
  return err;
}
