
#include "lpc17xx.h"
#include "can_lib.h"
#include "../GLCD/GLCD.h" 

extern uint8_t ch1_count;
extern uint8_t ch2_count;
extern CAN_MSG send_data;

void launch_sync(){
	send_syncrq(LPC_CAN1);
	send_syncrq(LPC_CAN2);
}

void send_syncrq(LPC_CAN_TypeDef * CH){
	uint8_t attempt = 3;
	if (CH==LPC_CAN1)
		print_debug((uint8_t *) "Send req on CH1");
	else
		print_debug((uint8_t *) "Send req on CH2");
	send_data.id = 0x000;
	send_data.len = 0;
	send_data.id_format = 0;
	send_data.msg_type = 0;
	//CAN_resetTXerr(CH);
	while (CAN_Send(CH, &send_data)!= 0 && attempt > 0 )
		attempt--;
	// wait for the transmission to finish
	// while ( ( (CH->GSR & 0x20) >> 5) == 1 );
	while( ( (CH->GSR & 0x8) >> 3) == 0 && (CH->GSR & 0xFF000000) != 0x80000000 );
	if ( ( (CH->GSR & 0x8) >> 3) == 0 || attempt == 0){
		print_debug((uint8_t *) "Send failed      ");
		// if error on send
		// send reply to other channel with cnt = 1
		if (CH==LPC_CAN1){
			ch1_count = 0;
			send_syncrp(LPC_CAN2, 1); 
		}
		else {
			ch2_count = 0;  
			send_syncrp(LPC_CAN1, 1);
		}
		//CAN_resetTXerr(CH);
	}
}

void send_syncrp(LPC_CAN_TypeDef * CH, uint8_t cnt){
	if (CH==LPC_CAN1)
		print_debug((uint8_t *) "Send rep on CH1");
	else
		print_debug((uint8_t *) "Send rep on CH2");
	send_data.id = 0x001;
	send_data.len = 1;
	send_data.id_format = 0;
	send_data.msg_type = 0;
	send_data.dataA[0] = cnt;
	CAN_Send(CH, &send_data);
	//CAN_resetTXerr(CH);
}
