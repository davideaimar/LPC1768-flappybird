
#include "can_lib.h"
#include <stdio.h>
#include "lpc17xx.h"
#include "../GLCD/GLCD.h" 
#include "../RIT/RIT.h"

char str[40];
uint8_t rec = 0;
volatile uint8_t ch1_count = 0;
volatile uint8_t ch2_count = 0;
extern uint8_t lobby;
uint32_t counter = 0;
uint16_t line = 0;

void print_debug(uint8_t * text){
	#ifdef DEBUG
	int i;
	GUI_Text(0, line, text, Black, Cyan);
	line += 16;
	if (line > MAX_Y - 50){
		line = 0;
		for(i=0; i<MAX_X;i++)
			LCD_DrawLine(i, 0, i, MAX_Y-50, Cyan);
	}
	#endif
}

static void replicate_message(LPC_CAN_TypeDef * REC_CH, CAN_MSG * msg){
	if (REC_CH==LPC_CAN1 && ch2_count>0)	CAN_Send(LPC_CAN2, msg);
	else if (REC_CH==LPC_CAN2 && ch1_count>0)	CAN_Send(LPC_CAN1, msg);
}

static void decodeMessage(CAN_MSG * rec_data, LPC_CAN_TypeDef * REC_CH) {
	uint8_t rec_lobby, rec_id, rec_rp_count;
	
	// get che lobby by the bits 9..11 of the received ID
	rec_lobby = (0x700 & rec_data->id) >> 8;
	// broadcast lobby for sync messages
	if (rec_lobby==0){
		rec_id = rec_data->id & 0xff;
		switch (rec_id){
			case 0x0:
				// received sync request
				// forward the request (error handling is in the function)
				print_debug((uint8_t *) "Received sync request");
				send_syncrq(REC_CH==LPC_CAN1 ? LPC_CAN2 : LPC_CAN1);
				break;
			case 0x1:
				// received sync reply
				rec_rp_count = rec_data->dataA[0];
				if (REC_CH==LPC_CAN1){
					ch1_count = rec_rp_count;
					sprintf(str, "Rep rec on CH1. CH1: %d CH2: %d", ch1_count, ch2_count);
					print_debug((uint8_t *) str); // debug
					send_syncrp(LPC_CAN2, rec_rp_count+1);
				}else{
					ch2_count = rec_rp_count;
					sprintf(str, "Rep rec on CH2. CH1: %d CH2: %d", ch1_count, ch2_count);
					print_debug((uint8_t *) str); // debug
					send_syncrp(LPC_CAN1, rec_rp_count+1);
				}
				sprintf(str, "CH1: %d - CH2: %d", ch1_count, ch2_count);
				GUI_Text(0, MAX_Y-16,(uint8_t *) str, Black, Green);
				break;
			default:
				replicate_message(REC_CH, rec_data);
				break;
		}
	}
	// if the received message is in the same lobby of the current instance then process the message.
	else if (rec_lobby == lobby) {
		rec_id = rec_data->id & 0xff;
		switch (rec_id) {
			case 0x1:
				break;
			default:
				break;
		}
		// replicate the message if needed
		replicate_message(REC_CH, rec_data);
	} else {
		// replicate the message if needed
		replicate_message(REC_CH, rec_data);
	}
}

void CAN_IRQHandler(){
	CAN_MSG rec_data;
	// if CH1 has a message to be received
	if ( CAN_HasReceivedMessage(LPC_CAN1) ) {
		// receive the message 
    if (CAN_Receive(LPC_CAN1, &rec_data)==0){
			decodeMessage(&rec_data, LPC_CAN1);
		}
  }
  if ( CAN_HasReceivedMessage(LPC_CAN2) ) {
		// receive the message 
    if (CAN_Receive(LPC_CAN2, &rec_data)==0){
			decodeMessage(&rec_data, LPC_CAN2);
		}
  }
}
