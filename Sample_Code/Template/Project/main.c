/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) 2019 Nuvoton Technology Corp. All rights reserved.                                         */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

/***********************************************************************************************************/
/* Website: http://www.nuvoton.com                                                                         */
/*  E-Mail : MicroC-8bit@nuvoton.com                                                                       */
/*  Date   : Jan/21/2019                                                                                   */
/***********************************************************************************************************/

/************************************************************************************************************/
/*  File Function: MS51 DEMO project                                                                        */
/************************************************************************************************************/

//#include "MS51.h"
#include <stdio.h>

#include "UART_Command.h"

#define SYS_CLOCK 								(24000000ul)

#define UART_INTERRUPT

//Timer variable 
#define TIMER_LOG_MS							(1000)
#define GPIO_TOGGLE_MS							(500)

uint8_t 	u8TH0_Tmp = 0;
uint8_t 	u8TL0_Tmp = 0;

void GPIO_Toggle(void)
{
    static uint8_t flag = 1;

	if (flag)
	{
		P12 = 1;
		flag = 0;
	}
	else
	{
		P12 = 0;
		flag = 1;
	}
}

void GPIO_Init(void)
{
    P12_PUSHPULL_MODE;
}

void Timer0_IRQHandler(void)
{
//	static uint16_t LOG_TIMER = 0;
	static uint16_t CNT_TIMER = 0;
	static uint16_t CNT_GPIO = 0;

	if (CNT_GPIO++ >= GPIO_TOGGLE_MS)
	{		
		CNT_GPIO = 0;
		GPIO_Toggle();
	}	

	if (CNT_TIMER++ >= TIMER_LOG_MS)
	{		
		CNT_TIMER = 0;
//    	printf("LOG:%d\r\n",LOG_TIMER++);
	}
}

void Timer0_ISR(void) interrupt 1        // Vector @  0x0B
{
    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;
    clr_TCON_TF0;
	
	Timer0_IRQHandler();
}

void BasicTimer_TIMER0_Init(void)
{
	uint16_t res = 0;

	ENABLE_TIMER0_MODE1;
	
	u8TH0_Tmp = HIBYTE(TIMER_DIV12_VALUE_1ms_FOSC_240000);
	u8TL0_Tmp = LOBYTE(TIMER_DIV12_VALUE_1ms_FOSC_240000); 

    TH0 = u8TH0_Tmp;
    TL0 = u8TL0_Tmp;

    ENABLE_TIMER0_INTERRUPT;                       //enable Timer0 interrupt
    ENABLE_GLOBAL_INTERRUPT;                       //enable interrupts
  
    set_TCON_TR0;                                  //Timer0 run
}

void UART0_ISR(void) interrupt 4         // Vector @  0x23
{
    if (RI)
    {   
//		DISABLE_UART0_INTERRUPT;		//clr_IE_ES;
		UartCmdParser_Rx_Receive(SBUF);
	  
		clr_SCON_RI;                                         // Clear RI (Receive Interrupt).
//		ENABLE_UART0_INTERRUPT;		//set_IE_ES;
    }
    if (TI)
    {       
        clr_SCON_TI;                                       // Clear TI (Transmit Interrupt).
    }
}

void UART0_Init(void)
{
    UART_Open(SYS_CLOCK,UART0_Timer3,115200);
    ENABLE_UART0_PRINTF; 

	#if defined (UART_INTERRUPT)
    ENABLE_UART0_INTERRUPT;                             /* Enable UART0 interrupt */
    ENABLE_GLOBAL_INTERRUPT;                            /* Global interrupt enable */	
	#endif

	u8UartRxCnt=0;
	u8SetProcessPkt=0;
	
}

void SYS_Init(void)
{
    MODIFY_HIRC_24();

    ALL_GPIO_QUASI_MODE;
    ENABLE_GLOBAL_INTERRUPT;                // global enable bit	
}

void main (void) 
{
    SYS_Init();

    UART0_Init();

	GPIO_Init();					//P1.2 , GPIO
			
	BasicTimer_TIMER0_Init();
	

    while(1)
    {

		UART0_Process();
    }
}



