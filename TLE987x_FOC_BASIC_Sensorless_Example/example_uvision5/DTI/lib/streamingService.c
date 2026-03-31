/**********************************************************************************************************************
 * File:  streamingService.c	
 * RqID:  
 * Brief: Streaming Service as part of Trace Handler for target interface
 * $Id: 291b781f734f6156db1bc3f7e6f0e6d9735a0ee9 $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-April-30:
 *     - Initial version
 *
 */
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include "targetInterface.h"
#include "streamingService.h"
#include "Clockdriver.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#define TRCH_SERVICE_STREAMING_MAX_CONFIGURATIONS (2)

/**********************************************************************************************************************
* LOCAL ENUMS
**********************************************************************************************************************/
typedef enum{
	TRCH_response_size_Streaming_GetConfiguration = 8,
	TRCH_response_size_Streaming_Configure = 8,
	TRCH_response_size_Streaming_MapChannels = 7,
	TRCH_response_size_Streaming_ERROR = 6,
} TRCH_response_size_Streaming_e;

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
typedef struct {
	union {
		uint32_t configuration[TRCH_SERVICE_STREAMING_MAX_CONFIGURATIONS];
		struct 
		{
			/* Get Version */
			uint16_t version;
			uint16_t reserved;
			/* Get Capability */
			uint8_t maxCh;
			uint8_t reserved1;
			uint8_t maxSrc;
			uint8_t reserved3;
		};
	};
	uint8_t numCh;
	uint8_t resolution;
	uint8_t clkdiv;
} TRCH_Streaming_ct;

/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
const TRCH_Streaming_ct TRCH_Streaming_cnf = {
	{.version = DTI_VERSION(0,0,1),
	.maxCh = DTI_STREAM_CHANNEL_MAX, /* Maximum number of straming channels */
	.maxSrc = TRCH_SERVICE_CLOCKSOURCE_MAX}, /* Maximum number of clock sources */

	/* default values*/
	.numCh = 4, /* Number of streaming channels */
	.resolution = 16, /* number of bits per channel */
	.clkdiv = 4 /* clock divider */
};

TRCH_ClockCallback_t TRCH_StreamingService_ClockCallback[TRCH_SERVICE_CLOCKSOURCE_MAX] = {
	NULL

#if (TRCH_SERVICE_CLOCKSOURCE_MAX > 1)
	, NULL
#endif

#if (TRCH_SERVICE_CLOCKSOURCE_MAX > 2)
	, NULL
#endif
	
#if (TRCH_SERVICE_CLOCKSOURCE_MAX > 3)
	, NULL
#endif
};

#if (TRCH_SERVICE_CLOCKSOURCE_MAX > 4)
#error TRCH_SERVICE_SAMPLING_CLOCKSOURCE_MAX is greater than 4. Only a maximum number of 4 sample clocks are supported.
#endif
/***********************************************************************************************************************
 * LOCAL ROUTINES
 **********************************************************************************************************************/

static TRCH_ERRORCODES_e TRCH_StreamingService_GetConfiguration(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e TRCH_StreamingService_Configure(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e TRCH_StreamingService_MapChannels(TRCH_buffer_t*,TRCH_buffer_t*);

/* Called by Streaming Service */
static inline void TRCH_StreamingService_ClockCallback_Register(
	uint32_t source, 
	TRCH_ClockCallback_t callback
)
{
	TRCH_StreamingService_ClockCallback[source] = callback;
}

/* Called by Streaming Service */
static inline void TRCH_StreamingService_ClockCallback_Unregister(
	uint32_t source
)
{
	TRCH_StreamingService_ClockCallback[source] = NULL;
}

/* Called by Streaming Service */
static inline int32_t TRCH_StreamingService_GetClockSource(void)
{
	int32_t clksrc = -1;
	for( uint32_t loop = 0; loop < TRCH_SERVICE_CLOCKSOURCE_MAX; ++loop )
	{
		if( TRCH_StreamingService_ClockCallback[loop] )
		{
			clksrc = loop;
		}
	}
	return clksrc;
}

/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
extern void DTI_interface_ProcessStreaming(void);
 
TRCH_ERRORCODES_e (*TRCH_StreamingCmd[])(TRCH_buffer_t*,TRCH_buffer_t*) = 
{
	TRCH_StreamingService_GetConfiguration,
	TRCH_StreamingService_GetConfiguration,
	TRCH_StreamingService_Configure,
	TRCH_StreamingService_MapChannels
};


TRCH_ERRORCODES_e TRCH_StreamingService(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	if( request->cmd < (sizeof(TRCH_StreamingCmd) / sizeof(TRCH_ERRORCODES_e(*)(TRCH_buffer_t*,TRCH_buffer_t*))) )
	{
		errorcode = TRCH_StreamingCmd[request->cmd](request, response);
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

TRCH_ERRORCODES_e TRCH_StreamingService_init(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	DTI_setMD_DU(DTI_router_withStreaming);
	DTI_sendSyncFrame_DU();
	DTI_setNumberChannels_DU( TRCH_Streaming_cnf.numCh, TRCH_Streaming_cnf.resolution );
	DTI_setClockDivider_DU( TRCH_Streaming_cnf.clkdiv ); /* execute DTI in clock source */
	TRCH_StreamingService_ClockCallback_Register( 0, DTI_interface_ProcessStreaming);

	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e TRCH_StreamingService_deinit(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	DTI_setNumberChannels_DU( TRCH_Streaming_cnf.maxCh, 16 );
	DTI_setClockDivider_DU( 0 ); /* execute DTI in background */
	DTI_setMD_DU(DTI_router_noStreaming);
	DTI_router_setChannelPtr_TX( DTI_channel0, NULL, TRCH_SignalType_16bit); /* stop the stream */
	for( uint32_t loop = 0; loop < TRCH_Streaming_cnf.maxSrc; ++loop)
	{
		TRCH_StreamingService_ClockCallback_Unregister(loop);
	}

	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e TRCH_StreamingService_GetConfiguration(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	response->cmd = request->cmd;
	response->channel = 0; 
	response->data32[0] = TRCH_Streaming_cnf.configuration[request->cmd];
	response->size = TRCH_response_size_Streaming_GetConfiguration;
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e TRCH_StreamingService_Configure(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	response->data32[0]=0; /* clear response data */
	response->cmd = request->cmd;
	response->channel = 0; 
	
/* Number of Channels (numCh)*/
	if( (request->data8[0] <= TRCH_Streaming_cnf.maxCh) || ((request->data8[2] == 32U) && (request->data8[0] <= TRCH_Streaming_cnf.maxCh/2)) )
	{
		if( request->data8[0] )
		{
			response->data8[0] = request->data8[0];
			DTI_setNumberChannels_DU( request->data8[0], DTI_router_getStreamResolution() );
		}
		else
		{ /* return current setting */
			response->data8[0] = DTI_router_getNumberChannels();
		}
		response->size = TRCH_response_size_Streaming_Configure;
		errorcode = TRCH_ERRORCODES_Success;
	}
	else
	{
		response->channel = 0x0;
		errorcode = TRCH_ERRORCODES_Streaming_MaxNumberChannelsExceeded;
	}
	
/* Clock Divider (clkdiv) */
	if( request->data8[1] )
	{
		response->data8[1] = request->data8[1];
		DTI_setClockDivider_DU( request->data8[1] );
		
		/* select clock source and re-register callback */
		if( request->data8[3] < TRCH_Streaming_cnf.maxSrc )
		{
			response->data8[3] = request->data8[3];
			for( uint32_t loop = 0; loop < TRCH_Streaming_cnf.maxSrc; ++loop)
			{
				TRCH_StreamingService_ClockCallback_Unregister(loop);
			}
			TRCH_StreamingService_ClockCallback_Register(request->data8[3], DTI_interface_ProcessStreaming );
		}
		else
		{
			response->channel = 0x0f;
			errorcode = TRCH_ERRORCODES_Streaming_MaxNumberClockSourcesExceeded;
		}		
	}
	else
	{ /* return current setting */
		response->data8[1] = DTI_getClockDivider_DU();
		response->data8[3] = TRCH_StreamingService_GetClockSource();
	}

/* Number of bits per channel (resolution) */
	if( (request->data8[2] == 0U) | (request->data8[2] == 8U) | (request->data8[2] == 12U) | (request->data8[2] == 16U) | (request->data8[2] == 32U))
	{
		if( request->data8[2] )
		{
			response->data8[2] = request->data8[2];
			DTI_setNumberChannels_DU( DTI_router_getNumberChannels(),  request->data8[2] );
		}
		else
		{ /* return current setting */
			response->data8[2] = DTI_router_getStreamResolution() ;
		}
	}
	else
	{
		response->channel = 0x0f;
		errorcode = TRCH_ERRORCODES_Streaming_ResolutionNotSupported;
	}
	
	return errorcode;
}

TRCH_ERRORCODES_e TRCH_StreamingService_MapChannels(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	DTI_stream_channel_t *ptr;
	TRCH_SignalType_t type;
	
	response->cmd = request->cmd;
	if( request->channel < TRCH_Streaming_cnf.maxCh )
	{
		response->channel = request->channel;
		
		ptr = (DTI_stream_channel_t*) TRCH_getSignalptr( request->data32[0] );
		type = TRCH_getSignalType( request->data32[0] );
		if( ptr )
		{
			DTI_router_setChannelPtr_TX( (DTI_channels_e) request->channel, ptr, type);
			response->data32[0] = request->data32[0];
			response->size = TRCH_response_size_Streaming_MapChannels;
			errorcode = TRCH_ERRORCODES_Success;
		}
		else
		{
			response->channel = 0x0f;
			errorcode = TRCH_ERRORCODES_Streaming_SignalNotAvailable;
		}
	}
	else
	{
		response->channel = request->channel;
		errorcode = TRCH_ERRORCODES_Streaming_ChannelNotAvailable;
	}
	
	return errorcode;
}
