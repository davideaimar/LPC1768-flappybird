
#include "lpc17xx.h"
#include "can_lib.h"
CAN_MSG send_data;


// credits to NXP MCU SW Application Team
int CAN_SetBaudrate(LPC_CAN_TypeDef * CH, uint32_t baudrate) {
	uint32_t result = 0;
	uint8_t NT, TSEG1, TSEG2, BRFail;
	uint32_t CANPclk = 0;
	uint32_t BRP;
	
	if (CH!=LPC_CAN1 && CH != LPC_CAN2) return -1;

	CANPclk = SystemFrequency / 4;

	result = CANPclk / baudrate;
	/* Calculate suitable nominal time value
	 * NT (nominal time) = (TSEG1 + TSEG2 + 3)
	 * NT <= 24
	 * TSEG1 >= 2*TSEG2
	 */
	BRFail = 1;
	for(NT=24;NT>0;NT=NT-2) {
		if ((result%NT)==0) {
			BRP = result / NT - 1;
			NT--;
			TSEG2 = (NT/3) - 1;
			TSEG1 = NT -(NT/3) - 1;
			BRFail = 0;
			break;
		}
	}
	if(BRFail)
		return -1; // Failed to calculate exact CAN baud rate
	/* Enter reset mode */
	CH->MOD = 0x01;
	/* Set bit timing
	 * Default: SAM = 0x00;
	 *          SJW = 0x03;
	 */
	CH->BTR  = (TSEG2<<20)|(TSEG1<<16)|(3<<14)|BRP;
	/* Return to normal operating */
	CH->MOD = 0;
	return 0;
}

/**********************************************************************
 * @brief		Initialize CAN peripheral
 * @param[in]	CANx pointer to LPC_CAN_TypeDef, should be:
 * 				- LPC_CAN1: CAN1 peripheral
 * 				- LPC_CAN2: CAN2 peripheral
 * @param[in]	baudrate of the channel to initialize
 * @return 		None
 *********************************************************************/
int CAN_Init(LPC_CAN_TypeDef * CH, uint32_t baudrate){
	if (CH!=LPC_CAN1 && CH != LPC_CAN2) return -1;
	
	// Set PCLK_CAN1, PCLK_CAN2 and PCLK_ACF to CCLK/4 = 25MHz (they MUST be the same)
	LPC_SC->PCLKSEL0 &= ~(0xFC000000);
	//Setup GPIO port 2 first 8 bits as output
	LPC_GPIO2->FIODIR |= 0xff ;
	// SETUP GPIO port 1 pin number 26 as input
	LPC_GPIO1->FIODIR &= (1 << 25);
	
	
	if (CH==LPC_CAN1){
		LPC_SC->PCONP |= (1<<13);// Power to CAN CH1
		
		// CAN1_RX -> PIN 0.0 to function 01 for RD1
		// CAN1_TX -> PIN 0.1 to function 01 for TD1 
		LPC_PINCON->PINSEL0 &= ~(0xF); 
		LPC_PINCON->PINSEL0 |= 0x5;
		
		LPC_PINCON->PINMODE0 &= ~(0xF);
		LPC_PINCON->PINMODE0 |= ( (0x2) | (0x2 << 2) );
	}
	else {
		LPC_SC->PCONP |= (1<<14);// Power to CAN CH2
		
		// CAN2_TX -> PIN 0.5 to function 10 for TD2
		// CAN2_RX -> PIN 0.4 to function 10 for RD2
		LPC_PINCON->PINSEL0 &= ~(0xF<<8); 
		LPC_PINCON->PINSEL0 |= 0xA<<8; 
		
		LPC_PINCON->PINMODE0 &= ~(0xF<<8);
		LPC_PINCON->PINMODE0 |= ( (0x2<<8) | (0x2 << 10) );
	}
	
	CH->MOD = 1;    /* Reset CAN */
  CH->IER = 0;    /* Disable Receive Interrupt */
  CH->GSR = 0;    /* Reset error counter when CANxMOD is in reset  */
  CH->MOD = 0x0;  /* CAN in normal operation mode */
	
	LPC_CANAF->AFMR = 1<<1; // set acceptance filter in bypass mode
	
	if (CAN_SetBaudrate(CH, baudrate)!=0) // set the baudrate of the channel
		return -1;
	
	return 0;
}

/**********************************************************************
 * @brief		Send data through CAN peripheral
 * @param[in]	CANx pointer to LPC_CAN_TypeDef, should be:
 * 				- LPC_CAN1: CAN1 peripheral
 * 				- LPC_CAN2: CAN2 peripheral
 * @param[in]	CAN_Msg pointer to the data to send
 * @return 		None
 *********************************************************************/
int CAN_Send(LPC_CAN_TypeDef * CH, CAN_MSG * CAN_Msg){
	uint32_t data;
	
	if (CH!=LPC_CAN1 && CH != LPC_CAN2) return -1;
	
	if ( CH->SR & 1<<2 ){
		// transmit on TB1
		
		CH->TFI1 &= ~0x000F0000;
		CH->TFI1 |= (CAN_Msg->len)<<16;
		// Set DataFrame Type
		CH->TFI1 &= ~(1<<30);
		if (CAN_Msg->id_format==0) {
			// set Standard ID format
			CH->TFI1 &= ~(1UL<<31);
		}
		else{
			// set Extended ID format
			CH->TFI1 |= (1UL<<31);
		}
		if (CAN_Msg->msg_type==0){
			// Set normal msg
			CH->TFI1 &= ~(1<<30);
			
		}else{
			// set Remote Type msg
			CH->TFI1 |= (1<<30);
		}

		/* Write CAN ID*/
		CH->TID1 = CAN_Msg->id;

		/*Write first 4 data bytes*/
		data = (CAN_Msg->dataA[0])|(((CAN_Msg->dataA[1]))<<8)|((CAN_Msg->dataA[2])<<16)|((CAN_Msg->dataA[3])<<24);
		CH->TDA1 = data;

		/*Write second 4 data bytes*/
		data = (CAN_Msg->dataB[0])|(((CAN_Msg->dataB[1]))<<8)|((CAN_Msg->dataB[2])<<16)|((CAN_Msg->dataB[3])<<24);
		CH->TDB1 = data;

		/*Write transmission request*/
		CH->CMR = 0x21;
		return 0;
	}
	else if ( CH->SR & 1<<10 ){
		// transmit on TB2
		
		/* Write frame informations and frame data into its CAN1TFI1,
		 * CAN1TID1, CAN1TDA1, CAN1TDB1 register */
		
		CH->TFI2 &= ~0x000F0000;
		CH->TFI2 |= (CAN_Msg->len)<<16;
		// Set DataFrame Type
		CH->TFI2 &= ~(1<<30);
		if (CAN_Msg->id_format==0) {
			// set Standard ID format
			CH->TFI2 &= ~(1UL<<31);
		}
		else {
			// set Extended ID format
			CH->TFI1 |= (1UL<<31);
		}
		if (CAN_Msg->msg_type==0){
			// Set normal msg
			CH->TFI2 &= ~(1UL<<31);
			
		}else{
			// set Remote Type msg
			CH->TFI2 |= (1<<30);
		}

		/* Write CAN ID*/
		CH->TID2 = CAN_Msg->id;

		/*Write first 4 data bytes*/
		data = (CAN_Msg->dataA[0])|(((CAN_Msg->dataA[1]))<<8)|((CAN_Msg->dataA[2])<<16)|((CAN_Msg->dataA[3])<<24);
		CH->TDA2 = data;

		/*Write second 4 data bytes*/
		data = (CAN_Msg->dataB[0])|(((CAN_Msg->dataB[1]))<<8)|((CAN_Msg->dataB[2])<<16)|((CAN_Msg->dataB[3])<<24);
		CH->TDB2 = data;

		/*Write transmission request*/
		CH->CMR = 0x41;
		return 0;
	}
	else if ( CH->SR & 1<<18 ){
		// transmit on TB3
		/* Write frame informations and frame data into its CAN1TFI13,
		 * CAN1TID3, CAN1TDA3, CAN1TDB3 register */
		
		CH->TFI3 &= ~0x000F0000;
		CH->TFI3 |= (CAN_Msg->len)<<16;
		// Set DataFrame Type
		CH->TFI3 &= ~(1<<30);
		if (CAN_Msg->id_format==0) {
			// set Standard ID format
			CH->TFI3 &= ~(1UL<<31);
		}
		else {
			// set Extended ID format
			CH->TFI1 |= (1UL<<31);
		}
		if (CAN_Msg->msg_type==0){
			// Set normal msg
			CH->TFI3 &= ~(1<<30);
			
		}else{
			// set Remote Type msg
			CH->TFI3 |= (1<<30);
		}

		/* Write CAN ID*/
		CH->TID3 = CAN_Msg->id;

		/*Write first 4 data bytes*/
		data = (CAN_Msg->dataA[0])|(((CAN_Msg->dataA[1]))<<8)|((CAN_Msg->dataA[2])<<16)|((CAN_Msg->dataA[3])<<24);
		CH->TDA3 = data;

		/*Write second 4 data bytes*/
		data = (CAN_Msg->dataB[0])|(((CAN_Msg->dataB[1]))<<8)|((CAN_Msg->dataB[2])<<16)|((CAN_Msg->dataB[3])<<24);
		CH->TDB3 = data;

		/*Write transmission request*/
		CH->CMR = 0x81;
		return 0;
	}
	return -1;
	
}

/**********************************************************************
 * @brief		Read received data in a CAN peripheral
 * @param[in]	CANx pointer to LPC_CAN_TypeDef, should be:
 * 				- LPC_CAN1: CAN1 peripheral
 * 				- LPC_CAN2: CAN2 peripheral
 * @param[in]	CAN_Msg poiter for storing the recieved data
 * @return 		None
 *********************************************************************/
int CAN_Receive(LPC_CAN_TypeDef * CH, CAN_MSG *Received_data){
	uint32_t data;
	
	if (CH!=LPC_CAN1 && CH != LPC_CAN2) return -1;
	
	//check status of Receive Buffer
	if((CH->SR &0x00000001))
	{
		/* Receive message is available */
		/* Read frame informations */
		Received_data->len      = (uint8_t)(((CH->RFS) & 0x000F0000)>>16);
		Received_data->id_format   = (uint8_t)(((CH->RFS) & 0x80000000)>>31);
		Received_data->msg_type     = (uint8_t)(((CH->RFS) & 0x40000000)>>30);


		/* Read CAN message identifier */
		Received_data->id = CH->RID;

		/* Read the data if received message was DATA FRAME */
		if ( Received_data->msg_type == 0 ) {
			/* Read first 4 data bytes */
			data = CH->RDA;
			*((uint8_t *) &Received_data->dataA[0])= data & 0x000000FF;
			*((uint8_t *) &Received_data->dataA[1])= (data & 0x0000FF00)>>8;;
			*((uint8_t *) &Received_data->dataA[2])= (data & 0x00FF0000)>>16;
			*((uint8_t *) &Received_data->dataA[3])= (data & 0xFF000000)>>24;

			/* Read second 4 data bytes */
			data = CH->RDB;
			*((uint8_t *) &Received_data->dataB[0])= data & 0x000000FF;
			*((uint8_t *) &Received_data->dataB[1])= (data & 0x0000FF00)>>8;
			*((uint8_t *) &Received_data->dataB[2])= (data & 0x00FF0000)>>16;
			*((uint8_t *) &Received_data->dataB[3])= (data & 0xFF000000)>>24;

			/*release receive buffer*/
			CH->CMR = 0x01 << 2;
		}
		else
		{
			/* Received Frame is a Remote Frame, not have data, we just receive
			 * message information only */
			CH->CMR = 0x01 << 2; /*release receive buffer*/
			return 0;
		}
	}
	else
	{
		// no receive message available
		return -1;
	}
	return 0;
}

/**********************************************************************
 * @brief		Check if a CAN channel has messages in receive buffer
 * @param[in]	CANx pointer to LPC_CAN_TypeDef, should be:
 * 				- LPC_CAN1: CAN1 peripheral
 * 				- LPC_CAN2: CAN2 peripheral
 * @return 		1 if present 0 otherwise
 *********************************************************************/
int CAN_HasReceivedMessage(LPC_CAN_TypeDef * CH){
	if (CH!=LPC_CAN1 && CH != LPC_CAN2) return -1;
	return ( CH->SR & 0x00000001 );
}

/**********************************************************************
 * @brief		Enable interrupt on message receive on channel CH
 * @param[in]	CANx pointer to LPC_CAN_TypeDef, should be:
 * 				- LPC_CAN1: CAN1 peripheral
 * 				- LPC_CAN2: CAN2 peripheral
 * @return 		void
 *********************************************************************/
void CAN_EnableInterrupt(LPC_CAN_TypeDef * CH){
	CH->IER = 0x01; /* Enable receive interrupts */
	NVIC_EnableIRQ(CAN_IRQn);
	NVIC_SetPriority(CAN_IRQn, 1);
}

/**********************************************************************
 * @brief		Disable interrupt on message receive on channel CH
 * @param[in]	CANx pointer to LPC_CAN_TypeDef, should be:
 * 				- LPC_CAN1: CAN1 peripheral
 * 				- LPC_CAN2: CAN2 peripheral
 * @return 		void
 *********************************************************************/
void CAN_DisableInterrupt(LPC_CAN_TypeDef * CH){
	CH->IER = 0x0; /* Disable receive interrupts */
}

void CAN_resetTXerr(LPC_CAN_TypeDef * CH){
	CH->MOD = 1;    /* Reset CAN */
  CH->GSR &= 0x00FFFFFF;    /* Reset error counter  */
  CH->MOD = 0;  /* CAN in normal operation mode */
}
