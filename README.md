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


==========================================================

主要概念就是先定義 command protocol 為底下這樣的結構 , 

header + function code + length + data + checksum + tailer

header : comand 開頭 , 用來定義是否為起始點

tailer : comand 結尾 , 用來定義是否為結束點

function code : 每次下command , 是想要寫資料 , 或是回讀資料 , 另外也作為判斷command 是否回應正常或是出現其他異常 (check UARTCMD_RESULT_t !)

length : 為每次下command 時 , 期望要寫入 , 或是回讀資料的長度

data : 為一個陣列 , 目前範例是訂為4 個byte , 可寫入4 個byte , 回傳4 個byte

checksum : function + data 總和 , 用0 去減


參考// TODO: Step1~6 , 可以加速porting 到project 

關鍵字//customize , 可以自訂 , 改為自己的project application

每個command 上面的command 有command example (目前是使用PC 端terminal 做TX 的腳色 , 用MCU 當RX 接收)
