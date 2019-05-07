
#include "MS51.h"
#include <stdio.h>

#define UARTCMD_HEADER							(0x34)	//customize
#define UARTCMD_TAILER							(0x81)	//customize
#define UARTCMD_DATA_LEN						(4)		//customize

typedef enum
{
	UARTCMD_1_BYTE_DATA = 1,
	UARTCMD_2_BYTE_DATA = 2,	
	UARTCMD_3_BYTE_DATA = 3,
	UARTCMD_4_BYTE_DATA = 4,
	
	UARTCMD_BYTE_MAX,
}UARTCMD_BYTE_DATA;	//customize , because UARTCMD_DATA_LEN define 4


//extern UART CMD variable
extern uint8_t u8UartRxCnt;
extern uint8_t u8SetProcessPkt;

void UartCmdParser_Rx_Receive(uint8_t tmp);
void UART0_Process(void);
void UartCmd_Send(uint8_t *pui1_data ,uint8_t z_len );




