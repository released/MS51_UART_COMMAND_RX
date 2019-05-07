# MS51_UART_COMMAND_RX

update @ 2019/05/07

Add receive UART command by protocol and some example ,

CMD structure

header + function code + length + data + checksum + tailer

UART CMD format
0   	1   	2    	3   	4   	5   	6   	7		8

HEADER 	FUNC 	LEN  	D1  	D2  	D3  	D4		CHK 	TAILER

Follow // TODO: Step1 ~ Step6 to porting

with variable or define with comment //customize , may change the value depend on expectation

use below command with PC terminal (TX) to test directly , get report from MCU (RX) , refer to other example in comment

TX : TX send data 0x11 , 0x22 , 0x33 , 0x44 , with function code 0x0A

\34\0A\04\11\22\33\44\48\81

RX : if confirm checksum correct , will feedback UARTCMD_ACK (B0)

34 B0 00 00 00 00 00 50 81