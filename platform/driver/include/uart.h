#ifndef __UART_H__
#define __UART_H__

/* Inclusion section ======================================================== */

/* Public macro definition section ========================================== */

/* Public type definition section =========================================== */
typedef enum
{
	UART_PORT0 = 0x0,
	UART_PORT1 = 0x1,
} UART_PortNo;

typedef enum
{
	UART_BAUD_RATE_300		= 300,
	UART_BAUD_RATE_600		= 600,
	UART_BAUD_RATE_1200		= 1200,
	UART_BAUD_RATE_2400		= 2400,
	UART_BAUD_RATE_4800		= 4800,
	UART_BAUD_RATE_9600		= 9600,
	UART_BAUD_RATE_19200	= 19200,
	UART_BAUD_RATE_38400	= 38400,
	UART_BAUD_RATE_57600	= 57600,
	UART_BAUD_RATE_74480	= 74480,
	UART_BAUD_RATE_115200	= 115200,
	UART_BAUD_RATE_230400	= 230400,
	UART_BAUD_RATE_460800	= 460800,
	UART_BAUD_RATE_921600	= 921600,
	UART_BAUD_RATE_1843200	= 1843200,
	UART_BAUD_RATE_3686400	= 3686400
} UART_BaudRate;

typedef enum
{
	UART_WORD_LENGTH_5b = 0x0,
	UART_WORD_LENGTH_6b = 0x1,
	UART_WORD_LENGTH_7b = 0x2,
	UART_WORD_LENGTH_8b = 0x3
} UART_WordLength;

typedef enum
{
	UART_PARITY_NONE	= 0x2,
	UART_PARITY_EVEN	= 0x0,
	UART_PARITY_ODD		= 0x1
} UART_ParityMode;

typedef enum
{
	UART_STOP_BITS_1	= 0x1,
	UART_STOP_BITS_1_5	= 0x2,
	UART_STOP_BITS_2	= 0x3,
} UART_StopBits;

typedef enum
{
	UART_HW_FLOW_CTRL_NONE		= 0x0,
	UART_HW_FLOW_CTRL_RTS		= 0x1,
	UART_HW_FLOW_CTRL_CTS		= 0x2,
	UART_HW_FLOW_CTRL_CTS_RTS	= 0x3
} UART_HwFlowCtrl;

typedef struct
{
	UART_PortNo			port_no;
	UART_BaudRate		baud_rate;
	UART_WordLength		word_length;
	UART_ParityMode		parity;
	UART_StopBits		stop_bits;
	UART_HwFlowCtrl		hw_flow_ctrl;
} UART_Config;

/* Public function prototype section ======================================== */
void UART_Init(UART_Config *config);

#endif
