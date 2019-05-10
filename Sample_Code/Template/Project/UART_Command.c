
#include "UART_Command.h"


#define UARTCMD_WILDCARD						(0xFD)

//CMD ack define
#define UARTCMD_ACK							(0xB0)
#define UARTCMD_NOT_SUPPORT					(0xB1)
#define UARTCMD_ERROR_CHECKSUM				(0xB2)
#define UARTCMD_NACK							(0xB3)
#define UARTCMD_BAD_FORMAT					(0xB4)

#define UARTCMD_IND_NOT_SUPPORT				"NSPT"
#define UARTCMD_IND_ERROR_CHECKSUM			"ECKM"
#define UARTCMD_IND_NACK						"NACK"
#define UARTCMD_IND_BAD_FORMAT				"BFMT"

//UART CMD variable
uint8_t 	u8UartRxCnt = 0;
uint8_t 	u8SetProcessPkt = 0;

//	CMD structure 
//	header + function code + length + data + checksum + tailer

//	UART CMD format
//	0   	1   	2    	3   	4   	5   	6   	7		8
//	HEADER 	FUNC 	LEN  	D1  	D2  	D3  	D4		CHK 	TAILER

//	response 
//	HEAD  	B0  	00  	00  	00  	00  	00		50  	TAILER //command send in ok and process
//	HEAD  	B1  	00  	00  	00  	00  	00		4F  	TAILER //command not correct or no process


// TODO: Step #1 , add function code index
//CMD function code table index define
typedef enum
{
	UARTCMD_FUNCTION01_SET = 0x0A,
	UARTCMD_FUNCTION02_GET = 0x0B,	
	UARTCMD_FUNCTION03_GET = 0x0C,
	UARTCMD_FUNCTION04_GET = 0x0D,
	
	UARTCMD_FUNCTION05_SET = 0x3E,
	UARTCMD_FUNCTION06_GET = 0x3F,
	
	UARTCMD_FUNCTION_MAX,
}UARTCMD_FUNCTION_CODE;	//customize

typedef enum
{
	RESULT_ACK_OK = 0,
	RESULT_ACK_NOT_SUPPORTED,	
	RESULT_ACK_ERROR_CHECKSUM,
	RESULT_ACK_NACK, 
	RESULT_ACK_BAD_FORMAT, 
	
	RESULT_MAX,
} UARTCMD_RESULT_t;

typedef struct {
	uint8_t header;
	uint8_t function;
	uint8_t datalength;
	uint8_t transferdata[UARTCMD_DATA_LEN];
	uint8_t checksum;
	uint8_t tailer;
}UARTCMD_BUF_t;

typedef struct
{
	UARTCMD_FUNCTION_CODE cmd_code;
	uint8_t data_length;
	uint8_t data_template;
	UARTCMD_RESULT_t (*handler) (UARTCMD_BUF_t *cmd);
} UARTCMD_TEMPLATE_t;

#define UARTCMD_CHKSUM_OFFSET					(sizeof(UARTCMD_BUF_t)-1)
#define UARTCMD_LENGTH							(sizeof(UARTCMD_BUF_t))

uint8_t 					aui1_cmd_buf[UARTCMD_LENGTH];
UARTCMD_BUF_t				UartCommand;

static void _UartCmdParser_CmdFormat(uint8_t length,uint8_t *transferdata);
static void _UartCmdParser_ClearDataBuffer(void);


//	UART CMD format
//	0   	1   	2    	3   	4   	5   	6   	7		8
//	HEADER 	0x0A 	4  		D0  	D1  	D2  	D3		CHK 	TAILER

/*
	Example 1: 
	TX send data 0x11 , 0x22 , 0x33 , 0x44 , with function code 0x0A
	\34\0A\04\11\22\33\44\48\81

	after RX receive , if confirm checksum correct , will feedback UARTCMD_ACK (B0)
	34 B0 00 00 00 00 00 50 81	

	Example 2:	
	if use wrong checksum , 
	\34\0a\04\11\22\33\44\49\81

	 will feedback UARTCMD_ERROR_CHECKSUM (B2)
	34 B2 04 45 43 4B 4D 2A 81

	Example 3: 	
	if use wrong function code with correct checksum , 
	\34\28\04\11\22\33\44\2A\81

	 will feedback UARTCMD_NACK (B3)
	34 B3 04 4E 41 43 4B 2C 81
*/

// TODO: Step #2 , modify function to expect behavior , set data by TX or get data and return to TX 
UARTCMD_RESULT_t CustomUartCmdHandler_Function01_Set(UARTCMD_BUF_t *cmd)	//customize
{
	uint8_t temp = 0;	//example , put command data into a variable
	uint16_t i = 0;

	//for this example , use temp to store data
	for (i=0;i<UARTCMD_DATA_LEN;i++)
	{


		temp = cmd->transferdata[i];
	}

	(void) temp;
	
	return RESULT_ACK_OK;
}

/*
	Example : 
	TX request data with function code 0x0B , 
	\34\0B\01\00\00\00\00\F4\81

	after RX receive , if confirm checksum correct , will feedback UARTCMD_ACK (B0) with 1 byte data
	34 B0 01 96 00 00 00 B9 81	
*/
UARTCMD_RESULT_t CustomUartCmdHandler_Function02_Get(UARTCMD_BUF_t *cmd)	//customize
{
	uint8_t OutData = 0x96;		//example , set 1 byte data out

	_UartCmdParser_CmdFormat(UARTCMD_1_BYTE_DATA,&OutData);

	(void) cmd;
	
	return RESULT_ACK_OK;
}

/*
	Example : 
	TX request data with function code 0x0C , 
	\34\0C\02\00\00\00\00\F2\81

	after RX receive , if confirm checksum correct , will feedback UARTCMD_ACK (B0) with 2 byte data
	34 B0 02 12 34 00 00 08 81 	
*/
UARTCMD_RESULT_t CustomUartCmdHandler_Function03_Get(UARTCMD_BUF_t *cmd)	//customize
{
	uint8_t Output[UARTCMD_2_BYTE_DATA] = {0};	//example , set 2 byte bit data out

	Output[0] = 0x12;
	Output[1] = 0x34;

	_UartCmdParser_CmdFormat(UARTCMD_2_BYTE_DATA,Output);

	(void) cmd;
	
	return RESULT_ACK_OK;
} 

/*
	Example : 
	TX request data with function code 0x0D , 
	\34\0D\04\00\00\00\00\EF\81

	after RX receive , if confirm checksum correct , will feedback UARTCMD_ACK (B0) with 4 byte data
	34 B0 04 55 66 32 31 2E 81 	
*/
UARTCMD_RESULT_t CustomUartCmdHandler_Function04_Get(UARTCMD_BUF_t *cmd)	//customize
{
	uint8_t Output[UARTCMD_4_BYTE_DATA] = {0};	//example , set 4 byte bit data out

	Output[0] = 0x55;
	Output[1] = 0x66;
	Output[2] = 0x32;
	Output[3] = 0x31;
	
	_UartCmdParser_CmdFormat(UARTCMD_4_BYTE_DATA,Output);

	(void) cmd;
	
	return RESULT_ACK_OK;
} 

/*
	Example : 
	TX send data 0x13 , 0x57 , 0x90 with function code 0x3E
	\34\3E\03\13\57\90\00\C5\81

	after RX receive , if confirm checksum correct , will feedback UARTCMD_ACK (B0)
	34 B0 00 00 00 00 00 50 81 	
*/
UARTCMD_RESULT_t CustomUartCmdHandler_Function05_Set(UARTCMD_BUF_t *cmd)	//customize
{
	uint8_t temp = 0;	//example , put command data into a your application data address
	uint16_t i = 0;

	//for this example , use temp to store data
	for (i=0;i<UARTCMD_3_BYTE_DATA;i++)
	{


		temp = cmd->transferdata[i];
	}

	(void) temp;
	
	return RESULT_ACK_OK;
} 

/*
	Example : 
	TX request data with function code 0x3F , 
	\34\3F\03\00\00\00\00\BE\81

	after RX receive , if confirm checksum correct , will feedback UARTCMD_ACK (B0) with 3 byte data
	34 B0 03 13 57 00 90 53 81 	
*/

UARTCMD_RESULT_t CustomUartCmdHandler_Function06_Get(UARTCMD_BUF_t *cmd)	//customize
{
	uint8_t Output[UARTCMD_3_BYTE_DATA] = {0};	//example , set 3 byte bit data out

	Output[0]=0x13;
	Output[1]=0x57;
	Output[2]=0x90;
	
	_UartCmdParser_CmdFormat(UARTCMD_3_BYTE_DATA,Output);

	(void) cmd;
	
	return RESULT_ACK_OK;
} 

// TODO: Step #3 , register function code and function to array
UARTCMD_TEMPLATE_t _UartCmdParser_Array[] = 
{
	{UARTCMD_FUNCTION01_SET	,UARTCMD_4_BYTE_DATA	, UARTCMD_WILDCARD, CustomUartCmdHandler_Function01_Set},
	{UARTCMD_FUNCTION02_GET	,UARTCMD_1_BYTE_DATA	, UARTCMD_WILDCARD, CustomUartCmdHandler_Function02_Get},
	{UARTCMD_FUNCTION03_GET	,UARTCMD_2_BYTE_DATA	, UARTCMD_WILDCARD, CustomUartCmdHandler_Function03_Get},
	{UARTCMD_FUNCTION04_GET	,UARTCMD_4_BYTE_DATA	, UARTCMD_WILDCARD, CustomUartCmdHandler_Function04_Get},

	{UARTCMD_FUNCTION05_SET	,UARTCMD_3_BYTE_DATA	, UARTCMD_WILDCARD, CustomUartCmdHandler_Function05_Set},
	{UARTCMD_FUNCTION06_GET	,UARTCMD_3_BYTE_DATA	, UARTCMD_WILDCARD, CustomUartCmdHandler_Function06_Get},
	
	{UARTCMD_FUNCTION_MAX	,0x00					, UARTCMD_WILDCARD, NULL},  // this is the end of list marker
};

/*
	UART CMD format
	0   	1   	2    	3   	4   	5   	6   	7		8
	HEADER 	0x0A 	4  		D0  	D1  	D2  	D3		CHK 	TAILER

	CHECKSUM = 0 - (byte 1 + ... byte 6) , if UARTCMD_DATA_LEN define 4 
*/
static uint8_t _UartCmdParser_GetChecksum(UARTCMD_BUF_t *cmd )
{
    int i;
    uint8_t u8Checksum = 0;
    uint8_t* u8Ptr = (uint8_t*)cmd;

    for (i=1; i < UARTCMD_CHKSUM_OFFSET-1; i++)
        u8Checksum += *(u8Ptr+i);

    //return (0-ucChecksum);
    return (uint8_t) (0-u8Checksum);
}

static void _UartCmdParser_ClearDataBuffer(void)
{
	uint16_t len = 0;
	uint8_t ClearBuffer[UARTCMD_DATA_LEN] = {0};

	UartCommand.datalength = 0;	//UARTCMD_DATA_LEN;	
	for (len=0;len<UARTCMD_DATA_LEN;len++)
	{
		UartCommand.transferdata[len] = *(ClearBuffer+len);
	}	
}

static void _UartCmdParser_CmdFormat(uint8_t length,uint8_t *transferdata)
{
	//Need to manual decide the cmd output data
	uint16_t len = 0 ;

	UartCommand.datalength = length;	
	for (len=0;len<length;len++)
	{
		UartCommand.transferdata[len] = *(transferdata+len);
	}	
}

static void _UartCmdParser_SendFeedback(UARTCMD_RESULT_t result,UARTCMD_BUF_t *cmd)
{
	uint8_t i = 0;
	UARTCMD_BUF_t rcmd;
	(void) cmd;
	
	switch( result )
	{
		case RESULT_ACK_OK:
			//buffer will depend on each function return value
			rcmd.function = UARTCMD_ACK;
			break;

		case RESULT_ACK_NOT_SUPPORTED:
			_UartCmdParser_CmdFormat(strlen(UARTCMD_IND_NOT_SUPPORT),UARTCMD_IND_NOT_SUPPORT);//use NSUP to verify whats happen			
			rcmd.function = UARTCMD_NOT_SUPPORT;						
			break;
			
		case RESULT_ACK_ERROR_CHECKSUM:
			_UartCmdParser_CmdFormat(strlen(UARTCMD_IND_ERROR_CHECKSUM),UARTCMD_IND_ERROR_CHECKSUM);//use ESUM to verify whats happen		
			rcmd.function = UARTCMD_ERROR_CHECKSUM;
			break;

		case RESULT_ACK_BAD_FORMAT:
			_UartCmdParser_CmdFormat(strlen(UARTCMD_IND_BAD_FORMAT),UARTCMD_IND_BAD_FORMAT);//use BADF to verify whats happen		
			rcmd.function = UARTCMD_BAD_FORMAT;
			break;
			
		default:
			_UartCmdParser_CmdFormat(strlen(UARTCMD_IND_NACK),UARTCMD_IND_NACK);//use NACK to verify whats happen
			rcmd.function = UARTCMD_NACK;
			break;
	}
	
	rcmd.header = UARTCMD_HEADER;

	//manual clear data
	rcmd.datalength = UartCommand.datalength; 
	for (i=0;i<UARTCMD_DATA_LEN;i++)
	{
		rcmd.transferdata[i]= UartCommand.transferdata[i]; 
	}
	
	rcmd.checksum = _UartCmdParser_GetChecksum(&rcmd);
	rcmd.tailer = UARTCMD_TAILER;

	UartCmd_Send((uint8_t*)&rcmd,UARTCMD_LENGTH);

}


// TODO: Step #4 , add function code to set command
static void _UartCmdParser_Set_cmd(UARTCMD_BUF_t *cmd)
{
	UARTCMD_RESULT_t result = RESULT_ACK_OK ;

	_UartCmdParser_ClearDataBuffer();

	switch(cmd->function)
	{
		case UARTCMD_FUNCTION01_SET:
			result = CustomUartCmdHandler_Function01_Set(cmd);
			break;
		case UARTCMD_FUNCTION02_GET:
			result = CustomUartCmdHandler_Function02_Get(cmd);
			break;			
		case UARTCMD_FUNCTION03_GET:
			result = CustomUartCmdHandler_Function03_Get(cmd);
			break;
		case UARTCMD_FUNCTION04_GET:
			result = CustomUartCmdHandler_Function04_Get(cmd);
			break;
		case UARTCMD_FUNCTION05_SET:
			result = CustomUartCmdHandler_Function05_Set(cmd);
			break;
		case UARTCMD_FUNCTION06_GET:
			result = CustomUartCmdHandler_Function06_Get(cmd);
			break;			
	}		

	_UartCmdParser_SendFeedback(result ,cmd);	
}

static int _UartCmdParser_FindHandlerIndex(UARTCMD_BUF_t *cmd)
{
    int i;

    for (i=0; _UartCmdParser_Array[i].cmd_code != 0xFF; i++)
    {
        if ((cmd->header == UARTCMD_HEADER) &&
          (cmd->function == _UartCmdParser_Array[i].cmd_code) &&
          (cmd->tailer == UARTCMD_TAILER))
		{
			#if 0	//debug
			//UART_Send_Data(UART0 ,_UartCmdParser_Array[i].datalength);
			//UART_Send_Data(UART0 ,cmd->length);			
			//UART_Send_Data(UART0 , _UartCmdParser_Array[i].data1);	
			//UART_Send_Data(UART0 , cmd->data[0]);	
			#endif
			
			if ( (_UartCmdParser_Array[i].data_length == UARTCMD_WILDCARD && _UartCmdParser_Array[i].data_template == UARTCMD_WILDCARD) ||
				(_UartCmdParser_Array[i].data_length == cmd->datalength && _UartCmdParser_Array[i].data_template == UARTCMD_WILDCARD) ||
				(_UartCmdParser_Array[i].data_length == UARTCMD_WILDCARD && _UartCmdParser_Array[i].data_template == cmd->transferdata[0]) ||
				(_UartCmdParser_Array[i].data_length == cmd->datalength && _UartCmdParser_Array[i].data_template == cmd->transferdata[0]) )
			{
				return i;
			}
			else
				continue;
        } //if 
   	}
    return -1;	//if we reached here then we never found the entry in the table
}

static void _UartCmdParser_Parse_cmd(void)
{
	UARTCMD_BUF_t*	cmd = (UARTCMD_BUF_t*)aui1_cmd_buf;
	int 			nTemplateIdx = 0;

	/* make sure we got a good packet. */
	if ( cmd->checksum != _UartCmdParser_GetChecksum(cmd) )
	{		
		#if 0 //debug		
		//Printf("RESULT_ACK_ERROR_CHECKSUM\n");		
		UartCmd_Send("\r1" , sizeof("\r1"));		
		#endif /*debug*/
		
		_UartCmdParser_SendFeedback(RESULT_ACK_ERROR_CHECKSUM, cmd);
		return;
	}

	/* if we can't find a match in our template, then its not good!! */
	nTemplateIdx = _UartCmdParser_FindHandlerIndex(cmd);
	if (nTemplateIdx < 0)
	{
		#if 0 //debug		
		//Printf("RESULT_ACK_NACK\n");		
		UartCmd_Send("\r1" , sizeof("\r1"));		
		#endif /*debug*/
	
		_UartCmdParser_SendFeedback(RESULT_ACK_NACK, cmd);
		return;
	}

	/* see if handler exist */
	if ( _UartCmdParser_Array[nTemplateIdx].handler == NULL )
	{
		#if 0 //debug		
		//Printf("RESULT_ACK_NOT_SUPPORTED\n");		
		UartCmd_Send("RESULT_ACK_NOT_SUPPORTED" , sizeof("RESULT_ACK_NOT_SUPPORTED"));		
		#endif /*debug*/		
		_UartCmdParser_SendFeedback(RESULT_ACK_NOT_SUPPORTED, cmd);
		return; 
	}

	/* finally execute the handler */
	_UartCmdParser_Set_cmd(cmd);

}

void SystemClkDelay(uint16_t Delay)
{
	while(Delay--);
}


// TODO: Step #5 , add UartCmd_Rx_Receive to UART IRQ handler
void UartCmd_Rx_Receive(uint8_t tmp)
{
	aui1_cmd_buf[u8UartRxCnt++] = tmp;

	//force to check buffer header/length/tail , reset counter if meet cmd format
	if(u8UartRxCnt !=0 ||	aui1_cmd_buf[u8UartRxCnt] == UARTCMD_HEADER)		
	{
		if ( u8UartRxCnt == UARTCMD_LENGTH )		
		{
			if ( aui1_cmd_buf[u8UartRxCnt-1] == UARTCMD_TAILER )
			{
				u8UartRxCnt = 0;
				u8SetProcessPkt = 1;
			}
			else
			{
				//bad cmd format , send not support ack			
				_UartCmdParser_SendFeedback(RESULT_ACK_BAD_FORMAT, NULL);	
				
				#if 0 //debug
				UartCmd_Send(">>>\r\n",sizeof(">>>\r\n"));				
				UART_Send_Data(UART0 , 	UARTCMD_LENGTH);
				UartCmd_Send(">>>\r\n",sizeof(">>>\r\n"));
				UartCmd_Send(aui1_cmd_buf,sizeof( aui1_cmd_buf));
				UartCmd_Send("\r\n<<<\r\n",sizeof("\r\n<<<\r\n"));
				#endif /*debug*/	

				//need to manual reset the buffer data , prevent buffer overflow
				memset( aui1_cmd_buf,0,sizeof( aui1_cmd_buf));	
				u8UartRxCnt = 0;
				u8SetProcessPkt = 0;	
			}
		}
	}

	//make sure clear RX flag	
}

void UartCmd_Send(uint8_t *pui1_data ,uint8_t z_len )
{
   	uint8_t i;

	DISABLE_UART0_INTERRUPT;	//ES = 0

	for (i=0 ;i<z_len;i++)
	{
		SystemClkDelay(0x40);
		UART_Send_Data(UART0 , (uint8_t)*(pui1_data+i));
	}

	ENABLE_UART0_INTERRUPT;	//ES = 1
}

// TODO: Step #6 , add UART0_Process to while loop
void UART0_Process(void)
{
	if(u8SetProcessPkt)
	{
		u8SetProcessPkt=0;

		_UartCmdParser_Parse_cmd();

		#if 0 //debug
		UartCmd_Send(aui1_cmd_buf,sizeof( aui1_cmd_buf));
		#endif

		//reset buffer after cmd process
		memset( aui1_cmd_buf,0,sizeof( aui1_cmd_buf));			
	}
}


