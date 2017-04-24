/* Inclusion section ======================================================== */
#include "sdk/esp_common.h"
#include "i2cm.h"
#include "hal_gpio.h"
#include "freertos.h"
#include "freertos_semphr.h"
#include "freertos_task.h"

/* Private macro definition section ========================================= */
#define I2CM_WRITE                      0x00
#define I2CM_READ                       0x01

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */
static void i2cm_set_speed(I2CM_SpeedModeType speed_mode);
static void i2cm_start(I2CM_StartModeType start_mode);
static void i2cm_stop(void);
static I2CM_ReturnType i2cm_access(I2CM_SlaveConfigType *slave_config,
                                   uint16_t reg_addr);
static I2CM_ReturnType i2cm_tx_one_byte(uint8_t data, bool stop);
static void i2cm_rx_one_byte(uint8_t *data, bool stop);

/* Private variable section ================================================= */
static bool                     i2cm_initialized = false;
static uint8_t                  i2cm_delay_half_cycle;
static uint8_t                  i2cm_delay_quarter_cycle;
static SemaphoreHandle_t        i2cm_semaphore;

/* Public function definition section ======================================= */
void I2CM_Init(void)
{
  /* Do nothing if the I2C master hardware has already been initialized */
  if (i2cm_initialized == false)
  {
    HAL_GPIO_ConfigType gpio_scl, gpio_sda;

    /* Initialize SCL pin */
    gpio_scl.pin = I2CM_SCL_PIN;
    gpio_scl.mode = HAL_GPIO_MODE_OUT_OD;
    gpio_scl.pull = HAL_GPIO_PULL_UP;
    gpio_scl.sleepable = false;
    HAL_GPIO_Init(&gpio_scl);

    /* Initialize SDA pin */
    gpio_sda.pin = I2CM_SDA_PIN;
    gpio_sda.mode = HAL_GPIO_MODE_OUT_OD;
    gpio_sda.pull = HAL_GPIO_PULL_UP;
    gpio_sda.sleepable = false;
    HAL_GPIO_Init(&gpio_sda);

    /* Release I2C bus */
    HAL_GPIO_SetHigh(gpio_scl.pin);
    HAL_GPIO_SetHigh(gpio_sda.pin);

    /* Create semaphore to protect Tx/Rx in multiple tasks */
    i2cm_semaphore = xSemaphoreCreateMutex();

    /* Set initialization status */
    i2cm_initialized = true;
  }
}

I2CM_ReturnType I2CM_Transmit(I2CM_SlaveConfigType *slave_config,
                              uint8_t length, uint8_t *data)
{
  /* Wait for previous Tx/Rx to complete and take resource */
  if (xSemaphoreTake(i2cm_semaphore, I2CM_TIMEOUT_MS / portTICK_PERIOD_MS))
  {
    I2CM_ReturnType ret;
    uint8_t i;

    /* Set speed mode */
    i2cm_set_speed(slave_config->speed_mode);

    /* Send START condition */
    i2cm_start(slave_config->start_mode);

    /* Send slave address with WRITE action */
    ret = i2cm_tx_one_byte((slave_config->slave_addr << 1) | I2CM_WRITE,
    false);

    if (ret == I2CM_OK)
    {
      /* Wait for completion of slave access */
      vTaskDelay(slave_config->access_time / portTICK_PERIOD_MS);
      /* Transmit data to slave */
      for (i = 0; (ret == I2CM_OK) && (i < length); i++)
      {
        ret = i2cm_tx_one_byte(data[i], (i == (length - 1)));
      }
    }

    /* Release resource */
    xSemaphoreGive(i2cm_semaphore);

    return ret;
  }
  else
    return I2CM_TIMED_OUT;
}

I2CM_ReturnType I2CM_Receive(I2CM_SlaveConfigType *slave_config,
                             uint8_t length, uint8_t *data)
{
  /* Wait for previous Tx/Rx to complete and take resource */
  if (xSemaphoreTake(i2cm_semaphore, I2CM_TIMEOUT_MS / portTICK_PERIOD_MS))
  {
    I2CM_ReturnType ret;
    uint8_t i;

    /* Set speed mode */
    i2cm_set_speed(slave_config->speed_mode);

    /* Send START condition */
    i2cm_start(slave_config->start_mode);

    /* Send slave address with READ action */
    ret = i2cm_tx_one_byte((slave_config->slave_addr << 1) | I2CM_READ,
    false);

    if (ret == I2CM_OK)
    {
      /* Wait for completion of slave access */
      vTaskDelay(slave_config->access_time / portTICK_PERIOD_MS);
      /* Receive data from slave */
      for (i = 0; (ret == I2CM_OK) && (i < length); i++)
      {
        i2cm_rx_one_byte(data + i, (i == (length - 1)));
      }
    }

    /* Release resource */
    xSemaphoreGive(i2cm_semaphore);

    return ret;
  }
  else
    return I2CM_TIMED_OUT;
}

I2CM_ReturnType I2CM_Write(I2CM_SlaveConfigType *slave_config,
                           uint16_t reg_addr, uint16_t reg_access_time,
                           uint8_t *data)
{
  I2CM_ReturnType ret;

  /* Access to the register of slave */
  ret = i2cm_access(slave_config, reg_addr);

  if (ret == I2CM_OK)
  {
    /* Wait for completion of register access */
    vTaskDelay(reg_access_time / portTICK_PERIOD_MS);
    /* Write data to this register */
    ret = I2CM_Transmit(slave_config, slave_config->reg_size, data);
  }

  return ret;
}

I2CM_ReturnType I2CM_Read(I2CM_SlaveConfigType *slave_config,
                          uint16_t reg_addr, uint16_t reg_access_time,
                          uint8_t *data)
{
  I2CM_ReturnType ret;

  /* Access to the register of slave */
  ret = i2cm_access(slave_config, reg_addr);

  if (ret == I2CM_OK)
  {
    /* Wait for completion of register access */
    vTaskDelay(reg_access_time / portTICK_PERIOD_MS);
    /* Read data from this register */
    ret = I2CM_Receive(slave_config, slave_config->reg_size, data);
  }

  return ret;
}

/* Private function definition section ====================================== */
static void i2cm_set_speed(I2CM_SpeedModeType speed_mode)
{
  if (speed_mode == I2CM_SPEED_100KHz)
  {
    i2cm_delay_half_cycle = 4;
    i2cm_delay_quarter_cycle = 3;
  }
  else
  {
    i2cm_delay_half_cycle = 2;
    i2cm_delay_quarter_cycle = 1;
  }
}

static void i2cm_start(I2CM_StartModeType start_mode)
{
  if (start_mode == I2CM_START_MODE_I2C)
  {
    HAL_GPIO_SetLow(I2CM_SDA_PIN);
    sdk_os_delay_us(i2cm_delay_half_cycle);
    HAL_GPIO_SetLow(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
  }
  else
  {
    HAL_GPIO_SetLow(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
    HAL_GPIO_SetHigh(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
    HAL_GPIO_SetLow(I2CM_SDA_PIN);
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
    HAL_GPIO_SetLow(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_half_cycle);
    HAL_GPIO_SetHigh(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
    HAL_GPIO_SetHigh(I2CM_SDA_PIN);
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
    HAL_GPIO_SetLow(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
  }
}

static void i2cm_stop(void)
{
  HAL_GPIO_SetLow(I2CM_SDA_PIN);
  sdk_os_delay_us(i2cm_delay_quarter_cycle);
  HAL_GPIO_SetHigh(I2CM_SCL_PIN);
  sdk_os_delay_us(i2cm_delay_quarter_cycle);
  HAL_GPIO_SetHigh(I2CM_SDA_PIN);
  sdk_os_delay_us(i2cm_delay_half_cycle);
}

static I2CM_ReturnType i2cm_access(I2CM_SlaveConfigType *slave_config,
                                   uint16_t reg_addr)
{
  uint8_t reg[2];
  uint8_t reg_len = 0;

  if (slave_config->reg_addr_mode == I2CM_REG_ADDR_TWO_BYTES)
    reg[reg_len++] = reg_addr >> 8;
  reg[reg_len++] = (uint8_t)reg_addr;

  return I2CM_Transmit(slave_config, reg_len, reg);
}

static I2CM_ReturnType i2cm_tx_one_byte(uint8_t data, bool stop)
{
  int8_t i;
  bool ACK;

  /* Send one byte to slave */
  for (i = 7; i >= 0; i--)
  {
    HAL_GPIO_Set(I2CM_SDA_PIN, (data >> i) & BIT(0));
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
    HAL_GPIO_SetHigh(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_half_cycle);
    HAL_GPIO_SetLow(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
  }

  /* Read back ACK from slave */
  HAL_GPIO_SetHigh(I2CM_SDA_PIN);
  sdk_os_delay_us(i2cm_delay_quarter_cycle);
  HAL_GPIO_SetHigh(I2CM_SCL_PIN);
  sdk_os_delay_us(i2cm_delay_half_cycle);
  ACK = !HAL_GPIO_Read(I2CM_SDA_PIN);
  HAL_GPIO_SetLow(I2CM_SCL_PIN);
  sdk_os_delay_us(i2cm_delay_quarter_cycle);

  if (!ACK || stop)
  {
    /* Send STOP condition */
    i2cm_stop();
  }

  /* Return status */
  return ACK ? I2CM_OK : I2CM_NACK;
}

static void i2cm_rx_one_byte(uint8_t *data, bool stop)
{
  int8_t i;

  /* Read one byte from slave */
  *data = 0;
  for (i = 7; i >= 0; i--)
  {
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
    HAL_GPIO_SetHigh(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_half_cycle);
    *data |= HAL_GPIO_Read(I2CM_SDA_PIN) << i;
    HAL_GPIO_SetLow(I2CM_SCL_PIN);
    sdk_os_delay_us(i2cm_delay_quarter_cycle);
  }

  /* Send ACK/NACK to slave */
  if (!stop)
    HAL_GPIO_SetLow(I2CM_SDA_PIN);
  else
    HAL_GPIO_SetHigh(I2CM_SDA_PIN);
  sdk_os_delay_us(i2cm_delay_quarter_cycle);
  HAL_GPIO_SetHigh(I2CM_SCL_PIN);
  sdk_os_delay_us(i2cm_delay_half_cycle);
  HAL_GPIO_SetLow(I2CM_SCL_PIN);
  sdk_os_delay_us(i2cm_delay_quarter_cycle);
  HAL_GPIO_SetHigh(I2CM_SDA_PIN);

  if (stop)
  {
    /* Send STOP condition */
    i2cm_stop();
  }
}

/* ============================= End of file ================================ */
