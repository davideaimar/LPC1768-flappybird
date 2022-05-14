
#ifndef __CAN_H
#define __CAN_H

#include "lpc17xx.h"
extern uint32_t SystemFrequency;

/**
 * @brief CAN message object structure
 */
typedef struct {
	uint8_t id_format; 	/* 1 -> 29 bit; 0 -> 11 bit					*/
	uint32_t id; 				/* 11 or 29 bit id 									*/
	uint8_t len; 				/* Length of data field in bytes 		*/
	uint8_t msg_type;		/* 0 -> data frame; 1 -> remote frame */
	uint8_t dataA[4]; 	/* Data field A 										*/
	uint8_t dataB[4]; 	/* Data field B 										*/
} CAN_MSG;

int CAN_Init(LPC_CAN_TypeDef * CH, uint32_t baudrate);
int CAN_Send(LPC_CAN_TypeDef * CH, CAN_MSG * CAN_Msg);
int CAN_Receive(LPC_CAN_TypeDef * CH, CAN_MSG *Received_data);
int CAN_HasReceivedMessage(LPC_CAN_TypeDef * CH);
void CAN_EnableInterrupt(LPC_CAN_TypeDef * CH);
void CAN_DisableInterrupt(LPC_CAN_TypeDef * CH);
void CAN_resetTXerr(LPC_CAN_TypeDef * CH);
void launch_sync(void);
void send_syncrq(LPC_CAN_TypeDef * CH);
void send_syncrp(LPC_CAN_TypeDef * CH, uint8_t cnt, CAN_MSG * source_packet);
void print_debug(uint8_t * text);
void FlappyCAN_Send1(void);
void FlappyCAN_Send2(uint16_t start_y, int16_t start_speed, uint16_t score);
void FlappyCAN_Send3(uint16_t score);

#endif
