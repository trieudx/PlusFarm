/* A very simple OTA example
 *
 * Tries to run both a TFTP client and a TFTP server simultaneously, either will accept a TTP firmware and update it.
 *
 * Not a realistic OTA setup, this needs adapting (choose either client or server) before you'd want to use it.
 *
 * For more information about esp-open-rtos OTA see https://github.com/SuperHouse/esp-open-rtos/wiki/OTA-Update-Configuration
 *
 * NOT SUITABLE TO PUT ON THE INTERNET OR INTO A PRODUCTION ENVIRONMENT!!!!
 */
#include <string.h>
#include "sdk/esp_common.h"
#include "hal_uart.h"
#include "freertos.h"
#include "freertos_task.h"
#include "app_config.h"
#include "mbedtls/mbedtls_sha256.h"

#include "tftp.h"
#include "rboot-api.h"
#include "fota.h"
#include "log.h"

#include "lwip/lwip_err.h"
#include "lwip/lwip_api.h"
#include "lwip/lwip_sys.h"
#include "lwip/lwip_netdb.h"
#include "lwip/lwip_dns.h"
#include "lwip/lwip_mem.h"
#include <lwip_netbuf_helpers.h>
#include <sdk/spi_flash.h>
#include <sdk/esp_system.h>

#define MAX_IMAGE_SIZE 0x100000 /*1MB images max at the moment */

void fota_task(void *param)
{
  LOG_PRINTF("TFTP client task starting...");
  rboot_config conf;
  conf = rboot_get_config();
  int slot = (conf.current_rom + 1) % conf.count;
  LOG_PRINTF("Image will be saved in OTA slot %d.", slot);
  if (slot == conf.current_rom)
  {
    LOG_PRINTF("FATAL ERROR: Only one OTA slot is configured!");
    while (1)
    {
    }
  }

  while (1)
  {
    LOG_PRINTF("Downloading %s to slot %d...", TFTP_FW_PATH, slot);
    int res = fota_download(TFTP_SERVER, TFTP_PORT, TFTP_FW_PATH, 1000,
                                slot, NULL);
    LOG_PRINTF("ota_tftp_download %s result %d", TFTP_FW_PATH, res);

    if (res != 0)
    {
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      continue;
    }

    LOG_PRINTF("Looks valid, calculating SHA256...");
    uint32_t length;
    bool valid = rboot_verify_image(conf.roms[slot], &length, NULL);
    static mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    valid = valid
        && rboot_digest_image(conf.roms[slot], length,
                              (rboot_digest_update_fn)mbedtls_sha256_update,
                              &ctx);
    static uint8_t hash_result[32];
    mbedtls_sha256_finish(&ctx, hash_result);
    mbedtls_sha256_free(&ctx);

    if (!valid)
    {
      LOG_PRINTF("Not valid after all :(");
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      continue;
    }

    printf("Image SHA256 = ");
    valid = true;
    for (int i = 0; i < sizeof(hash_result); i++)
    {
      char hexbuf[3];
      snprintf(hexbuf, 3, "%02x", hash_result[i]);
      printf(hexbuf);
      if (strncmp(hexbuf, FW_SHA256 + i * 2, 2))
        valid = false;
    }
    printf("\n");

    if (!valid)
    {
      LOG_PRINTF("Downloaded image SHA256 didn't match expected '%s'", FW_SHA256);
      vTaskDelay(5000 / portTICK_PERIOD_MS);
      continue;
    }

    LOG_PRINTF("SHA256 Matches. Rebooting into slot %d...", slot);
    vPortEnterCritical();
    if (!rboot_set_current_rom(slot))
    {
      LOG_PRINTF("OTA TFTP failed to set new rboot slot");
    }
    sdk_system_restart();
  }
}

err_t fota_download(const char *server, int port, const char *filename,
                    int timeout, int ota_slot, tftp_receive_cb receive_cb)
{
  rboot_config rboot_config = rboot_get_config();
  /* Validate the OTA slot parameter */
  if (rboot_config.current_rom == ota_slot || rboot_config.count <= ota_slot)
  {
    return ERR_VAL;
  }
  /* This is all we need to know from the rboot config - where we need
   to write data to.
   */
  uint32_t flash_offset = rboot_config.roms[ota_slot];

  struct netconn *nc = netconn_new(NETCONN_UDP);
  err_t err;
  if (!nc)
  {
    return ERR_IF;
  }

  netconn_set_recvtimeout(nc, timeout);

  /* try to bind our client port as our local port,
   or keep trying the next 10 ports after it */
  int local_port = port - 1;
  do
  {
    err = netconn_bind(nc, IP_ADDR_ANY, ++local_port);
  } while (err == ERR_USE && local_port < port + 10);
  if (err)
  {
    netconn_delete(nc);
    return err;
  }

  ip_addr_t addr;
  err = netconn_gethostbyname(server, &addr);
  if (err)
  {
    netconn_delete(nc);
    return err;
  }

  netconn_connect(nc, &addr, port);

  err = tftp_send_rrq(nc, filename);
  if (err)
  {
    netconn_delete(nc);
    return err;
  }

  size_t received_len;
  err = tftp_receive_data(nc, flash_offset, flash_offset + MAX_IMAGE_SIZE,
                          &received_len, &addr, port, receive_cb);
  netconn_delete(nc);
  return err;
}


