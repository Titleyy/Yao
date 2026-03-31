/**********************************************************************************************************************
 * File:  messageHandler.c	
 * RqID:  
 * Brief: Messager Handler for target interface
 * $Id: 835e725acdb6b9a80290da0b26c7b26360a66e15 $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-25:
 *     - Renamed from messageHandler_api.c	into messageHandler.c, because this file is not the API
 * 2020-May-12:
 *     - Renamed from messageHandler.c	into messageHandler_api.c
 *     - provide APIs for use with callback and with scheduler (polling)
 * 2020-April-30:
 *     - Initial version
 *
 */
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include <stdint.h>
#include <stdbool.h>
#include "platform.h"
#include "messageHandler.h"
#include "targetInterface.h"
#include "sdoService.h"

/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
static SDOSERVICE_buffer_t MSGH_SDO_request_buffer;
static SDOSERVICE_buffer_t MSGH_SDO_response_buffer;

/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
static inline void MSGH_Init(void)
{
	DTI_arbiter_setPortRX(DTI_PORT_MessageHander, &MSGH_SDO_request_buffer, sizeof(MSGH_SDO_request_buffer)); 
	DTI_arbiter_setPortTX(DTI_PORT_MessageHander, &MSGH_SDO_response_buffer, sizeof(MSGH_SDO_response_buffer)); 
}

static inline void MSGH_Callback_ProcessMessage(DTI_port_t *port)
{ 
	SDOSERVICE_ProcessRequest( &MSGH_SDO_request_buffer, &MSGH_SDO_response_buffer);
	DTI_arbiter_setReadyPortRX(DTI_PORT_MessageHander);
	DTI_arbiter_setReadyPortTX(DTI_PORT_MessageHander);
}

void MSGH_Callback_Init(void)
{
	MSGH_Init();
/* Use callback after buffer has been received */
	DTI_arbiter_setCallbackPortRX(DTI_PORT_MessageHander, MSGH_Callback_ProcessMessage);
}

void MSGH_Polling_Init(void)
{
	MSGH_Init();
	DTI_arbiter_setCallbackPortRX(DTI_PORT_MessageHander, NULL);
}

uint32_t MSGH_Polling_ProcessMessage(void)
{
	int32_t message_length;
	
	message_length = DTI_arbiter_getLengthPortRX(DTI_PORT_MessageHander);
	if( message_length > 0 )
	{ /* new message received */
 		SDOSERVICE_ProcessRequest( &MSGH_SDO_request_buffer, &MSGH_SDO_response_buffer);
		DTI_arbiter_setReadyPortRX(DTI_PORT_MessageHander);
		DTI_arbiter_setReadyPortTX(DTI_PORT_MessageHander);
	}
	else{/*do nothing*/}	
	
	return message_length;
}

