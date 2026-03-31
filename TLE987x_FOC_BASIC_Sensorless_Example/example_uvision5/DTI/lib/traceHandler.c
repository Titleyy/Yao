/**********************************************************************************************************************
 * File:  traceHandler.c	
 * RqID:  
 * Brief: Tracve handler for target interface
 * $Id: 0d0acef4622220493e68afb88d486741e3b5f611 $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-April-30 (V0.0.1):
 *     - Initial version 
 * 2020-September-30 (V0.0.2):
 *     - Sampling Service: Command GetClockSourceScaling added
 * 2020-October-09 (V0.0.3):
 *     - Sampling Service: Consistent use of maxSrc as available number of sampling sources
 *
 */

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include <stdint.h>
#include <stdbool.h>
#include "platform.h"

#include "dti.h"
#include "signalHandler_api.h"

#include "targetInterface.h"
#include "traceHandler.h"
#include "streamingService.h"
#include "eventService.h"
#include "samplingService.h"
#include "parameterService.h"


/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/					
#define TRCH_SERVICE_TRCH_MAX_CONFIGURATIONS   (2)
#define TRCH_SERVICE_TRCH_MAX_TRACEID          (2)
#define TRCH_VERSION_VALUE                     DTI_VERSION(1,0,0)

#if (TRCH_SERVICE_PROGRAMMING_available)
	#define TRCH_SERVICE_PROGRAMMING_value (1UL)
#else
	#define TRCH_SERVICE_PROGRAMMING_value (0UL)
#endif

#if (TRCH_SERVICE_STREAMING_available)
	#define TRCH_SERVICE_STREAMING_value (1UL)
#else
	#define TRCH_SERVICE_STREAMING_value  (0UL)
#endif

#if (TRCH_SERVICE_EVENT_available)
	#define TRCH_SERVICE_EVENT_value (1UL)
#else
	#define TRCH_SERVICE_EVENT_value  (0UL)
#endif

#if (TRCH_SERVICE_SAMPLING_available)
	#define TRCH_SERVICE_SAMPLING_value (1UL)
#else
	#define TRCH_SERVICE_SAMPLING_value  (0UL)
#endif

#if (TRCH_SERVICE_PARAMETER_available) 
	#define TRCH_SERVICE_PARAMETER_value (1UL)
#else
	#define TRCH_SERVICE_PARAMETER_value  (0UL)
#endif

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					

typedef struct {
	uint16_t services;
	uint32_t traceId[TRCH_SERVICE_TRCH_MAX_TRACEID];
} TRCH_vt;


typedef union {
	uint32_t configuration[TRCH_SERVICE_TRCH_MAX_CONFIGURATIONS];
	struct 
	{
		uint16_t version;
		uint16_t reserved;
		uint16_t capability;
		uint8_t maxId;
		uint8_t reserved1;
	};
} TRCH_ct;

/**********************************************************************************************************************
* LOCAL ENUMS
**********************************************************************************************************************/
typedef enum{
	TRCH_response_size_GetConfiguration = 8,
	TRCH_response_size_EnableServices = 6,
	TRCH_response_size_DisableServices = 6,
	TRCH_response_size_SyncTargetTime = 8,
	TRCH_response_size_GetFrameStatistics = 12,
	TRCH_response_size_ResetServices = 4,
	TRCH_response_size_GetTraceId = 8,
	TRCH_response_size_ERROR = 6,
} TRCH_response_size_e;


/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
const TRCH_ct TRCH_cnf = {
	.version = TRCH_VERSION_VALUE,
	.capability = (1UL<<TRCH_SERVICE_TRACEHANDLER) | /* The trace handler itself */
	              (TRCH_SERVICE_PROGRAMMING_value<<TRCH_SERVICE_PROGRAMMING) |  
	              (TRCH_SERVICE_STREAMING_value<<TRCH_SERVICE_STREAMING) | 
	              (TRCH_SERVICE_EVENT_value<<TRCH_SERVICE_EVENT) | 
	              (TRCH_SERVICE_SAMPLING_value<<TRCH_SERVICE_SAMPLING) | 
	              (TRCH_SERVICE_PARAMETER_value<<TRCH_SERVICE_PARAMETER) |    
	               0UL,
	.maxId = 		TRCH_SERVICE_TRCH_MAX_TRACEID
};

TRCH_vt TRCH_var = {
	.services = (1UL<<TRCH_SERVICE_TRACEHANDLER), /* The trace handler itself, must be enabled */
	.traceId = {0,0},
};

TRCH_buffer_t TRCH_request_buffer;
TRCH_buffer_t TRCH_response_buffer;

/***********************************************************************************************************************
 * LOCAL ROUTINES
 **********************************************************************************************************************/
void TRCH_ProcessRequest( TRCH_buffer_t* request, TRCH_buffer_t* response);

static TRCH_ERRORCODES_e TRCH_Service_TraceHandler(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e TRCH_Service_TRCH_GetConfiguration(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e TRCH_Service_TRCH_EnableDisableServices(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e TRCH_Service_TRCH_SyncTargetTime(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e TRCH_Service_TRCH_GetFrameStatistics(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e TRCH_Service_TRCH_Reset(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e TRCH_Service_TRCH_SetGetTraceId(TRCH_buffer_t*,TRCH_buffer_t*);

/******************************************************************************************/
void TRCH_Init(void)
{
	DTI_arbiter_setPortRX(DTI_PORT_TraceHander, &TRCH_request_buffer, sizeof(TRCH_request_buffer)); 
	DTI_arbiter_setPortTX(DTI_PORT_TraceHander, &TRCH_response_buffer, sizeof(TRCH_response_buffer)); 
}

uint32_t TRCH_ProcessTrace(void)
{
	int32_t message_length;
	static uint8_t subblock = 0;

	message_length = DTI_arbiter_getLengthPortRX(DTI_PORT_TraceHander);
	if( message_length > 0 )
	{ /* new message received */
 		TRCH_ProcessRequest( &TRCH_request_buffer, &TRCH_response_buffer);
		if( !DTI_arbiter_isBusyPortTX(DTI_PORT_TraceHander) )
		{ /* the transmit port is ready to use */
			DTI_arbiter_setReadyPortRX(DTI_PORT_TraceHander); /* allow to receive new message */
			DTI_arbiter_setSizePortTX(DTI_PORT_TraceHander, TRCH_response_buffer.size); /* send response */
		}
		else {/* do nothing, send response at next cycle */}
	}
	else
	{ /* no new message reveived */
		if( DTI_arbiter_isBusyPortRX(DTI_PORT_TraceHander) )
		{ /* there was a response waiting to be transmitted */
			DTI_arbiter_setReadyPortRX(DTI_PORT_TraceHander); /* allow to receive new message */
			DTI_arbiter_setSizePortTX(DTI_PORT_TraceHander, TRCH_response_buffer.size); /* send response */
		}
		else
		{ /* ready to send asyncronous response */
			if( !TRCH_EventService_isBufferIdle() )
			{ /* there is an event waiting in the buffer */
				if( !DTI_arbiter_isBusyPortTX(DTI_PORT_TraceHander) )
				{ /* the transmit port is ready to use */
					/* transmit one event from the buffer */
					
					TRCH_response_buffer.service = TRCH_SERVICE_EVENT;
					TRCH_response_buffer.cmd = TRCH_SERVICE_RESPONSE_ASYNC;
					TRCH_EventService_getBufferEntry( &TRCH_response_buffer);
					DTI_arbiter_setSizePortTX(DTI_PORT_TraceHander, TRCH_response_buffer.size);
				}	
				else {/*do nothing*/}
			}	
			else 
			{
				if( !TRCH_ParameterService_isBufferIdle() )
				{/* there is a subscribed parameter waiting in the buffer */
					if( !DTI_arbiter_isBusyPortTX(DTI_PORT_TraceHander) )
					{ /* the transmit port is ready to use */
						/* transmit one parameter from the buffer */
						TRCH_response_buffer.service = TRCH_SERVICE_PARAMETER;
						TRCH_response_buffer.cmd = TRCH_SERVICE_RESPONSE_ASYNC;
						TRCH_ParameterService_getBufferEntry( &TRCH_response_buffer);
						DTI_arbiter_setSizePortTX(DTI_PORT_TraceHander, TRCH_response_buffer.size);
					}	
					else {/*do nothing*/}
				}
				else
				{ 
					if( TRCH_SamplingService_isReady() || subblock )
					{ /* there is a sampling buffer waiting to be transfered */
						if( !DTI_arbiter_isBusyPortTX(DTI_PORT_TraceHander) )
						{ /* the transmit port is ready to use right now */
							if( TRCH_ERRORCODES_Aborted == TRCH_SamplingService_getBufferEntry( &TRCH_response_buffer, subblock ) )
							{
									subblock = 0;
									TRCH_SamplingService_ReleaseSampling();
							}
							else
							{
								if( DTI_arbiter_setSizePortTX(DTI_PORT_TraceHander, TRCH_response_buffer.size) )
								{ /* check again, if the port was still ready to use */
									if( TRCH_SamplingService_getNumBlocks() == subblock )
									{
										subblock = 0;
										TRCH_SamplingService_ReleaseSampling();
									}
									else 
									{
										++subblock;
									}
								}
								else {/*do nothing*/}
							}
						}	
						else {/*do nothing*/}
					}	
					else {/*do nothing*/}
				}
			}
		}
	}
	return message_length;
}

typedef enum {
	TRCH_CallService_action,
	TRCH_CallService_init,
	TRCH_CallService_deinit,
	TRCH_CallService_MAX
} TRCH_CallService_e;

TRCH_ERRORCODES_e (*TRCH_CallService[][TRCH_CallService_MAX])(TRCH_buffer_t*,TRCH_buffer_t*) = 
{
	{TRCH_Service_TraceHandler,NULL,NULL},
	{NULL,NULL,NULL},
	{TRCH_StreamingService,TRCH_StreamingService_init,TRCH_StreamingService_deinit},
	{TRCH_EventService,TRCH_EventService_init,TRCH_EventService_deinit},
	{TRCH_SamplingService,TRCH_SamplingService_init,TRCH_SamplingService_deinit},
	{TRCH_ParameterService,TRCH_ParameterService_init,TRCH_ParameterService_deinit},
	{NULL,NULL,NULL},
};

void TRCH_ProcessRequest( TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	if( request->service < (sizeof(TRCH_CallService) / (TRCH_CallService_MAX*sizeof(TRCH_ERRORCODES_e(*)(TRCH_buffer_t*,TRCH_buffer_t*)))) )
	{
		if( TRCH_var.services & (1<<request->service) )
		{			
			errorcode = TRCH_CallService[request->service][0](request, response);
		}
		else
		{
			errorcode = TRCH_ERRORCODES_ServiceNotEnabled;
		}
	}
	else
	{
		errorcode = TRCH_ERRORCODES_UnkownCommand;
	}

	response->service = request->service;
	if( errorcode )
	{
		response->cmd = TRCH_SERVICE_RESPONSE_ERROR;
		response->data16[0] = errorcode;
		response->size = TRCH_response_size_ERROR;
	}
	else {/* do nothing */}
}

TRCH_ERRORCODES_e (*TRCH_TraceHandlerCmd[])(TRCH_buffer_t*,TRCH_buffer_t*) = 
{
	TRCH_Service_TRCH_GetConfiguration,
	TRCH_Service_TRCH_GetConfiguration,
	TRCH_Service_TRCH_EnableDisableServices,
	TRCH_Service_TRCH_EnableDisableServices,
	TRCH_Service_TRCH_SyncTargetTime,
	TRCH_Service_TRCH_GetFrameStatistics,
	TRCH_Service_TRCH_Reset,
	TRCH_Service_TRCH_SetGetTraceId,
};


TRCH_ERRORCODES_e TRCH_Service_TraceHandler(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	if( request->cmd < (sizeof(TRCH_TraceHandlerCmd) / sizeof(TRCH_ERRORCODES_e(*)(TRCH_buffer_t*,TRCH_buffer_t*))) )
	{
		errorcode = TRCH_TraceHandlerCmd[request->cmd](request, response);
	}
	else
	{
		errorcode = TRCH_ERRORCODES_UnkownCommand;
	}
	if( errorcode )
	{
		response->data16[0] = errorcode;
	}
	else {/* do nothing */}
	
	return errorcode;
}

TRCH_ERRORCODES_e TRCH_Service_TRCH_GetConfiguration(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	response->cmd = request->cmd;
	response->channel = 0; 
	response->data32[0] = TRCH_cnf.configuration[request->cmd];
	response->size = TRCH_response_size_GetConfiguration;
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e TRCH_Service_TRCH_EnableDisableServices(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	uint16_t mask = 0x0001U;
	uint16_t services;
	TRCH_ERRORCODES_e errorcode;
	
	errorcode = TRCH_ERRORCODES_Success;
	response->cmd = request->cmd;
	response->channel = 0;
	
	/* check if unavailable services were selected */
	if( (~TRCH_cnf.capability) & request->data16[0] ) 
	{
		errorcode = TRCH_ERRORCODES_ServiceNotAvailable;
		response->channel = 0x0F;
	}
	else
	{	
		/* only enable/disable available services */
		services = request->data16[0] & TRCH_cnf.capability;
		for(uint32_t loop = 0; loop < TRCH_SERVICE_MAX; ++loop)
		{
			if( mask & services )
			{
				if( request->cmd == 2 )
				{ 
					if( ! (TRCH_var.services & mask) )
					{
						if( TRCH_CallService[loop][TRCH_CallService_init] )
						{
							/* Enable service */
							errorcode = TRCH_CallService[loop][TRCH_CallService_init](request, response);
							TRCH_var.services |= mask;
						}
						else
						{
							errorcode = TRCH_ERRORCODES_ServiceEnableFailed;
							response->channel = loop;
							break;
						}
					}
					else
					{
						errorcode = TRCH_ERRORCODES_ServiceAlreadyEnabled;
						response->channel = loop;
						break;
					}
				}
				else
				{
					if( TRCH_var.services & mask )
					{
						if( TRCH_CallService[loop][TRCH_CallService_deinit] )
						{
							/* Disable service */
							errorcode = TRCH_CallService[loop][TRCH_CallService_deinit](request, response);
							TRCH_var.services &= ~mask;
						}
						else
						{
							errorcode = TRCH_ERRORCODES_ServiceDisableFailed;
							response->channel = loop;
							break;
						}
					}
					else
					{
						errorcode = TRCH_ERRORCODES_ServiceAlreadyDisabled;
						response->channel = loop;
						break;
					}
				}
			} 
			else {/*do nothing*/}
			
			if( errorcode )
			{
				response->channel = loop;
				break;
			}
			else {/*do nothing*/}
			mask <<= 1; /* prepare mask for next service number */
		}
	}
	response->data16[0] = TRCH_var.services;
	response->size = TRCH_response_size_EnableServices;
	return errorcode;
}

TRCH_ERRORCODES_e TRCH_Service_TRCH_SyncTargetTime(TRCH_buffer_t* request, TRCH_buffer_t* response )
{
	DTI_sendSyncFrame_DU();
	response->cmd = request->cmd;
	response->channel = 0; 
	response->data32[0] = DTI_TSdriver_getTargetTime();
	response->size = TRCH_response_size_SyncTargetTime;
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e TRCH_Service_TRCH_GetFrameStatistics(TRCH_buffer_t* request, TRCH_buffer_t* response )
{
	TRCH_ERRORCODES_e errorcode;

	response->cmd = request->cmd;
	response->channel = request->channel; 
	switch( request->channel )
	{
		case 0:
			response->data32[0] = DTI_buffer.frame_DU.frameCount;
			response->data32[1] = DTI_buffer.frame_DU.errorCount;
			errorcode = TRCH_ERRORCODES_Success;
		break;
		
		case 1:
			response->data32[0] = DTI_buffer.frame_DD.frameCount;
			response->data32[1] = DTI_buffer.frame_DD.errorCount;
			errorcode = TRCH_ERRORCODES_Success;
		break;
		default:
			errorcode = TRCH_ERRORCODES_StatisticNotAvailable;
			break;
	}
	response->size = TRCH_response_size_GetFrameStatistics;
	return errorcode;
}

TRCH_ERRORCODES_e TRCH_Service_TRCH_Reset(TRCH_buffer_t* request, TRCH_buffer_t* response )
{
	uint16_t mask = 0x0001U;
	response->cmd = request->cmd;
	
	for(uint32_t loop = 0; loop < TRCH_SERVICE_MAX; ++loop)
	{
		/* only disable available services */
		if( mask & TRCH_cnf.capability )
		{
			/* only disable enabled services */
			if( TRCH_var.services & mask )
			{
				if( TRCH_CallService[loop][TRCH_CallService_deinit] )
				{
					/* Disable service */
					TRCH_CallService[loop][TRCH_CallService_deinit](request, response);
					TRCH_var.services &= ~mask;
				}
			}
		}
		else {/*do nothing*/}
		mask <<= 1; /* prepare mask for next service number */
	}

	for( uint32_t loop = 0; loop < TRCH_SERVICE_TRCH_MAX_TRACEID; ++loop)
	TRCH_var.traceId[loop] = 0;
	
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e TRCH_Service_TRCH_SetGetTraceId(TRCH_buffer_t* request, TRCH_buffer_t* response )
{
	TRCH_ERRORCODES_e errorcode;

	response->cmd = request->cmd;
	response->id = request->id; 

	if( request->id < TRCH_SERVICE_TRCH_MAX_TRACEID )
	{
		switch( request->subcmd )
		{
			case 0:
				TRCH_var.traceId[request->id] = request->data32[0];
				response->data32[0] = TRCH_var.traceId[request->id];
				errorcode = TRCH_ERRORCODES_Success;
				break;

			case 1:
				response->data32[0] = TRCH_var.traceId[request->id];
				errorcode = TRCH_ERRORCODES_Success;
				break;

			default:
				errorcode = TRCH_ERRORCODES_CommandNotAvailable;
				break;
		}
	}
	else
	{
		errorcode = TRCH_ERRORCODES_IdNotAvailable;
	}
	return errorcode;
}
