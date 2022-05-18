/****************************************Copyright (c)****************************************************
**
**                                 http://www.powermcu.com
**
**--------------File Info---------------------------------------------------------------------------------
** File name:               main.c
** Descriptions:            The GLCD application function
**
**--------------------------------------------------------------------------------------------------------
** Created by:              AVRman
** Created date:            2010-11-7
** Version:                 v1.0
** Descriptions:            The original version
**
**--------------------------------------------------------------------------------------------------------
** Modified by:             Paolo Bernardi
** Modified date:           03/01/2020
** Version:                 v2.0
** Descriptions:            basic program for LCD and Touch Panel teaching
**
*********************************************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"
#include "GLCD/GLCD.h"
#include "timer/timer.h"
#include "can/can_lib.h"
#include "RIT/RIT.h"
#include "button_EXINT/button.h"
#include "TouchPanel/TouchPanel.h"
#include "flappy_bird/flappy_bird.h"
#include <stdlib.h>

int main(void)
{

	SystemInit(); /* System Initialization (i.e., PLL)  */

	BUTTON_init();

	/* these lines are used to enable DAC */
	LPC_PINCON->PINSEL1 |= (1 << 21);
	LPC_PINCON->PINSEL1 &= ~(1 << 20);
	LPC_GPIO0->FIODIR |= (1 << 26);

	/* Initialize random seed */
	srand((unsigned)1);

	LCD_Initialization();
	TP_Init();
	init_RIT(0x004C4B40);
	CAN_Init(LPC_CAN1, 125000);
	CAN_Init(LPC_CAN2, 125000);

	TouchPanel_Calibrate();
	enable_RIT();

	// init_timer(1, 0x5F5E100 ); 						/* 500ms * 25MHz */

	GUI_Text(MAX_X / 2 + 20, MAX_Y - 16, (uint8_t *)"ID lobby: 1", Black, Green);

	CAN_EnableInterrupt(LPC_CAN1);
	CAN_EnableInterrupt(LPC_CAN2);

	launch_sync();

	game_set(150, 0, 0, 0);

	LPC_SC->PCON |= 0x1; /* power-down	mode										*/
	LPC_SC->PCON &= ~(0x2);

	while (1)
	{
		__ASM("wfi");
	}
}

/*********************************************************************************************************
	  END FILE
*********************************************************************************************************/
