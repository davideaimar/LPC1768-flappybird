
#include "lpc17xx.h"
#include "can_lib.h"
#include "../GLCD/GLCD.h" 

extern uint8_t ch1_count;
extern uint8_t ch2_count;
extern uint8_t ch1_same_lobby_count;
extern uint8_t ch2_same_lobby_count;
extern uint8_t lobby;
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
			send_syncrp(LPC_CAN2, 1, &send_data); 
		}
		else {
			ch2_count = 0;  
			send_syncrp(LPC_CAN1, 1, &send_data);
		}
		//CAN_resetTXerr(CH);
	}
}

void send_syncrp(LPC_CAN_TypeDef * CH, uint8_t cnt, CAN_MSG * source_packet){
	if (CH==LPC_CAN1)
		print_debug((uint8_t *) "Send rep on CH1");
	else
		print_debug((uint8_t *) "Send rep on CH2");
	send_data.id = 0x001;
	send_data.len = 8;
	send_data.id_format = 0;
	send_data.msg_type = 0;
	send_data.dataA[0] = cnt;
	if (cnt == 1) {
		// if first reply from the edge
		send_data.dataA[1] = send_data.dataA[2] = send_data.dataA[3] = 0x0;
		send_data.dataB[0] = send_data.dataB[1] = send_data.dataB[2] = send_data.dataB[3] = 0x0;
		if (lobby <= 3) {
			send_data.dataA[lobby] = 0x1;
		} else {
			send_data.dataB[lobby-4] = 0x1;
		}
	} else {
		// if not first reply from the edge
		send_data.dataA[1] = source_packet->dataA[1];
		send_data.dataA[2] = source_packet->dataA[2];
		send_data.dataA[3] = source_packet->dataA[3];
		send_data.dataB[0] = source_packet->dataB[0];
		send_data.dataB[1] = source_packet->dataB[1];
		send_data.dataB[2] = source_packet->dataB[2];
		send_data.dataB[3] = source_packet->dataB[3];
		if (lobby <= 3) {
			send_data.dataA[lobby] = send_data.dataA[lobby] + 0x1;
		} else {
			send_data.dataB[lobby-4] = send_data.dataB[lobby-4] + 0x1;
		}
	}
	CAN_Send(CH, &send_data);
	//CAN_resetTXerr(CH);
}

void FlappyCAN_Send1(){
	send_data.id = 0x1;
	send_data.id |= lobby << 8;
	send_data.msg_type = 0;
	send_data.id_format = 0;
	send_data.len = 0;
	send_data.dataA[0] = send_data.dataA[1] = send_data.dataA[2] = send_data.dataA[3] = 0x0;
	send_data.dataB[0] = send_data.dataB[1] = send_data.dataB[2] = send_data.dataB[3] = 0x0;
	if (ch1_count>0) CAN_Send(LPC_CAN1, &send_data);
	if (ch2_count>0) CAN_Send(LPC_CAN2, &send_data);
}

void FlappyCAN_Send2(uint16_t start_y, int16_t start_speed, uint16_t score){
	send_data.id = 0x2;
	send_data.id |= lobby << 8;
	send_data.msg_type = 0;
	send_data.id_format = 0;
	send_data.len = 6;
	send_data.dataA[0] = ((start_y & 0xFF00) >> 8);
	send_data.dataA[1] = start_y & 0xFF;
	send_data.dataA[2] = ((start_speed & 0xFF00) >> 8);
	send_data.dataA[3] = start_speed & 0xFF;
	send_data.dataB[0] = ((score & 0xFF00) >> 8);
	send_data.dataB[1] = score & 0xFF;
	send_data.dataB[2] = send_data.dataB[3] = 0x0;
	if (ch1_count>0) CAN_Send(LPC_CAN1, &send_data);
	if (ch2_count>0) CAN_Send(LPC_CAN2, &send_data);
}

void FlappyCAN_Send3(uint16_t score){
	send_data.id = 0x3;
	send_data.id |= lobby << 8;
	send_data.msg_type = 0;
	send_data.id_format = 0;
	send_data.len = 2;
	send_data.dataA[0] = ((score & 0xFF00) >> 8);
	send_data.dataA[1] = score & 0xFF;
	send_data.dataA[2] = send_data.dataA[3] = 0x0;
	send_data.dataB[0] = send_data.dataB[1] = send_data.dataB[2] = send_data.dataB[3] = 0x0;
	if (ch1_count>0) CAN_Send(LPC_CAN1, &send_data);
	if (ch2_count>0) CAN_Send(LPC_CAN2, &send_data);
}
