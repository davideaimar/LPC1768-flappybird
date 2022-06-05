/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "lpc17xx.h"
#include "RIT.h"
#include "../GLCD/GLCD.h"
#include "../button_EXINT/button.h"
#include "../can/can_lib.h"
#include "../flappy_bird/flappy_bird.h"
#include <stdio.h>
#include "../timer/timer.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/

int down_INT0=0;
int down_KEY1=0;
int down_KEY2=0;
extern uint8_t ch1_count;
extern uint8_t ch2_count;
extern uint8_t ch1_same_lobby_count;
extern uint8_t ch2_same_lobby_count;

volatile uint8_t lobby = 1; // 1...7 -> first 3 bit of CAN ID
CAN_MSG rec_data;
extern CAN_MSG send_data;
extern uint32_t counter;
extern uint8_t GAME_STATUS;

void RIT_IRQHandler (void)
{						
	/* button management */
	/* INT0 */
	if(down_INT0!=0){ 
		if((LPC_GPIO2->FIOPIN & (1<<10)) == 0){	/* INT0 pressed */
			down_INT0++;				
			switch(down_INT0){
				case 2:
					// RIGHT BUTTON CODE HERE 
					if (GAME_STATUS==0){
						lobby = lobby == 7 ? 7 : lobby+1;
						// draw_bottom_line();
						launch_sync();
						launch_sync();
						// draw_bottom_line();
					}
					break;
				default:
					break;
			}
		}
		else {	/* button released */
			down_INT0=0;			
			NVIC_EnableIRQ(EINT0_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 20);     /* External interrupt 0 pin selection */
		}
	}
	/* KEY1 */
	if(down_KEY1!=0){ 
		if((LPC_GPIO2->FIOPIN & (1<<11)) == 0){	/* KEY1 pressed */
			down_KEY1++;				
			switch(down_KEY1){
				case 2:
					// LEFT BUTTON CODE HERE 
					if (GAME_STATUS==0){
						lobby = lobby == 1 ? 1 : lobby-1;
						// draw_bottom_line();
						launch_sync();
						launch_sync();
						// draw_bottom_line();
					}
					break;
				default:
					break;
			}
		}
		else {	/* button released */
			down_KEY1=0;			
			NVIC_EnableIRQ(EINT1_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 22);     /* External interrupt 0 pin selection */
		}
	}
	/* KEY2 */
	if(down_KEY2!=0){ 
		if((LPC_GPIO2->FIOPIN & (1<<12)) == 0){	/* KEY2 pressed */
			down_KEY2++;				
			switch(down_KEY2){
				case 2:
					// CENTRAL BUTTON CODE HERE 
					if (GAME_STATUS==0){
						launch_sync();
						// draw_bottom_line();
					} else if (GAME_STATUS == 2){
						GAME_STATUS = 3;
						disable_timer(0);
					} else if (GAME_STATUS == 3){
						GAME_STATUS = 2;
						enable_timer(0);
					}
					break;
				default:
					break;
			}
		}
		else {	/* button released */
			down_KEY2=0;			
			NVIC_EnableIRQ(EINT2_IRQn);							 /* enable Button interrupts			*/
			LPC_PINCON->PINSEL4    |= (1 << 24);     /* External interrupt 0 pin selection */
		}
	}
	
	
//	/* joystick management */
//	if((LPC_GPIO1->FIOPIN & (1<<25)) == 0){	/* Joytick Select pressed */
//		counter += 1;
//		sprintf(str, "%d", counter);
//		GUI_Text(0, 0, (uint8_t *) str, White, Red);
//		send_data.id = 0x1;
//		send_data.id |= lobby << 8;
//		send_data.msg_type = 0;
//		send_data.id_format = 0;
//		send_data.len = 0;
//		if (ch1_count>0) CAN_Send(LPC_CAN1, &send_data);
//		if (ch2_count>0) CAN_Send(LPC_CAN2, &send_data);
//	}
//	else if((LPC_GPIO1->FIOPIN & (1<<26)) == 0){	/* Joytick down pulled */
//	}
//	else if((LPC_GPIO1->FIOPIN & (1<<27)) == 0){	/* Joytick left pulled */
//			
//	}
//	else if((LPC_GPIO1->FIOPIN & (1<<28)) == 0){	/* Joytick right pulled */
//		
//	}
//	else if((LPC_GPIO1->FIOPIN & (1<<29)) == 0){	/* Joytick up pulled */
//	}
			
  LPC_RIT->RICTRL |= 0x1;	/* clear interrupt flag */
}

/******************************************************************************
**                            End Of File
******************************************************************************/
