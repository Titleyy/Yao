/**********************************************************************************************************************
 * File:  samplingService.c	
 * RqID:  
 * Brief: Sampling Service as part of Trace Handler for target interface
 * $Id: 26550533720578edf1f2b8a28043fd568683385e $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-15:
 *     - Initial version
 *
 */
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include "signalHandler_api.h"
#include "Clockdriver.h"
#include "targetInterface.h"
#include "samplingService.h"
#include "eventService.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#define TRCH_SERVICE_SAMPLING_MAX_CONFIGURATIONS (2)
#define TRCH_SERVICE_SAMPLING_TIMEOUT_TRIGGERMASK (0x80000000)

/**********************************************************************************************************************
* LOCAL ENUMS
**********************************************************************************************************************/
typedef enum{
	TRCH_response_size_Sampling_GetConfiguration = 8,
	TRCH_response_size_Sampling_Configure = 8,
	TRCH_response_size_Sampling_MapChannels = 7,
	TRCH_response_size_Sampling_Control = 8,
	TRCH_response_size_Sampling_GetClockSourceScaling = 8,
	TRCH_response_size_Sampling_HeaderBlock = 20,
	TRCH_response_size_Sampling_DataBlock = 20,
	TRCH_response_size_Sampling_ERROR = 6,
} TRCH_response_size_Sampling_e;

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
typedef struct {
	union {
		uint32_t configuration[TRCH_SERVICE_SAMPLING_MAX_CONFIGURATIONS];
		struct 
		{
			/* Get Version */
			uint16_t version;
			uint16_t reserved;
			/* Get Capability */
			uint8_t maxCh;
			uint8_t maxBlk; /* maximum number of data blocks */
			uint8_t maxSrc;
			uint8_t maxTrgmode;
		};
	};
	uint8_t numCh;
	uint8_t numBlk;
	uint8_t trgpos;
	uint8_t trgmode;
	uint8_t clksource;
	uint8_t clkdiv;
	uint8_t trgev;
	uint8_t timeout;
	
} TRCH_Sampling_ct;


typedef struct {
	DTI_stream_channel_t *buffer;
	DTI_stream_channel_t* buffer_end;
	DTI_stream_channel_t* buffer_ptr;
	volatile DTI_stream_channel_t* buffer_stop;
	uint32_t trigger;
	uint32_t triggerMask;
	int32_t triggerPos;
	uint32_t targettime;
	uint8_t timestampTS;
	uint8_t numCh;
	uint8_t numBlk;
	uint8_t trgpos;
	uint8_t trgmode;
	uint8_t clksource;
	volatile uint8_t clkdiv;
	uint8_t trgev;
	uint8_t timeout;
	uint8_t timeoutCounter;
	uint8_t requestAbort : 1;
	DTI_stream_channelmap_t channelmap[TRCH_SERVICE_SAMPLING_CHANNEL_MAX+1];
} TRCH_Sampling_vt;

/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
const TRCH_Sampling_ct TRCH_Sampling_cnf = {
	.version = (DTI_VERSION(0,0,2)),
	.maxBlk = TRCH_SERVICE_SAMPLING_DATABLOCK_MAX-1,
	.maxCh = TRCH_SERVICE_SAMPLING_CHANNEL_MAX, /* Maximum number of straming channels */
	.maxSrc = TRCH_SERVICE_CLOCKSOURCE_MAX,
	.maxTrgmode = 1,

	/* default values*/
	.numCh = 4,     /* number of channels */
	.numBlk = TRCH_SERVICE_SAMPLING_DATABLOCK_MAX-1,
	.trgpos = TRCH_SERVICE_SAMPLING_TRIGGERPOS_100 / 2UL, 
	.trgmode = 0,   /* trigger mode */
	.clksource = 0, /* clock source */
	.clkdiv = 1,    /* clock divider */
	.trgev = 0,     /* event channel for trigger */
	.timeout = 0   /* timeout 0: no timeout*/
};

DTI_stream_channel_t TRCH_SamplingBuffer[(TRCH_SERVICE_SAMPLING_DATABLOCK_MAX * TRCH_SERVICE_SAMPLING_SUBBLOCKSIZE / sizeof (DTI_stream_channel_t)) + 1]; /* allow overshoot of one entry */

TRCH_Sampling_vt TRCH_Sampling_var = {
	.buffer = TRCH_SamplingBuffer
};

TRCH_ClockCallback_t TRCH_SamplingService_ClockCallback[TRCH_SERVICE_CLOCKSOURCE_MAX] = {
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

static TRCH_ERRORCODES_e GetConfiguration(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e ConfigureService(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e MapChannels(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e SamplingControl(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e GetClockSourceScaling(TRCH_buffer_t*,TRCH_buffer_t*);
static void TRCH_SamplingService_InhibitSampling(void);

static void clearChannelMap(void)
{
	for(int loop = 0; loop < TRCH_SERVICE_SAMPLING_CHANNEL_MAX; ++loop)
	{
		TRCH_Sampling_var.channelmap[loop].ptr = NULL;
	}
}

static inline void TRCH_SamplingService_ClockCallback_Register(
	uint32_t source, 
	TRCH_ClockCallback_t callback
)
{
	TRCH_SamplingService_ClockCallback[source] = callback;
}

static inline void TRCH_SamplingService_ClockCallback_Unregister(
	uint32_t source
)
{
	TRCH_SamplingService_ClockCallback[source] = NULL;
}

/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
TRCH_ERRORCODES_e (*TRCH_SamplingCmd[])(TRCH_buffer_t*,TRCH_buffer_t*) = 
{
	GetConfiguration,
	GetConfiguration,
	ConfigureService,
	MapChannels,
	SamplingControl,
	GetClockSourceScaling
};


TRCH_ERRORCODES_e TRCH_SamplingService(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	if( request->cmd < (sizeof(TRCH_SamplingCmd) / sizeof(TRCH_ERRORCODES_e(*)(TRCH_buffer_t*,TRCH_buffer_t*))) )
	{
		errorcode = TRCH_SamplingCmd[request->cmd](request, response);
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

TRCH_ERRORCODES_e TRCH_SamplingService_init(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_Sampling_var.numCh     = TRCH_Sampling_cnf.numCh;
	TRCH_Sampling_var.numBlk    = TRCH_Sampling_cnf.maxBlk;
	TRCH_Sampling_var.trgpos    = TRCH_Sampling_cnf.trgpos;
	TRCH_Sampling_var.trgmode   = TRCH_Sampling_cnf.maxTrgmode;
	TRCH_Sampling_var.clksource = TRCH_Sampling_cnf.clksource;
	TRCH_Sampling_var.clkdiv    = TRCH_Sampling_cnf.clkdiv;
	TRCH_Sampling_var.trgev     = TRCH_Sampling_cnf.trgev;
	TRCH_Sampling_var.timeout   = TRCH_Sampling_cnf.timeout;
	clearChannelMap();
	TRCH_SamplingService_releaseBuffer();

	TRCH_EventService_Register_EvOut( &TRCH_Sampling_var.trigger );

	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e TRCH_SamplingService_deinit(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_EventService_Register_EvOut( NULL );

	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e GetConfiguration(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	response->cmd = request->cmd;
	response->channel = 0; 
	response->data32[0] = TRCH_Sampling_cnf.configuration[request->cmd];
	response->size = TRCH_response_size_Sampling_GetConfiguration;
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e ConfigureService(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	response->data32[0]=0; /* clear response data */
	response->cmd = request->cmd;
	response->channel = 0;
	errorcode = TRCH_ERRORCODES_UnknownError;
	
	/* number of channels */
	if( request->data8[0] <= TRCH_Sampling_cnf.maxCh)
	{
		if( request->data8[0] )
		{ 
			/* number of channels */
			clearChannelMap();
			TRCH_Sampling_var.numCh = request->data8[0];
			response->data8[0] = request->data8[0];
			
			/* configure Number of Buffer Subblocks - 1 (N) */
			if( request->data8[1] <= TRCH_Sampling_cnf.maxBlk )
			{
				TRCH_Sampling_var.numBlk = request->data8[1];
				response->data8[1] = request->data8[1];
			}
			else
			{
				response->channel = 0x0;
				errorcode = TRCH_ERRORCODES_Sampling_MaxBuffersizeExceeded;
			}
			
			/* trigger position */
			TRCH_Sampling_var.trgpos = request->data8[2];
			response->data8[2] = request->data8[2];
			
			if( request->data8[3] <= TRCH_Sampling_cnf.maxTrgmode )
			{/* configure trigger mode */
				TRCH_Sampling_var.trgmode = request->data8[3];
				response->data8[3] = request->data8[3];
			}
			else
			{
				errorcode = TRCH_ERRORCODES_Sampling_TriggermodeNotSupported;
			}
		}
		else
		{ /* return current setting */
			response->data8[0] = TRCH_Sampling_var.numCh;
			response->data8[1] = TRCH_Sampling_var.numBlk;
			response->data8[2] = TRCH_Sampling_var.trgpos;
			response->data8[3] = TRCH_Sampling_var.trgmode;
		}
		
		if( errorcode == TRCH_ERRORCODES_UnknownError )
		{
			response->size = TRCH_response_size_Sampling_Configure;
			errorcode = TRCH_ERRORCODES_Success;
		}
	}
	else
	{
		response->channel = 0x0;
		errorcode = TRCH_ERRORCODES_Sampling_MaxNumberChannelsExceeded;
	}

	if( TRCH_ERRORCODES_Success == errorcode )
	{
		TRCH_Sampling_var.triggerPos = 
			TRCH_Sampling_var.numBlk * (TRCH_SERVICE_SAMPLING_SUBBLOCKSIZE/sizeof(DTI_stream_channel_t))
			-
			((
				TRCH_Sampling_var.numBlk * ((TRCH_SERVICE_SAMPLING_SUBBLOCKSIZE/sizeof(DTI_stream_channel_t)) * TRCH_Sampling_var.trgpos) / TRCH_SERVICE_SAMPLING_TRIGGERPOS_100 
			) / TRCH_Sampling_var.numCh + 2) * TRCH_Sampling_var.numCh
		;
	
		if( TRCH_Sampling_var.triggerPos < 0 )
		{
			TRCH_Sampling_var.triggerPos = 0;
		}
		else {/* do nothing */}
	}
	
	return errorcode;
}

TRCH_ERRORCODES_e MapChannels(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	DTI_stream_channel_t *ptr;

	response->cmd = request->cmd;
	if( request->channel < TRCH_Sampling_var.numCh )
	{
		response->channel = request->channel;
		ptr = (DTI_stream_channel_t*) TRCH_getSignalptr( request->data32[0] );
		if( ptr )
		{
			TRCH_Sampling_var.channelmap[request->channel].ptr = ptr;
			TRCH_Sampling_var.channelmap[request->channel].type = TRCH_getSignalType( request->data32[0] );
			response->data32[0] = request->data32[0];
			response->size = TRCH_response_size_Sampling_MapChannels;
			errorcode = TRCH_ERRORCODES_Success;
		}
		else
		{
			response->channel = 0x0f;
			errorcode = TRCH_ERRORCODES_Sampling_SignalNotAvailable;
		}
	}
	else
	{
		response->channel = request->channel;
		errorcode = TRCH_ERRORCODES_Sampling_ChannelNotAvailable;
	}
	
	return errorcode;
}

TRCH_ERRORCODES_e SamplingControl(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	response->data32[0]=0; /* clear response data */
	response->cmd = request->cmd;

	TRCH_SamplingService_InhibitSampling();
	
	if( request->subcmd == 0 )
	{ /* subcommand: SET */
		if( request->start == 1 )
		{ /* Start Sampling */
			/* select clock source */
			if( request->data8[0] < TRCH_Sampling_cnf.maxSrc )
			{
				TRCH_Sampling_var.clksource  = request->data8[0];
				if( request->data8[1] )
				{
					TRCH_Sampling_var.clkdiv   = request->data8[1];
				}
				else
				{
					TRCH_Sampling_var.clkdiv   = TRCH_Sampling_cnf.clkdiv;
				}
				TRCH_Sampling_var.trgev      = request->data8[2];
				TRCH_Sampling_var.timeout    = request->data8[3];
				TRCH_Sampling_var.buffer_ptr = TRCH_Sampling_var.buffer;
				TRCH_Sampling_var.buffer_end = 
					TRCH_Sampling_var.buffer 
						+ (TRCH_Sampling_var.numBlk * TRCH_SERVICE_SAMPLING_SUBBLOCKSIZE / sizeof (DTI_stream_channel_t));
				response->start = 1;
				response->data32[0] = request->data32[0];
				response->size = TRCH_response_size_Sampling_Control;
				errorcode = TRCH_ERRORCODES_Success;

				TRCH_Sampling_var.trigger = 0; /* clear pending trigger */
				TRCH_Sampling_var.timeoutCounter = TRCH_Sampling_var.timeout; /* load timeout counter */ 
				if( TRCH_Sampling_var.timeout )
				{
					TRCH_Sampling_var.triggerMask = TRCH_SERVICE_SAMPLING_TIMEOUT_TRIGGERMASK; /* activate timeout trigger */
				}
				else 
				{
					TRCH_Sampling_var.triggerMask = 0; /* clear all triggers */
				}
				TRCH_Sampling_var.triggerMask |= TRCH_Sampling_var.trgev;
				
				/* start processing the sampling, if clock divider is not 0 */
				if(TRCH_Sampling_var.clkdiv)
				{
					TRCH_SamplingService_ClockCallback_Register( TRCH_Sampling_var.clksource, TRCH_SamplingService_ProcessSampling);
				}
				else {/* do nothing */}
			}
			else
			{
				response->start = 0x0;
				TRCH_Sampling_var.clkdiv    = 0;
				errorcode = TRCH_ERRORCODES_Sampling_MaxNumberClockSourcesExceeded;
			}
		}
		else
		{ /* Stop Sampling */
				TRCH_Sampling_var.clkdiv  = 0;               /* stop processing the sampling */
				TRCH_Sampling_var.trigger = 0;               /* clear pending trigger */
				TRCH_Sampling_var.timeoutCounter = 0;        /* clear timeout counter */ 

				TRCH_SamplingService_ClockCallback_Unregister( TRCH_Sampling_var.clksource );
				errorcode = TRCH_ERRORCODES_Success;
		}
	}
	else
	{ /* subcommand: GET */
		response->data8[0] = TRCH_Sampling_var.clksource;
		response->data8[1] = TRCH_Sampling_var.clkdiv;
		response->data8[2] = TRCH_Sampling_var.trgev;
		response->data8[3] = TRCH_Sampling_var.timeout;
	}

	/* abort a running sampling in any case */
	TRCH_Sampling_var.requestAbort = true;
	
	return errorcode;
}

TRCH_ERRORCODES_e GetClockSourceScaling(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 

	response->cmd = request->cmd;
	response->clksrc = request->clksrc;
	
	if( request->clksrc < TRCH_Sampling_cnf.maxSrc )
	{
		response->data16[0] = TRCH_SamplingService_getClockScale(request->clksrc);
		response->data16[1] = TRCH_SamplingService_getTimerScale(request->clksrc);
		response->size = TRCH_response_size_Sampling_GetClockSourceScaling;
		errorcode = TRCH_ERRORCODES_Success;
	}
	else
	{
		errorcode = TRCH_ERRORCODES_Sampling_MaxNumberClockSourcesExceeded;
	}
	
	return errorcode;
}

void TRCH_SamplingService_ProcessSampling(void)
{
	DTI_stream_channelmap_t* channelmap;
	static int32_t counter = 1;
	
	if( --counter > 0 )
	{/*do nothing*/	}
	else
	{ /* clock divider counter elapsed */
		counter = TRCH_Sampling_var.clkdiv;
		if( TRCH_Sampling_var.buffer_ptr != TRCH_Sampling_var.buffer_stop )
		{
			channelmap = TRCH_Sampling_var.channelmap;
			while( channelmap->ptr  && (TRCH_Sampling_var.buffer_ptr != TRCH_Sampling_var.buffer_stop))
			{
				switch( channelmap->type )
				{
					/* Output signals */
					case TRCH_SignalType_8bit: /* use 8bit channel pointer */
						*TRCH_Sampling_var.buffer_ptr = *(uint8_t*)channelmap->ptr;
						break;
					
					case TRCH_SignalType_16bit: /* use 16bit channel pointer */
						*TRCH_Sampling_var.buffer_ptr = *(uint16_t*)channelmap->ptr;
						break;
					
					case TRCH_SignalType_16bitH: /* use high word of 32bit channel pointer */
						*TRCH_Sampling_var.buffer_ptr = (*(uint32_t*)channelmap->ptr)>>16;
						break;
					
					case TRCH_SignalType_32bit: 						//ARNO: this may shoot beyond the buffer limit!!
						*TRCH_Sampling_var.buffer_ptr++ =  (*(uint32_t*)channelmap->ptr) & 0x0000ffff;
						*TRCH_Sampling_var.buffer_ptr = (*(uint32_t*)channelmap->ptr) >> 16;
						break;
					
					/* Input signals */
					case TRCH_SignalType_Input8bit: /* use 8bit channel pointer */
						*TRCH_Sampling_var.buffer_ptr = **(uint8_t**)channelmap->ptr;
						break;
					
					case TRCH_SignalType_Input16bit: /* use 16bit channel pointer */
						*TRCH_Sampling_var.buffer_ptr = **(uint16_t**)channelmap->ptr;
						break;
					
					case TRCH_SignalType_Input16bitH: /* use high word of 32bit channel pointer */
						*TRCH_Sampling_var.buffer_ptr = (**(uint32_t**)channelmap->ptr)>>16;
						break;
					
					case TRCH_SignalType_Input32bit: 						//ARNO: this may shoot beyond the buffer limit!!
						*TRCH_Sampling_var.buffer_ptr++ =  (**(uint32_t**)channelmap->ptr) & 0x0000ffff;
						*TRCH_Sampling_var.buffer_ptr = (**(uint32_t**)channelmap->ptr) >> 16;
						break;
					
					/* API signals */
					case TRCH_SignalType_API8bit: /* use 8bit channel pointer */
						*TRCH_Sampling_var.buffer_ptr = ((TRCH_getSignalApi_ptr_t)channelmap->ptr)();
						break;
					
					case TRCH_SignalType_API16bit: /* use 16bit channel pointer */
						*TRCH_Sampling_var.buffer_ptr = ((TRCH_getSignalApi_ptr_t)channelmap->ptr)();
						break;
					
					case TRCH_SignalType_API16bitH: /* use high word of 32bit channel pointer */
						*TRCH_Sampling_var.buffer_ptr = (((TRCH_getSignalApi_ptr_t)channelmap->ptr)())>>16;
						break;
					
					case TRCH_SignalType_API32bit: 						//ARNO: this may shoot beyond the buffer limit!!
						*TRCH_Sampling_var.buffer_ptr++ =  (((TRCH_getSignalApi_ptr_t)channelmap->ptr)()) & 0x0000ffff;
						*TRCH_Sampling_var.buffer_ptr = (((TRCH_getSignalApi_ptr_t)channelmap->ptr)()) >> 16;
						break;
					
					default:
						break;
				}
//				*TRCH_Sampling_var.buffer_ptr = (uint16_t)TRCH_Sampling_var.buffer_ptr; /* test pointers */
				++TRCH_Sampling_var.buffer_ptr;
				++channelmap;
			
				if( TRCH_Sampling_var.buffer_ptr >= TRCH_Sampling_var.buffer_end )
				{
					if( ((int32_t)TRCH_Sampling_var.buffer_stop) < 0)
					{ /* ensure the sample buffer is filled before the trigger arrives and delete pending trigger */
						TRCH_Sampling_var.buffer_stop = (void *) ((int32_t)(TRCH_Sampling_var.buffer_stop) + 1);
						TRCH_Sampling_var.trigger = 0;
					} 
					else
					{
						if( TRCH_Sampling_var.timeout )
						{ /* timeout is not disabled */
							--TRCH_Sampling_var.timeoutCounter;
							if( !TRCH_Sampling_var.timeoutCounter )
							{ /* timeout time is elapsed */
								TRCH_Sampling_var.trigger |= TRCH_SERVICE_SAMPLING_TIMEOUT_TRIGGERMASK;     /* generate timeout trigger */
							}
							else {/* do nothing */}
						}
						else {/* do nothing */}
					}
					/* prepare the next round */
					TRCH_Sampling_var.buffer_ptr = TRCH_Sampling_var.buffer;
				}
				else {/* do nothing */}
			}
			
			if( (!TRCH_Sampling_var.buffer_stop) && (TRCH_Sampling_var.trigger & TRCH_Sampling_var.triggerMask) )
			{ /* this could be a callback from the event */
				TRCH_Sampling_var.timestampTS = DTI_TSdriver_getTimeStamp();
				TRCH_Sampling_var.targettime = DTI_TSdriver_getTargetTime();

				TRCH_Sampling_var.trigger = 0;
				TRCH_Sampling_var.buffer_stop = TRCH_Sampling_var.buffer_ptr + TRCH_Sampling_var.triggerPos;
				if( TRCH_Sampling_var.buffer_stop >= TRCH_Sampling_var.buffer_end )
				{
					TRCH_Sampling_var.buffer_stop -= (TRCH_Sampling_var.buffer_end - TRCH_Sampling_var.buffer);
				} 
				else
				{
//					if( TRCH_Sampling_var.buffer_stop < TRCH_Sampling_var.buffer )
//					{
//						TRCH_Sampling_var.buffer_stop += (TRCH_Sampling_var.buffer_end - TRCH_Sampling_var.buffer);
//					}
//					else {/* do nothing */}
				}
			} else {/* do nothing */}
		} 
		else
		{
			/* remove the callback from the caller */
			TRCH_SamplingService_ClockCallback_Unregister( TRCH_Sampling_var.clksource );			
			/* mask all trigger values */
			TRCH_Sampling_var.triggerMask = 0;
		}
	}
}

TRCH_ERRORCODES_e TRCH_SamplingService_getBufferEntry( TRCH_buffer_t* response, uint8_t subblock)
{
	TRCH_ERRORCODES_e errorcode; 
	DTI_stream_channel_t* buffer_ptr;
	uint32_t loop;

	if( (TRCH_Sampling_var.requestAbort && subblock) )
	{
		errorcode = TRCH_ERRORCODES_Aborted;
	}
	else
	{
		response->service = 0x04;
		response->cmd = TRCH_SERVICE_RESPONSE_ASYNC;
		response->subblock = subblock;
		
		if( !subblock )
		{ /* header block */
			response->data8[0] = TRCH_Sampling_var.numBlk;
			response->data8[1] = TRCH_Sampling_var.trgpos;
			response->data8[2] = TRCH_Sampling_var.numCh;
			response->data8[3] = TRCH_Sampling_var.trgev;
			response->data8[4] = TRCH_Sampling_var.clksource;
			response->data8[5] = TRCH_Sampling_var.clkdiv;
			response->data8[6] = TRCH_Sampling_var.trgmode;
			response->data8[7] = TRCH_Sampling_var.timestampTS;
			response->data32[2] = TRCH_Sampling_var.targettime;
			response->data16[6] = TRCH_SamplingService_getClockScale(TRCH_Sampling_var.clksource);
			response->data16[7] = TRCH_SamplingService_getTimerScale(TRCH_Sampling_var.clksource);

			TRCH_Sampling_var.requestAbort=false; /* in this case, the abort request can be cleared */
			response->size = TRCH_response_size_Sampling_HeaderBlock;
			errorcode = TRCH_ERRORCODES_Success;
		}
		else
		{ /* data block */
			if( subblock > TRCH_Sampling_var.numBlk )
			{
				response->cmd = TRCH_SERVICE_RESPONSE_ERROR;
				response->data16[0] = TRCH_ERRORCODES_Sampling_MaxBuffersizeExceeded;
				response->size = TRCH_response_size_Sampling_ERROR;
				errorcode = TRCH_ERRORCODES_Sampling_MaxBuffersizeExceeded;
			}
			else
			{
				buffer_ptr = (DTI_stream_channel_t*) TRCH_Sampling_var.buffer_stop + ((subblock - 1 - TRCH_Sampling_var.numBlk) * ((TRCH_SERVICE_SAMPLING_SUBBLOCKSIZE)/sizeof(DTI_stream_channel_t)));
				if( buffer_ptr >= TRCH_Sampling_var.buffer_end )
				{
					buffer_ptr -= TRCH_Sampling_var.buffer_end - TRCH_Sampling_var.buffer;
				} 
				else 
				{
					if( buffer_ptr < TRCH_Sampling_var.buffer )
					{
						buffer_ptr += TRCH_Sampling_var.buffer_end - TRCH_Sampling_var.buffer;
					} else {/* do nothing */}
				}

				if( buffer_ptr > TRCH_Sampling_var.buffer && buffer_ptr < TRCH_Sampling_var.buffer_end )
				{ /* make sure, the buffer_ptr is pointing into the buffer */
					for(loop = 0; loop < 8; ++loop )
					{
		//				response->data16[loop] = (uint16_t)(buffer_ptr++); /* test buffer pointers */
						response->data16[loop] = *(buffer_ptr++);
						
						if( buffer_ptr == TRCH_Sampling_var.buffer_end )
						{
							buffer_ptr = TRCH_Sampling_var.buffer;
						} else {/* do nothing */}
					}
					response->size = TRCH_response_size_Sampling_DataBlock;
					errorcode = TRCH_ERRORCODES_Success;
				}
				else
				{
					errorcode = TRCH_ERRORCODES_Aborted;
				}
			}
		}
	}
	
	return errorcode;
}

void TRCH_SamplingService_ReleaseSampling(void)
{
	TRCH_SamplingService_releaseBuffer();

	/* Check if trigger mode is not single shot mode */
	if( TRCH_Sampling_var.trgmode && !TRCH_Sampling_var.requestAbort)
	{ /* continue with sampling */
		if( TRCH_Sampling_var.timeout )
		{
			TRCH_Sampling_var.triggerMask = TRCH_SERVICE_SAMPLING_TIMEOUT_TRIGGERMASK; /* activate timeout trigger */
		}
		else {/* do nothing */}
		TRCH_Sampling_var.triggerMask |= TRCH_Sampling_var.trgev;
		TRCH_Sampling_var.timeoutCounter = TRCH_Sampling_var.timeout;               /* reload timeout counter */ 
		TRCH_SamplingService_ClockCallback_Register( TRCH_Sampling_var.clksource, TRCH_SamplingService_ProcessSampling );
	}
	TRCH_Sampling_var.requestAbort = false;
}

void TRCH_SamplingService_InhibitSampling(void)
{
	/* remove the callback from the caller */
	TRCH_SamplingService_ClockCallback_Unregister( TRCH_Sampling_var.clksource );			
	/* mask all trigger values */
	TRCH_Sampling_var.triggerMask = 0;
}

uint32_t TRCH_SamplingService_isSampling(void)
{
	return TRCH_Sampling_var.triggerMask;
}

uint32_t TRCH_SamplingService_isReady(void)
{
	return 
		   (0 != TRCH_Sampling_var.buffer_stop) 
		&& (TRCH_Sampling_var.buffer_ptr == TRCH_Sampling_var.buffer_stop)
//		&& (!TRCH_Sampling_var.requestAbort)
		;
}

uint8_t TRCH_SamplingService_getNumBlocks(void)
{
	return TRCH_Sampling_var.numBlk;
}

void TRCH_SamplingService_releaseBuffer(void)
{
	TRCH_Sampling_var.trigger = 0;
//	TRCH_Sampling_var.buffer_stop = NULL;
	TRCH_Sampling_var.buffer_stop = (DTI_stream_channel_t*) -4L;
}
