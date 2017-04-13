#include "sdk/esp_common.h"
#include "freertos.h"
#include "freertos_task.h"
#include "log.h"
#include "hal_gpio.h"
#include "bh1750.h"
#include "sht1x.h"
#include "freertos_semphr.h"
#include "mqtt_port.h"
#include "mqtt_client.h"
#include "app_config.h"
#include "string.h"

#define PUB_MSG_LEN 16

QueueHandle_t publish_queue;

void task_gpio(void *param)
{
  HAL_GPIO_ConfigType gpio_config;
  gpio_config.pin = HAL_GPIO_PIN_15;
  gpio_config.mode = HAL_GPIO_MODE_OUT_PP;
  gpio_config.pull = HAL_GPIO_NO_PULL;
  gpio_config.sleepable = false;
  HAL_GPIO_Init(&gpio_config);

  while (1)
  {
    HAL_GPIO_Toggle(gpio_config.pin);
    LOG_PRINTF("GPIO%d toggled", gpio_config.pin);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void task_bh1750(void *param)
{
  uint16_t data;

  BH1750_Init();

  while (1)
  {
    if (BH1750_ReadAmbientLight(BH1750_0P5LX_RES_120MS_MT, &data) == I2CM_OK)
    {
      LOG_PRINTF("Current ambient light from BH1750: 0x%04X", data);
    }
    else
      LOG_PRINTF("Timeout when reading BH1750");

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void task_sht1x(void *param)
{
  uint16_t data;

  SHT1X_Init();

  while (1)
  {
    if (SHT1X_ReadTemperature(&data) == I2CM_OK)
      LOG_PRINTF("Current temperature: 0x%04X", data);
    else
      LOG_PRINTF("Timeout when reading SHT1x");

    if (SHT1X_ReadRelativeHumidity(&data) == I2CM_OK)
      LOG_PRINTF("Current RH: 0x%04X", data);
    else
      LOG_PRINTF("Timeout when reading SHT1x");

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

const char *mqtt_get_id(void)
{
  /* Use MAC address for Station as unique ID */
  static char my_id[13];
  static bool my_id_done = false;
  int8_t i;
  uint8_t x;

  if (my_id_done)
    return my_id;

  if (!sdk_wifi_get_macaddr(STATION_IF, my_id))
    return NULL;

  for (i = 5; i >= 0; i--)
  {
    x = my_id[i] & 0x0F;
    if (x > 9)
      x += 7;
    my_id[i * 2 + 1] = x + '0';
    x = my_id[i] >> 4;
    if (x > 9)
      x += 7;
    my_id[i * 2] = x + '0';
  }
  my_id[12] = '\0';
  my_id_done = true;

  return my_id;
}

/* Demonstrating sending something to MQTT broker
 In this task we simply queue up messages in publish_queue. The MQTT task will dequeue the
 message and sent.
 */
void task_beat(void * pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  char msg[PUB_MSG_LEN];
  int count = 0;

  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, 5000 / portTICK_PERIOD_MS);
    LOG_PRINTF("%s", __FUNCTION__);
    snprintf(msg, PUB_MSG_LEN, "Beat %d", ++count);
    if (xQueueSend(publish_queue, (void *)msg, 1) == pdFALSE)
    {
      LOG_PRINTF("Publish queue overflow.");
    }
  }
}

// Callback when receiving subscribed message
void topic_received(mqtt_message_data_t* md)
{
  char topic[30];
  char msg[50];

  memcpy(topic, md->topic->lenstring.data, md->topic->lenstring.len);
  topic[md->topic->lenstring.len] = '\0';
  memcpy(msg, md->message->payload, md->message->payloadlen);
  msg[md->message->payloadlen] = '\0';
  LOG_PRINTF("Topic Received: %s = %s", topic, msg);
}

void task_mqtt(void *param)
{
  struct sdk_station_config wifi_config;
  mqtt_network_t network;
  mqtt_client_t client = mqtt_client_default;
  mqtt_packet_connect_data_t data = mqtt_packet_connect_data_initializer;
  unsigned char mqtt_buf[100];
  unsigned char mqtt_readbuf[100];
  char mqtt_client_id[30];
  int ret;
  uint8_t wifi_status;
  uint8_t timeout;

  /* Configure WiFi */
  strcpy(wifi_config.ssid, STA_SSID);
  strcpy(wifi_config.password, STA_PASS);
  sdk_wifi_station_set_config(&wifi_config);

  mqtt_network_new(&network);

  /* Unique client ID */
  strcpy(mqtt_client_id, "PlusFarm-ESP-");
  strcat(mqtt_client_id, mqtt_get_id());

  while (1)
  {
    if (sdk_wifi_station_get_connect_status() != STATION_GOT_IP)
    {
      LOG_PRINTF("WiFi: (Re)connecting to %s", wifi_config.ssid);
      sdk_wifi_station_connect();
      wifi_status = sdk_wifi_station_get_connect_status();
      for (timeout = 30; (wifi_status != STATION_GOT_IP) && (timeout > 0);
          timeout--)
      {
        wifi_status = sdk_wifi_station_get_connect_status();
        if (wifi_status == STATION_WRONG_PASSWORD)
        {
          LOG_PRINTF("WiFi: Wrong password");
          break;
        }
        else if (wifi_status == STATION_NO_AP_FOUND)
        {
          LOG_PRINTF("WiFi: AP not found");
          break;
        }
        else if (wifi_status == STATION_CONNECT_FAIL)
        {
          LOG_PRINTF("WiFi: Connection failed");
          break;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }

      if (wifi_status == STATION_GOT_IP)
      {
        LOG_PRINTF("WiFi: Connected");
      }

      while (sdk_wifi_station_get_connect_status() == STATION_GOT_IP)
      {
        /* Connect to server */
        LOG_PRINTF("(Re)connecting to MQTT server %s ... ", MQTT_HOST);
        ret = mqtt_network_connect(&network, MQTT_HOST, MQTT_PORT);

        if (ret == MQTT_SUCCESS)
        {
          LOG_PRINTF("(Re)connecting to MQTT server %s OK ", MQTT_HOST);

          /* Create new MQTT client */
          mqtt_client_new(&client, &network, 5000, mqtt_buf, 100, mqtt_readbuf,
                        100);
          data.willFlag = 0;
          data.MQTTVersion = 3;
          data.clientID.cstring = mqtt_client_id;
          data.username.cstring = MQTT_USER;
          data.password.cstring = MQTT_PASS;
          data.keepAliveInterval = 10;
          data.cleansession = 0;
          /* Connect to MQTT service */
          LOG_PRINTF("Send MQTT connect ...");
          ret = mqtt_connect(&client, &data);

          if (ret == MQTT_SUCCESS)
          {
            LOG_PRINTF("Send MQTT connect OK");

            /* Subscriptions */
            ret = mqtt_subscribe(&client, "temperature", MQTT_QOS1,
                                 topic_received);

            if (ret == MQTT_SUCCESS)
            {
              LOG_PRINTF("Subscription OK");

              while (1)
              {
                /* Publish all pending messages */
                char msg[PUB_MSG_LEN];
                while (xQueueReceive(publish_queue, (void *)msg, 1) == pdTRUE)
                {
                  msg[PUB_MSG_LEN - 1] = '\0';
                  mqtt_message_t message;
                  message.payload = msg;
                  message.payloadlen = PUB_MSG_LEN;
                  message.dup = 0;
                  message.qos = MQTT_QOS1;
                  message.retained = 0;
                  ret = mqtt_publish(&client, "beat", &message);
                  if (ret != MQTT_SUCCESS)
                    break;
                }
                /* Receiving / Ping */
                ret = mqtt_yield(&client, 1000);
                if (ret == MQTT_DISCONNECTED)
                {
                  LOG_PRINTF("Connection broken, request restart");
                  break;
                }
              }
            }
            else
            {
              LOG_PRINTF("Subscription failed");
            }
          }
          else
          {
            LOG_PRINTF("Send MQTT connect failed");
          }

          mqtt_network_disconnect(&network);
        }
        else
        {
          LOG_PRINTF("(Re)connecting to MQTT server %s failed ", MQTT_HOST);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
      }

      LOG_PRINTF("WiFi: Disconnected");
      sdk_wifi_station_disconnect();
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void mqtt_init(void)
{
  publish_queue = xQueueCreate(3, PUB_MSG_LEN);

  if (sdk_wifi_get_opmode() != STATION_MODE)
  {
    LOG_PRINTF("Setting wifi station mode");
    sdk_wifi_set_opmode(STATION_MODE);
    LOG_PRINTF("Restarting system");
    sdk_system_restart();
  }

  xTaskCreate(task_mqtt, "task_mqtt", 1152, NULL, tskIDLE_PRIORITY + 2, NULL);
  xTaskCreate(task_beat, "task_beat", 512, NULL, tskIDLE_PRIORITY + 3, NULL);
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
 *******************************************************************************/
void user_init(void)
{
  /* Initialize log */
  Log_Init();

  /* Initialize MQTT */
  mqtt_init();

  /* Test GPIO */
  xTaskCreate(task_gpio, "task_gpio", 256, NULL, 5, NULL);
  /* Test BH1750 */
  xTaskCreate(task_bh1750, "task_bh1750", 256, NULL, 5, NULL);
  /* Test SHT1X */
  xTaskCreate(task_sht1x, "task_sht1x", 256, NULL, 5, NULL);
}

///*
// * HTTP server example.
// *
// * This sample code is in the public domain.
// */
//#include <sdk/esp_common.h>
//#include <esp8266.h>
//#include <hal_uart.h>
//#include <string.h>
//#include <stdio.h>
//#include <freertos.h>
//#include <freertos_task.h>
//#include <app_config.h>
//#include <httpd.h>
//#include <dhcpserver.h>
//
//#define LED_PIN 2
//
//enum
//{
//  SSI_UPTIME,
//  SSI_FREE_HEAP,
//  SSI_LED_STATE
//};
//
//int32_t ssi_handler(int32_t iIndex, char *pcInsert, int32_t iInsertLen)
//{
//  switch (iIndex)
//  {
//    case SSI_UPTIME:
//      snprintf(pcInsert, iInsertLen, "%d",
//               xTaskGetTickCount() * portTICK_PERIOD_MS / 1000);
//      break;
//    case SSI_FREE_HEAP:
//      snprintf(pcInsert, iInsertLen, "%d", (int)xPortGetFreeHeapSize());
//      break;
//    case SSI_LED_STATE:
//      snprintf(pcInsert, iInsertLen, (GPIO.OUT & BIT(LED_PIN)) ? "Off" : "On");
//      break;
//    default:
//      snprintf(pcInsert, iInsertLen, "N/A");
//      break;
//  }
//
//  /* Tell the server how many characters to insert */
//  return (strlen(pcInsert));
//}
//
//char *gpio_cgi_handler(int iIndex, int iNumParams, char *pcParam[],
//                       char *pcValue[])
//{
//  HAL_GPIO_ConfigType gpio_config =
//  {
//    .mode = HAL_GPIO_MODE_OUT_PP,
//    .pull = HAL_GPIO_NO_PULL,
//    .sleepable = false
//  };
//
//  for (int i = 0; i < iNumParams; i++)
//  {
//    gpio_config.pin = atoi(pcValue[i]);
//    HAL_GPIO_Init(&gpio_config);
//
//    if (strcmp(pcParam[i], "on") == 0)
//    {
//      HAL_GPIO_SetHigh(gpio_config.pin);
//    }
//    else if (strcmp(pcParam[i], "off") == 0)
//    {
//      HAL_GPIO_SetLow(gpio_config.pin);
//    }
//    else if (strcmp(pcParam[i], "toggle") == 0)
//    {
//      HAL_GPIO_Toggle(gpio_config.pin);
//    }
//  }
//  return "/index.ssi";
//}
//
//char *about_cgi_handler(int iIndex, int iNumParams, char *pcParam[],
//                        char *pcValue[])
//{
//  return "/about.html";
//}
//
//char *websocket_cgi_handler(int iIndex, int iNumParams, char *pcParam[],
//                            char *pcValue[])
//{
//  return "/websockets.html";
//}
//
//void websocket_task(void *pvParameter)
//{
//  struct tcp_pcb *pcb = (struct tcp_pcb *)pvParameter;
//
//  for (;;)
//  {
//    if (pcb == NULL || pcb->state != ESTABLISHED)
//    {
//      printf("Connection closed, deleting task\n");
//      break;
//    }
//
//    int uptime = xTaskGetTickCount() * portTICK_PERIOD_MS / 1000;
//    int heap = (int)xPortGetFreeHeapSize();
//    int led = !HAL_GPIO_Read(LED_PIN);
//
//    /* Generate response in JSON format */
//    char response[64];
//    int len = snprintf(response, sizeof(response), "{\"uptime\" : \"%d\","
//                       " \"heap\" : \"%d\","
//                       " \"led\" : \"%d\"}",
//                       uptime, heap, led);
//    if (len < sizeof(response))
//      websocket_write(pcb, (unsigned char *)response, len, WS_TEXT_MODE);
//
//    vTaskDelay(2000 / portTICK_PERIOD_MS);
//  }
//
//  vTaskDelete(NULL);
//}
//
///**
// * This function is called when websocket frame is received.
// *
// * Note: this function is executed on TCP thread and should return as soon
// * as possible.
// */
//void websocket_cb(struct tcp_pcb *pcb, uint8_t *data, u16_t data_len,
//                  uint8_t mode)
//{
//  printf("[websocket_callback]:\n%.*s\n", (int)data_len, (char*)data);
//
//  uint8_t response[2];
//  uint16_t val;
//
//  switch (data[0])
//  {
//    case 'A': // ADC
//      /* This should be done on a separate thread in 'real' applications */
//      val = sdk_system_adc_read();
//      break;
//    case 'D': // Disable LED
//      HAL_GPIO_SetHigh(LED_PIN);
//      val = 0xDEAD;
//      break;
//    case 'E': // Enable LED
//      HAL_GPIO_SetLow(LED_PIN);
//      val = 0xBEEF;
//      break;
//    default:
//      printf("Unknown command\n");
//      val = 0;
//      break;
//  }
//
//  response[1] = (uint8_t)val;
//  response[0] = val >> 8;
//
//  websocket_write(pcb, response, 2, WS_BIN_MODE);
//}
//
///**
// * This function is called when new websocket is open and
// * creates a new websocket_task if requested URI equals '/stream'.
// */
//void websocket_open_cb(struct tcp_pcb *pcb, const char *uri)
//{
//  printf("WS URI: %s\n", uri);
//  if (!strcmp(uri, "/stream"))
//  {
//    printf("request for streaming\n");
//    xTaskCreate(&websocket_task, "websocket_task", 256, (void *)pcb, 2, NULL);
//  }
//}
//
//void httpd_task(void *pvParameters)
//{
//  tCGI pCGIs[] = { { "/gpio", (tCGIHandler)gpio_cgi_handler }, {
//      "/about", (tCGIHandler)about_cgi_handler },
//                   { "/websockets", (tCGIHandler)websocket_cgi_handler }, };
//
//  const char *pcConfigSSITags[] = { "uptime", // SSI_UPTIME
//      "heap",   // SSI_FREE_HEAP
//      "led"     // SSI_LED_STATE
//      };
//
//  /* register handlers and start the server */
//  http_set_cgi_handlers(pCGIs, sizeof(pCGIs) / sizeof(pCGIs[0]));
//  http_set_ssi_handler((tSSIHandler)ssi_handler, pcConfigSSITags,
//                       sizeof(pcConfigSSITags) / sizeof(pcConfigSSITags[0]));
//  websocket_register_callbacks((tWsOpenHandler)websocket_open_cb,
//                               (tWsHandler)websocket_cb);
//  httpd_init();
//
//  while (1)
//  {
//    vTaskDelay(1000 / portTICK_PERIOD_MS);
//  }
//}
//
//void user_init(void)
//{
//  uart_set_baud(0, 115200);
//  printf("SDK version:%s\n", sdk_system_get_sdk_version());
//
//  struct sdk_station_config sta_config =
//  {
//    .ssid = STA_SSID,
//    .password = STA_PASS
//  };
//
//  /* required to call wifi_set_opmode before station_set_config */
//  sdk_wifi_set_opmode(STATIONAP_MODE);
//
//  struct ip_info ap_ip;
//  IP4_ADDR(&ap_ip.ip, 172, 16, 0, 1);
//  IP4_ADDR(&ap_ip.gw, 0, 0, 0, 0);
//  IP4_ADDR(&ap_ip.netmask, 255, 255, 0, 0);
//  sdk_wifi_set_ip_info(1, &ap_ip);
//
//  struct sdk_softap_config ap_config =
//  {
//    .ssid = AP_SSID,
//    .ssid_hidden = 0,
//    .channel = 3,
//    .ssid_len = strlen(AP_SSID),
//    .authmode = AUTH_WPA_WPA2_PSK,
//    .password = AP_PASS,
//    .max_connection = 3,
//    .beacon_interval = 100,
//  };
//  sdk_wifi_softap_set_config(&ap_config);
//  sdk_wifi_station_set_config(&sta_config);
//  sdk_wifi_station_connect();
//
//  ip_addr_t first_client_ip;
//  IP4_ADDR(&first_client_ip, 172, 16, 0, 2);
//  dhcpserver_start(&first_client_ip, 4);
//
//  /* turn off LED */
//  HAL_GPIO_ConfigType led_config =
//  {
//    .mode = HAL_GPIO_MODE_OUT_PP,
//    .pull = HAL_GPIO_NO_PULL,
//    .sleepable = false
//  };
//
//  led_config.pin = LED_PIN;
//  HAL_GPIO_Init(&led_config);
//  HAL_GPIO_SetHigh(led_config.pin);
//
//  /* initialize tasks */
//  xTaskCreate(&httpd_task, "HTTP Daemon", 128, NULL, 2, NULL);
//}
