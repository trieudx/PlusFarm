#include "esp_common.h"
#include "FreeRTOS.h"
#include "task.h"
#include "log.h"
#include "gpio.h"
#include "bh1750.h"
#include "sht1x.h"
#include "semphr.h"
#include "MQTTESP8266.h"
#include "MQTTClient.h"
#include "user_config.h"

#define PUB_MSG_LEN 16

xQueueHandle publish_queue;

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 user_rf_cal_sector_set(void)
{
	flash_size_map size_map = system_get_flash_size_map();
	uint32 rf_cal_sec = 0;

	switch (size_map)
	{
		case FLASH_SIZE_4M_MAP_256_256:
			rf_cal_sec = 128 - 5;
			break;
		case FLASH_SIZE_8M_MAP_512_512:
			rf_cal_sec = 256 - 5;
			break;
		case FLASH_SIZE_16M_MAP_512_512:
		case FLASH_SIZE_16M_MAP_1024_1024:
			rf_cal_sec = 512 - 5;
			break;
		case FLASH_SIZE_32M_MAP_512_512:
		case FLASH_SIZE_32M_MAP_1024_1024:
			rf_cal_sec = 1024 - 5;
			break;
		default:
			rf_cal_sec = 0;
			break;
	}

	return rf_cal_sec;
}

static void task_gpio(void *param)
{
	GPIO_Config gpio_config;
	gpio_config.pin = GPIO_PIN_15;
	gpio_config.mode = GPIO_MODE_OUT_PP;
	gpio_config.pull = GPIO_NO_PULL;
	GPIO_Init(&gpio_config);

	while (1)
	{
		GPIO_Toggle(gpio_config.pin);
		LOG_PRINTF("GPIO%d toggled", gpio_config.pin);

		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

static void task_bh1750(void *param)
{
	uint16 data;

	BH1750_Init();

	while (1)
	{
		if (BH1750_ReadAmbientLight(BH1750_0P5LX_RES_120MS_MT, &data)
			== I2CM_OK)
		{
			LOG_PRINTF("Current ambient light from BH1750: 0x%04X", data);
		}
		else
			LOG_PRINTF("Timeout when reading BH1750");

		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

static void task_sht1x(void *param)
{
	uint16 data;

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

		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

static const char *mqtt_get_id(void)
{
	/* Use MAC address for Station as unique ID */
	static char my_id[13];
	static bool my_id_done = false;
	int8_t i;
	uint8_t x;

	if (my_id_done)
		return my_id;

	if (!wifi_get_macaddr(STATION_IF, my_id))
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
static void task_beat(void * pvParameters)
{
	portTickType xLastWakeTime = xTaskGetTickCount();
	char msg[PUB_MSG_LEN];
	int count = 0;

	while (1)
	{
		vTaskDelayUntil(&xLastWakeTime, 5000 / portTICK_RATE_MS);
		LOG_PRINTF("%s", __FUNCTION__);
		snprintf(msg, PUB_MSG_LEN, "Beat %d", ++count);
		if (xQueueSend(publish_queue, (void *)msg, 1) == pdFALSE)
		{
			LOG_PRINTF("Publish queue overflow.");
		}
	}
}


// Callback when receiving subscribed message
static void topic_received(MQTTMessageData* md)
{
	char topic[30];
	char msg[50];

	memcpy(topic, md->topic->lenstring.data, md->topic->lenstring.len);
	topic[md->topic->lenstring.len] = '\0';
	memcpy(msg, md->message->payload, md->message->payloadlen);
	msg[md->message->payloadlen] = '\0';
	LOG_PRINTF("Topic Received: %s = %s", topic, msg);
}


static void task_mqtt(void *param)
{
	struct station_config wifi_config;
	Network network;
	MQTTClient client = DefaultClient;
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	unsigned char mqtt_buf[100];
	unsigned char mqtt_readbuf[100];
	char mqtt_client_id[30];
	int ret;
	uint8_t wifi_status;
	uint8_t timeout;

	/* Configure WiFi */
	strcpy(wifi_config.ssid, STA_SSID);
	strcpy(wifi_config.password, STA_PASSWORD);
	wifi_station_set_config(&wifi_config);

	NetworkInit(&network);

	/* Unique client ID */
	strcpy(mqtt_client_id, "PlusFarm-ESP-");
	strcat(mqtt_client_id, mqtt_get_id());

	while (1)
	{
		if (wifi_station_get_connect_status() != STATION_GOT_IP)
		{
			LOG_PRINTF("WiFi: (Re)connecting to %s", wifi_config.ssid);
			wifi_station_connect();
			wifi_status = wifi_station_get_connect_status();
			for (timeout = 30; (wifi_status != STATION_GOT_IP) && (timeout > 0);
					timeout--)
			{
				wifi_status = wifi_station_get_connect_status();
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

				vTaskDelay(1000 / portTICK_RATE_MS);
			}

			if (wifi_status == STATION_GOT_IP)
			{
				LOG_PRINTF("WiFi: Connected");
			}

			while (wifi_station_get_connect_status() == STATION_GOT_IP)
			{
				/* Connect to server */
				LOG_PRINTF("(Re)connecting to MQTT server %s ... ", MQTT_HOST);
				ret = NetworkConnect(&network, MQTT_HOST, MQTT_PORT);

				if (ret == SUCCESS)
				{
					LOG_PRINTF("(Re)connecting to MQTT server %s OK ", MQTT_HOST);

					/* Create new MQTT client */
					NewMQTTClient(&client, &network,
									5000, mqtt_buf, 100, mqtt_readbuf, 100);
					data.willFlag = 0;
					data.MQTTVersion = 3;
					data.clientID.cstring = mqtt_client_id;
					data.username.cstring = MQTT_USER;
					data.password.cstring = MQTT_PASS;
					data.keepAliveInterval = 10;
					data.cleansession = 0;
					/* Connect to MQTT service */
					LOG_PRINTF("Send MQTT connect ...");
					ret = MQTTConnect(&client, &data);

					if (ret == SUCCESS)
					{
						LOG_PRINTF("Send MQTT connect OK");

						/* Subscriptions */
						ret = MQTTSubscribe(&client, "temperature",
											QOS1, topic_received);

						if (ret == SUCCESS)
						{
							LOG_PRINTF("Subscription OK");

							while (1)
							{
								/* Publish all pending messages */
								char msg[PUB_MSG_LEN];
								while (xQueueReceive(publish_queue, (void *)msg, 1) == pdTRUE)
								{
									msg[PUB_MSG_LEN - 1] = '\0';
									MQTTMessage message;
									message.payload = msg;
									message.payloadlen = PUB_MSG_LEN;
									message.dup = 0;
									message.qos = QOS1;
									message.retained = 0;
									ret = MQTTPublish(&client, "beat", &message);
									if (ret != SUCCESS)
										break;
								}
								/* Receiving / Ping */
								ret = MQTTYield(&client, 1000);
								if (ret == DISCONNECTED)
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

					NetworkDisconnect(&network);
				}
				else
				{
					LOG_PRINTF("(Re)connecting to MQTT server %s failed ", MQTT_HOST);
				}

				vTaskDelay(1000 / portTICK_RATE_MS);
			}

			LOG_PRINTF("WiFi: Disconnected");
			wifi_station_disconnect();
		}

		vTaskDelay(1000 / portTICK_RATE_MS);
	}
}

void mqtt_init(void)
{
	publish_queue = xQueueCreate(3, PUB_MSG_LEN);

	if (wifi_get_opmode() != STATION_MODE)
	{
		LOG_PRINTF("Setting wifi station mode");
		wifi_set_opmode(STATION_MODE);
		LOG_PRINTF("Restarting system");
		system_restart();
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

//	/* Test GPIO */
//	xTaskCreate(task_gpio, "task_gpio", 256, NULL, 5, NULL);
//	/* Test BH1750 */
//	xTaskCreate(task_bh1750, "task_bh1750", 256, NULL, 5, NULL);
//	/* Test SHT1X */
//	xTaskCreate(task_sht1x, "task_sht1x", 256, NULL, 5, NULL);
}

