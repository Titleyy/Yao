/**********************************************************************************************************************
 * File:  parameterService.c	
 * RqID:  
 * Brief: Parameter Service as part of Trace Handler for DTI
 * $Id: 3f6b8b27c20c3d3f1f50db9b4a607ee546112a4d $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2023-Feb-13:
 *     - Initial version
 *
 */
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include "parameterHandler_api.h"
#include "targetInterface.h"
#include "parameterService.h"


/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#define TRCH_SERVICE_PARAMETER_MAX_CONFIGURATIONS (2)
#define TRCH_SERVICE_PARAMETER_PENDING_MAX        (1 + TRCH_SERVICE_PARAMETER_CHANNEL_MAX/32)

/**********************************************************************************************************************
* LOCAL ENUMS
**********************************************************************************************************************/
typedef enum{
	TRCH_response_size_Parameter_GetConfiguration = 8,
	TRCH_response_size_Parameter_MapChannels = 7,
	TRCH_response_size_Parameter_Control = 8,
	TRCH_response_size_Parameter_GetBufferEntry = 12,
	TRCH_response_size_Parameter_ERROR = 6,
} TRCH_response_size_Sampling_e;

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
typedef struct {
	union {
		uint32_t configuration[TRCH_SERVICE_PARAMETER_MAX_CONFIGURATIONS];
		struct 
		{
			/* Get Version */
			uint16_t version;
			uint16_t reserved;
			/* Get Capability */
			uint8_t maxCh;
			uint8_t reserved1;
			uint8_t reserved2;
			uint8_t reserved3;
		};
	};
} TRCH_Parameter_ct;

typedef struct {
	uint32_t* mapPtr; /* must be first member of this structure in order to match with NULL_mapPtr beyond last entry */
	uint16_t  period;
	uint16_t  offset;
	uint16_t  captureTime;
	uint8_t   size;
} TRCH_ParameterEntry_t;


typedef struct {
	TRCH_ParameterEntry_t channelEntry[TRCH_SERVICE_PARAMETER_CHANNEL_MAX];
	const void* NULL_mapPtr; /* end of list of channel entries */
	uint8_t start;
	uint8_t queuelevel;
	uint32_t valuequeue[TRCH_SERVICE_PARAMETER_QUEUE_MAX];
	uint32_t targettime; /* target time at point of capture */
	uint32_t pending[TRCH_SERVICE_PARAMETER_PENDING_MAX];    /* pending channel entry number for transmit */
} TRCH_Parameter_vt;

/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
const TRCH_Parameter_ct TRCH_Parameter_cnf = {
	.version = (DTI_VERSION(0,0,1)),
	.maxCh = TRCH_SERVICE_PARAMETER_CHANNEL_MAX, /* Maximum number of parameter channels */

	/* default values*/
};

TRCH_Parameter_vt TRCH_Parameter_var = {
	.start = 0,
	.queuelevel = 0,
	.pending = {0},
	.channelEntry[0].mapPtr = NULL,
	.NULL_mapPtr = NULL,
};


/***********************************************************************************************************************
 * LOCAL ROUTINES
 **********************************************************************************************************************/

static TRCH_ERRORCODES_e GetConfiguration(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e MapChannels(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e ParameterControl(TRCH_buffer_t*,TRCH_buffer_t*);

static void clearChannelMap(void)
{
	for(int loop = 0; loop < TRCH_SERVICE_PARAMETER_CHANNEL_MAX; ++loop)
	{
		TRCH_Parameter_var.channelEntry[loop].mapPtr = NULL;
	}
}

static uint8_t getChannelMapLevel(void)
{
	uint32_t level ;

	for(level = 0; level < TRCH_SERVICE_PARAMETER_CHANNEL_MAX; ++level)
	{
		if(!TRCH_Parameter_var.channelEntry[level].mapPtr)
		{
			break;
		}
	}
	
	return level;
}

/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
uint32_t TRCH_ParameterService_isBufferIdle(void)
{
	uint32_t pending;
	pending = 0;
	
	for( uint32_t i = 0; i < TRCH_SERVICE_PARAMETER_PENDING_MAX; ++i)
	{
		pending |= TRCH_Parameter_var.pending[i];
	}
	return pending == 0;
}

TRCH_ERRORCODES_e TRCH_ParameterService_getBufferEntry(TRCH_buffer_t* response)
{ /* this takes the last value from the list and decrements the fill level */
	uint32_t pending;
	uint32_t channel;
	
	pending = TRCH_Parameter_var.pending[TRCH_SERVICE_PARAMETER_PENDING_MAX - 1];
	channel = TRCH_SERVICE_PARAMETER_CHANNEL_MAX - 1;

	while( !(pending & 0x80000000) )
	{
		pending<<=1;
		--channel;
		if( (channel & 0x1f) == 0x1f )
		{
			pending = TRCH_Parameter_var.pending[ channel >> 5 ];
		}
	}
	
	if( pending )
	{
		TRCH_Parameter_var.pending[ channel >> 5 ] &= ~(1<<(channel & 0x1f));
		--TRCH_Parameter_var.queuelevel;

		response->channel = channel;
		response->data32[0] = TRCH_Parameter_var.targettime;
		response->data32[1] = TRCH_Parameter_var.valuequeue[TRCH_Parameter_var.queuelevel];
	}
	else
	{ /* ERROR, no pending entry in buffer */
		response->channel  = 0xFF;
		response->data32[0] = 0;
		response->data32[1] = 0;
	}	
	response->size = TRCH_response_size_Parameter_GetBufferEntry;
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e (*TRCH_ParameterCmd[])(TRCH_buffer_t*,TRCH_buffer_t*) = 
{
	GetConfiguration,
	GetConfiguration,
	MapChannels,
	ParameterControl
};


TRCH_ERRORCODES_e TRCH_ParameterService(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	if( request->cmd < (sizeof(TRCH_ParameterCmd) / sizeof(TRCH_ERRORCODES_e(*)(TRCH_buffer_t*,TRCH_buffer_t*))) )
	{
		errorcode = TRCH_ParameterCmd[request->cmd](request, response);
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

TRCH_ERRORCODES_e TRCH_ParameterService_init(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	clearChannelMap();
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e TRCH_ParameterService_deinit(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	clearChannelMap();
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e GetConfiguration(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	response->cmd = request->cmd;
	response->channel = 0; 
	response->data32[0] = TRCH_Parameter_cnf.configuration[request->cmd];
	response->size = TRCH_response_size_Parameter_GetConfiguration;
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e MapChannels(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	uint32_t* ptr;

	response->cmd = request->cmd;
	if( request->channel < TRCH_Parameter_cnf.maxCh )
	{
		response->channel = request->channel;
		PARH_paramAddress(request->data32[0], (void*) ptr);
		if( ptr )
		{
			TRCH_Parameter_var.channelEntry[request->channel].mapPtr = ptr;
			TRCH_Parameter_var.channelEntry[request->channel].size = PARH_paramMaxNumberBytes(request->data32[0] & 0x00ffffff );
			TRCH_Parameter_var.channelEntry[request->channel].period = request->data8[3] + (((uint16_t)request->data8[4])<<8);
			if( !TRCH_Parameter_var.channelEntry[request->channel].period )
			{ /* a period value of 0 is changed to 1 automatically */
				TRCH_Parameter_var.channelEntry[request->channel].period = 1;
			}
			TRCH_Parameter_var.channelEntry[request->channel].offset = request->data8[5] + (((uint16_t)request->data8[6])<<8);
			response->data32[0] = request->data32[0];
			response->size = TRCH_response_size_Parameter_MapChannels;
			errorcode = TRCH_ERRORCODES_Success;
		}
		else
		{
			response->channel = request->channel;
			errorcode = TRCH_ERRORCODES_Parameter_ParameterNotAvailable;
		}
	}
	else
	{
		response->channel = request->channel;
		errorcode = TRCH_ERRORCODES_Parameter_ChannelNotAvailable;
	}
	
	return errorcode;
}

TRCH_ERRORCODES_e ParameterControl(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ParameterEntry_t* parameter_ptr;
	TRCH_ERRORCODES_e errorcode; 
	
	parameter_ptr = TRCH_Parameter_var.channelEntry;

	response->data32[0]=0; /* clear response data */
	response->cmd = request->cmd;
	response->subcmd = 0;
	response->start = request->start;

	if( request->subcmd == 0 )
	{ /* subcommand: SET */
		if( request->start == 1 )
		{ 
			/* update target time */
			TRCH_Parameter_var.targettime = getRuntimeCounter();
			/* set offset values */
			while(parameter_ptr->mapPtr)
			{
				parameter_ptr->captureTime = TRCH_Parameter_var.targettime + parameter_ptr->offset;
			}
			/* Start Processing */
			TRCH_Parameter_var.start = 1;
		}
		else
		{ /* Stop Processing */
			TRCH_Parameter_var.start = 0;
		}
		if( response->data8[0] )
		{
			clearChannelMap();
		}
		errorcode = TRCH_ERRORCODES_Success;
	}
	else
	{ /* subcommand: GET */
		response->data8[0] = getChannelMapLevel();
	}

	return errorcode;
}

void TRCH_ParameterService_Process(void)
{
	TRCH_ParameterEntry_t* parameter_ptr;
	uint32_t channel;

	parameter_ptr = TRCH_Parameter_var.channelEntry;

	if( TRCH_Parameter_var.start && TRCH_ParameterService_isBufferIdle() )
	{
		/* no captured values are pending, update target time */
		TRCH_Parameter_var.targettime = getRuntimeCounter();
		channel = 0;
		
		/* dispatch and capture channel */
		while(parameter_ptr->mapPtr)
		{
			if( ((int16_t) (TRCH_Parameter_var.targettime - ((uint32_t) parameter_ptr->captureTime))) > 0 )
			{
				/* prepare for next capture */
				parameter_ptr->captureTime = TRCH_Parameter_var.targettime + parameter_ptr->period - 1; 
				/* capture value in queue and set pending flag */
				switch( parameter_ptr->size )
				{
					case 1: 
						TRCH_Parameter_var.valuequeue[TRCH_Parameter_var.queuelevel++] = (uint32_t)*((uint8_t*)parameter_ptr->mapPtr);
						TRCH_Parameter_var.pending[channel >> 5] |= 1<<(channel & 0x1f);
						break;
					case 2: 
						TRCH_Parameter_var.valuequeue[TRCH_Parameter_var.queuelevel++] = (uint32_t)*((uint16_t*)parameter_ptr->mapPtr);
						TRCH_Parameter_var.pending[channel >> 5] |= 1<<(channel & 0x1f);
						break;
					case 4: 
						TRCH_Parameter_var.valuequeue[TRCH_Parameter_var.queuelevel++] = (uint32_t)*((uint32_t*)parameter_ptr->mapPtr);
						TRCH_Parameter_var.pending[channel >> 5] |= 1<<(channel & 0x1f);
						break;

					default:
						break;
				}
			}
			else { /* do nothing */ }
			++parameter_ptr;
			++channel;
		}
	}
	else{/* do nothing */}
}

