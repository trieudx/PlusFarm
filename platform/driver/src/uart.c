/* Inclusion section ======================================================== */
#include "esp_common.h"
#include "uart.h"

/* Private macro definition section ========================================= */
#define UART_REG_BASE(i)				(0x60000000 + (i) * 0xf00)

#define UART_FIFO(i)					(UART_REG_BASE(i) + 0x0)

#define UART_STATUS(i)					(UART_REG_BASE(i) + 0x1C)
#define UART_TXFIFO_CNT					0x000000FF
#define UART_TXFIFO_CNT_S				16

#define UART_CONF0(i)					(UART_REG_BASE(i) + 0x20)
#define UART_TXFIFO_RST					BIT18
#define UART_RXFIFO_RST					BIT17
#define UART_TX_FLOW_EN					BIT15
#define UART_STOP_BIT_NUM_S				4
#define UART_BIT_NUM_S					2
#define UART_PARITY_EN					BIT1

#define UART_CONF1(i)					(UART_REG_BASE(i) + 0x24)
#define UART_RX_TOUT_EN					BIT31
#define UART_RX_TOUT_THRHD				0x0000007F
#define UART_RX_TOUT_THRHD_S			24
#define UART_RX_FLOW_EN					BIT23
#define UART_RX_FLOW_THRHD				0x0000007F
#define UART_RX_FLOW_THRHD_S			16
#define UART_TXFIFO_EMPTY_THRHD			0x0000007F
#define UART_TXFIFO_EMPTY_THRHD_S		8

/* Private type definition section ========================================== */

/* Private function prototype section ======================================= */
static void uart_tx_one_char(UART_PortNo port_no, char tx_char);
static void uart0_write_char(char c);
static void uart1_write_char(char c);

/* Public function definition section ======================================= */
void UART_Init(UART_Config *config)
{
	/* Wait Tx FIFO empty */
	while (READ_PERI_REG(UART_STATUS(config->port_no)) &
			(UART_TXFIFO_CNT << UART_TXFIFO_CNT_S));

	/* Select UART functionality */
	if (config->port_no == UART_PORT0)
	{
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
	}
	else
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);

	/* Set hardware flow control */
	if (config->hw_flow_ctrl & UART_HW_FLOW_CTRL_RTS)
	{
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_U0RTS);
		SET_PERI_REG_BITS(UART_CONF1(config->port_no), UART_RX_FLOW_THRHD, 120,
							UART_RX_FLOW_THRHD_S);
		SET_PERI_REG_MASK(UART_CONF1(config->port_no), UART_RX_FLOW_EN);
	}
	else
	{
		CLEAR_PERI_REG_MASK(UART_CONF1(config->port_no), UART_RX_FLOW_EN);
	}

	if (config->hw_flow_ctrl & UART_HW_FLOW_CTRL_CTS)
	{
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_UART0_CTS);
		SET_PERI_REG_MASK(UART_CONF0(config->port_no), UART_TX_FLOW_EN);
	}
	else
	{
		CLEAR_PERI_REG_MASK(UART_CONF0(config->port_no), UART_TX_FLOW_EN);
	}

	/* Set baud rate */
	uart_div_modify(config->port_no, UART_CLK_FREQ / config->baud_rate);

	WRITE_PERI_REG(UART_CONF0(config->port_no),
					((config->parity == UART_PARITY_NONE) ? 0x0 :
							(UART_PARITY_EN | config->parity)) |
					(config->stop_bits << UART_STOP_BIT_NUM_S) |
					(config->word_length << UART_BIT_NUM_S) |
					((config->hw_flow_ctrl & UART_HW_FLOW_CTRL_CTS) ?
							UART_TX_FLOW_EN : 0x0));

	/* Reset FIFO */
	SET_PERI_REG_MASK(UART_CONF0(config->port_no),
						UART_RXFIFO_RST | UART_TXFIFO_RST);
	CLEAR_PERI_REG_MASK(UART_CONF0(config->port_no),
						UART_RXFIFO_RST | UART_TXFIFO_RST);

	/* Register UART handler for the "printf" function */
	if (config->port_no == UART_PORT0)
		os_install_putc1(uart0_write_char);
	else
		os_install_putc1(uart1_write_char);
}

/* Private function definition section ====================================== */
static void uart_tx_one_char(UART_PortNo port_no, char tx_char)
{
	uint32 fifo_cnt;

	do
	{
		fifo_cnt = READ_PERI_REG(UART_STATUS(port_no)) &
						(UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
	} while ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) > 125);

	WRITE_PERI_REG(UART_FIFO(port_no), tx_char);
}

static void uart0_write_char(char c)
{
	if (c == '\n')
	{
		uart_tx_one_char(UART_PORT0, '\r');
		uart_tx_one_char(UART_PORT0, '\n');
	}
	else if (c != '\r')
	{
		uart_tx_one_char(UART_PORT0, c);
	}
}

static void uart1_write_char(char c)
{
	if (c == '\n')
	{
		uart_tx_one_char(UART_PORT1, '\r');
		uart_tx_one_char(UART_PORT1, '\n');
	}
	else if (c != '\r')
	{
		uart_tx_one_char(UART_PORT1, c);
	}
}

/* ============================= End of file ================================ */
