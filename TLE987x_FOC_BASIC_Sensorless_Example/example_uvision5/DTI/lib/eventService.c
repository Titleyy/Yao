/**********************************************************************************************************************
 * File:  eventService.c	
 * RqID:  
 * Brief: Event Service as part of Trace Handler for target interface
 * $Id: 68e78a4dce0309d0d3f04e0137e8557332ad2fa3 $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-14:
 *     - Initial version
 *
 */
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include "targetInterface.h"
#include "eventService.h"


/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#define TRCH_SERVICE_EVENT_MAX_CONFIGURATIONS (2)

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/
typedef enum{
	TRCH_response_size_Event_GetConfiguration = 8,
	TRCH_response_size_Event_ConfigureService = 8,
	TRCH_response_size_Event_ConfigureEvent = 12,
	TRCH_response_size_Event_GetEventBufferEntry = 9,
	TRCH_response_size_Event_ERROR = 6,
} TRCH_response_size_Event_e;

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
typedef struct {
	union {
		uint32_t configuration[TRCH_SERVICE_EVENT_MAX_CONFIGURATIONS];
		struct 
		{
			/* Get Version */
			uint16_t version;
			uint16_t reserved;
			/* Get Capability */
			uint8_t maxEv;
			uint8_t buffersize; /* size of event buffer */
			uint8_t reserved2;
			uint8_t reserved3;
		};
	};
	uint8_t numEv;
	uint8_t clkdiv;
} TRCH_Event_ct;

typedef struct {
	uint32_t idxTargetX;
	union {
		uint32_t idxTargetY;
		uint32_t threshold;
	};
	uint32_t* signalXptr;
	uint32_t* signalYptr;
	uint32_t signalX;
	uint32_t signalY;
	TRCH_EventFunction_e function;
	TRCH_SignalType_t signalXtype:4;
	TRCH_SignalType_t signalYtype:4;
	uint8_t result;
	uint8_t event;
} TRCH_EventChannel_t;

typedef struct {
	uint8_t event;
	DTI_frame_FTS_t TS_value;
	uint32_t target_time;
} TRCH_EventBuffer_t;

typedef struct {
	TRCH_EventBuffer_t eventbuffer[TRCH_SERVICE_EVENT_BUFFER_MAX];
	TRCH_EventBuffer_t *writeptr;
	TRCH_EventBuffer_t *readptr;
	TRCH_EventChannel_t eventchannel[TRCH_SERVICE_EVENT_CHANNEL_MAX];
	uint8_t numEv;	
	uint8_t clkdiv;
} TRCH_Event_vt;

/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
const TRCH_Event_ct TRCH_Event_cnf = {
	{
		.version = DTI_VERSION(0,0,1),
		.maxEv = TRCH_SERVICE_EVENT_CHANNEL_MAX, /* maximum number of event channels */
		.buffersize = TRCH_SERVICE_EVENT_BUFFER_MAX /* maximum number of event entries in buffer */
	},
	/* default values*/
	.numEv = 0, /* Number of active events */
	.clkdiv = 0 /* event channel  processing stopped */
};

uint32_t* TRCH_Event_EvOut;

/***********************************************************************************************************************
 * LOCAL ROUTINES
 **********************************************************************************************************************/

static TRCH_ERRORCODES_e GetConfiguration(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e 	ConfigureService(TRCH_buffer_t*,TRCH_buffer_t*);
static TRCH_ERRORCODES_e 	ConfigureEvent(TRCH_buffer_t*,TRCH_buffer_t*);


/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
 TRCH_ERRORCODES_e (*TRCH_EventCmd[])(TRCH_buffer_t*,TRCH_buffer_t*) = 
{
	GetConfiguration,
	GetConfiguration,
	ConfigureService,
	ConfigureEvent
};

TRCH_Event_vt TRCH_Event_var = {
	.eventchannel[0].function = TRCH_EventFunction_END,
};

TRCH_ERRORCODES_e TRCH_EventService(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	if( request->cmd < (sizeof(TRCH_EventCmd) / sizeof(TRCH_ERRORCODES_e(*)(TRCH_buffer_t*,TRCH_buffer_t*))) )
	{
		errorcode = TRCH_EventCmd[request->cmd](request, response);
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

TRCH_ERRORCODES_e TRCH_EventService_init(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_Event_var.numEv = TRCH_Event_cnf.numEv;
	TRCH_Event_var.clkdiv = TRCH_Event_cnf.clkdiv;
	
	for(int i = 0; i < TRCH_Event_cnf.maxEv; ++i)
	{
		TRCH_Event_var.eventchannel[i].function = TRCH_EventFunction_END;
	}
	
	/* initialize FIFO buffer */
	TRCH_Event_var.writeptr = TRCH_Event_var.eventbuffer;
	TRCH_Event_var.readptr = TRCH_Event_var.writeptr;
	
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e TRCH_EventService_deinit(TRCH_buffer_t* request,TRCH_buffer_t* response)
{
	TRCH_Event_var.numEv = 0; /* set number of active events to 0 */
	TRCH_Event_var.clkdiv = 0; /* stop event channel processing */
	TRCH_Event_var.eventchannel[0].function = TRCH_EventFunction_END;
	
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e GetConfiguration(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	response->cmd = request->cmd;
	response->channel = 0; 
	response->data32[0] = TRCH_Event_cnf.configuration[request->cmd];
	response->size = TRCH_response_size_Event_GetConfiguration;
	return TRCH_ERRORCODES_Success;
}

TRCH_ERRORCODES_e 	ConfigureService(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	
	response->cmd = request->cmd;
	response->event = 0; 

	if( request->data8[0] < TRCH_Event_cnf.maxEv)
	{
		if( request->data8[0] )
		{ /* Number of events is greater than 0 */
			TRCH_Event_var.numEv = request->data8[0];
			TRCH_Event_var.clkdiv = request->data8[1];
			TRCH_Event_var.eventchannel[TRCH_Event_var.numEv].function = TRCH_EventFunction_END;
			errorcode = TRCH_ERRORCODES_Success;
		}
		else 
		{ /* Number of events is 0, clear event output and report error */
			if( TRCH_Event_EvOut )
			{ /* clear all events from the output pointer */
				*TRCH_Event_EvOut = 0;
			}
			errorcode = TRCH_ERRORCODES_Event_ZeroEventsNotAllowed;
		}
		response->data8[0] = TRCH_Event_var.numEv; /* return currrent setting*/
		response->data8[1] = TRCH_Event_var.clkdiv; /* return currrent setting*/
		
		response->size = TRCH_response_size_Event_ConfigureService;
	}
	else
	{
		response->event = 0x0;
		errorcode = TRCH_ERRORCODES_Event_MaxNumberEventsExceeded;
	}

	return errorcode;
}

/***********************************
 * data8[0] .. data8[2]: signalX   *
 * data8[3]            : function  *
 * data8[4] .. data8[6]: signalY   *
 * data32[1]           : threshold *
 ***********************************/
TRCH_ERRORCODES_e ConfigureEvent(TRCH_buffer_t* request, TRCH_buffer_t* response)
{
	TRCH_ERRORCODES_e errorcode; 
	TRCH_EventChannel_t* eventchannel;
	uint8_t function;
	
	errorcode = TRCH_ERRORCODES_Event_FunctionNotAvailable;
	response->cmd = request->cmd;
	response->event = request->event;

	if( request->event <= TRCH_Event_cnf.maxEv )
	{
		/* valid event channel */
		eventchannel = &TRCH_Event_var.eventchannel[request->event];
		if( /*function*/ 0xFF == request->data8[3] )
		{ 
			/* return the event channel configuration */
			response->data32[0] = eventchannel->idxTargetX;
			response->data32[1] = eventchannel->idxTargetY; /* signal Y is union with threshold */
			response->data8[3] = eventchannel->function;
			response->size = TRCH_response_size_Event_ConfigureEvent;
			errorcode = TRCH_ERRORCODES_Success;
		}
		else
		{
			/* configure this event */
			function = request->data8[3];
			eventchannel->function = TRCH_EventFunction_none; // ARNO: better do not touch the event channel configuration unless its validity is proven
			eventchannel->event = request->event;
			/* start in detected state, so a 1->0->1 transition is required to actually trigger the event */
			eventchannel->result = 1;
			if( TRCH_Event_EvOut )
			{ /* clear a pending event flag */
				*TRCH_Event_EvOut &= ~(1<< eventchannel->event);
			}

			if( (function & 0x1F) >= TRCH_EventFunction_XeqY && (function &0x1F) <= TRCH_EventFunction_XneqY)
			{
				/* store signal numbers */
				eventchannel->idxTargetX = request->data32[0] & 0x00ffffff;
				eventchannel->idxTargetY = request->data32[1] & 0x00ffffff;

				/* get signal pointer */
				eventchannel->signalXptr = (uint32_t*)TRCH_getSignalptr( eventchannel->idxTargetX );
				eventchannel->signalXtype =  TRCH_getSignalType( eventchannel->idxTargetX );
				eventchannel->signalYptr = (uint32_t*)TRCH_getSignalptr( eventchannel->idxTargetY );
				eventchannel->signalYtype =  TRCH_getSignalType( eventchannel->idxTargetY );
				
				
				if( eventchannel->signalXptr && eventchannel->signalYptr )
				{ /* signal assignment OK */
					eventchannel->function = (TRCH_EventFunction_e) function;
					eventchannel->event = request->event;
					response->data32[0] = eventchannel->idxTargetX;
					response->data32[1] = eventchannel->idxTargetY;
					response->size = TRCH_response_size_Event_ConfigureEvent;
					errorcode = TRCH_ERRORCODES_Success;
				}
				else
				{/* signal assignment error */
					errorcode = TRCH_ERRORCODES_Event_SignalNotAvailable;
				}
			}
			else if( (function & 0x1F) >= TRCH_EventFunction_XeqX1 && (function & 0x1F) <= TRCH_EventFunction_XneqT)
			{
				/* store signal number */
				eventchannel->idxTargetX = request->data32[0] & 0x00ffffff;

				/* get signal pointer */
				eventchannel->signalXptr = (uint32_t*)TRCH_getSignalptr( eventchannel->idxTargetX );
				eventchannel->signalXtype =  TRCH_getSignalType( eventchannel->idxTargetX );
				eventchannel->signalYtype =  TRCH_SignalType_none;

				if( eventchannel->signalXptr )
				{ /* signal assignment OK */
					eventchannel->threshold = request->data32[1];
					eventchannel->function = (TRCH_EventFunction_e) function;
					response->data32[0] = eventchannel->idxTargetX;
					response->data32[1] = eventchannel->threshold;
					response->size = TRCH_response_size_Event_ConfigureEvent;
					errorcode = TRCH_ERRORCODES_Success;
				}
				else
				{/* signal assignment error */
					errorcode = TRCH_ERRORCODES_Event_SignalNotAvailable;
				}
			}
			else if( function == TRCH_EventFunction_none )
			{
				eventchannel->function = TRCH_EventFunction_none;
				response->data32[0] = 0;
				response->data32[1] = 0;
				response->size = TRCH_response_size_Event_ConfigureEvent;
				errorcode = TRCH_ERRORCODES_Success;
			}
			else
			{
				/* unknown function */
					errorcode = TRCH_ERRORCODES_Event_FunctionNotAvailable;
			}
		}
	}
	else
	{
		/* invalid event channel */
		response->event = 0x0;
		errorcode = TRCH_ERRORCODES_Event_MaxNumberEventsExceeded;
	}
	return errorcode;
}

uint32_t TRCH_EventService_isBufferIdle(void)
{
	return TRCH_Event_var.readptr == TRCH_Event_var.writeptr;
}

TRCH_ERRORCODES_e TRCH_EventService_getBufferEntry(TRCH_buffer_t* response)
{
	if( !TRCH_EventService_isBufferIdle() )
	{
		response->event = TRCH_Event_var.readptr->event;
		response->data32[0] = TRCH_Event_var.readptr->target_time;
		response->data8[4] = TRCH_Event_var.readptr->TS_value;
		++TRCH_Event_var.readptr;
		if( TRCH_Event_var.readptr == &TRCH_Event_var.eventbuffer[TRCH_SERVICE_EVENT_BUFFER_MAX] )
		{
			TRCH_Event_var.readptr = TRCH_Event_var.eventbuffer;
		}
	}
	else
	{
		response->event  = 0xFF;
		response->data32[0] = 0;
		response->data32[1] = 0;
	}
	
	response->size = TRCH_response_size_Event_GetEventBufferEntry;
	return TRCH_ERRORCODES_Success;
}

void TRCH_EventService_BufferAppend( uint8_t event )
{
	TRCH_Event_var.writeptr->target_time = DTI_TSdriver_getTargetTime();
	TRCH_Event_var.writeptr->TS_value = DTI_TSdriver_getTimeStamp();
	TRCH_Event_var.writeptr->event = event;
	
	++TRCH_Event_var.writeptr;
	if( TRCH_Event_var.writeptr == &TRCH_Event_var.eventbuffer[TRCH_SERVICE_EVENT_BUFFER_MAX] )
	{
		TRCH_Event_var.writeptr = TRCH_Event_var.eventbuffer;
	}
}


bool TRCH_EventFunction_none_func(TRCH_EventChannel_t* channel)
{
	return false;
}

/* 8bit functions */
bool TRCH_EventFunction_XeqX1_func8bit(TRCH_EventChannel_t* channel)
{
	uint8_t retval;
	
	retval = ((uint8_t)channel->signalX) == channel->signalY;
	channel->signalY = ((uint8_t)channel->signalX);
	
	return retval;
}

bool TRCH_EventFunction_uXgtX1_func8bit(TRCH_EventChannel_t* channel)
{
	uint8_t retval;

	retval = ((uint8_t)channel->signalX) > ((uint8_t)channel->signalY);
	channel->signalY = ((uint8_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_sXgtX1_func8bit(TRCH_EventChannel_t* channel)
{
	uint8_t retval;

	retval = ((int8_t)channel->signalX) > ((int8_t)channel->signalY);
	channel->signalY = ((int8_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_uXltX1_func8bit(TRCH_EventChannel_t* channel)
{
	uint8_t retval;

	retval = ((uint8_t)channel->signalX) < ((uint8_t)channel->signalY);
	channel->signalY = ((uint8_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_sXltX1_func8bit(TRCH_EventChannel_t* channel)
{
	uint8_t retval;

	retval = ((int8_t)channel->signalX) < ((int8_t)channel->signalY);
	channel->signalY = ((int8_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_XneqX1_func8bit(TRCH_EventChannel_t* channel)
{
	uint8_t retval;

	retval = ((uint8_t)channel->signalX) != (uint8_t)channel->signalY;
	channel->signalY = ((uint8_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_XeqT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) == (uint8_t)channel->threshold;
}


bool TRCH_EventFunction_uXgtT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) > ((uint8_t)channel->threshold);
}


bool TRCH_EventFunction_sXgtT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((int8_t)channel->signalX) > ((int8_t)channel->threshold);
}

bool TRCH_EventFunction_uXgeT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) >= ((uint8_t)channel->threshold);
}


bool TRCH_EventFunction_sXgeT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((int8_t)channel->signalX) >= ((int8_t)channel->threshold);
}


bool TRCH_EventFunction_uXltT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) < ((uint8_t)channel->threshold);
}


bool TRCH_EventFunction_sXltT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((int8_t)channel->signalX) < ((int8_t)channel->threshold);
}

bool TRCH_EventFunction_uXleT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) <= ((uint8_t)channel->threshold);
}


bool TRCH_EventFunction_sXleT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((int8_t)channel->signalX) <= ((int8_t)channel->threshold);
}


bool TRCH_EventFunction_XneqT_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) != (uint8_t)channel->threshold;
}



bool TRCH_EventFunction_XeqY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) == ((uint8_t)channel->signalY);
}


bool TRCH_EventFunction_uXgtY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) > ((uint8_t)channel->signalY);
}


bool TRCH_EventFunction_sXgtY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((int8_t)channel->signalX) > ((int8_t)channel->signalY);
}


bool TRCH_EventFunction_uXgeY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) >= ((uint8_t)channel->signalY);
}


bool TRCH_EventFunction_sXgeY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((int8_t)channel->signalX) >= ((int8_t)channel->signalY);
}


bool TRCH_EventFunction_uXltY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) < ((uint8_t)channel->signalY);
}


bool TRCH_EventFunction_sXltY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((int8_t)channel->signalX) < ((int8_t)channel->signalY);
}

bool TRCH_EventFunction_uXleY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) <= ((uint8_t)channel->signalY);
}


bool TRCH_EventFunction_sXleY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((int8_t)channel->signalX) <= ((int8_t)channel->signalY);
}


bool TRCH_EventFunction_XneqY_func8bit(TRCH_EventChannel_t* channel)
{
	return  ((uint8_t)channel->signalX) != ((uint8_t)channel->signalY);
}

/* 16bit functions */
bool TRCH_EventFunction_XeqX1_func16bit(TRCH_EventChannel_t* channel)
{
	uint16_t retval;

	retval = ((int16_t)channel->signalX) == ((int16_t)channel->signalY);
	channel->signalY = channel->signalX;

	return retval;
}

bool TRCH_EventFunction_uXgtX1_func16bit(TRCH_EventChannel_t* channel)
{
	uint16_t retval;

	retval = ((uint16_t)channel->signalX) > ((uint16_t)channel->signalY);
	channel->signalY = channel->signalX;

	return retval;
}


bool TRCH_EventFunction_sXgtX1_func16bit(TRCH_EventChannel_t* channel)
{
	uint16_t retval;

	retval = ((int16_t)channel->signalX) > ((int16_t)channel->signalY);
	channel->signalY = channel->signalX;
	
	return retval;
}


bool TRCH_EventFunction_uXltX1_func16bit(TRCH_EventChannel_t* channel)
{
	uint16_t retval;

	retval = ((uint16_t)channel->signalX) < ((uint16_t)channel->signalY);
	channel->signalY = channel->signalX;
	
	return retval;
}


bool TRCH_EventFunction_sXltX1_func16bit(TRCH_EventChannel_t* channel)
{
	uint16_t retval;

	retval = ((int16_t)channel->signalX) < ((int16_t)channel->signalY);
	channel->signalY = channel->signalX;
	
	return retval;
}


bool TRCH_EventFunction_XneqX1_func16bit(TRCH_EventChannel_t* channel)
{
	uint16_t retval;

	retval = ((int16_t)channel->signalX) != ((int16_t)channel->signalY);
	channel->signalY = channel->signalX;
	
	return retval;
}


bool TRCH_EventFunction_XeqT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) == (uint16_t)channel->threshold;
}


bool TRCH_EventFunction_uXgtT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) > ((uint16_t)channel->threshold);
}


bool TRCH_EventFunction_sXgtT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((int16_t)channel->signalX) > ((int16_t)channel->threshold);
}

bool TRCH_EventFunction_uXgeT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) >= ((uint16_t)channel->threshold);
}


bool TRCH_EventFunction_sXgeT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((int16_t)channel->signalX) >= ((int16_t)channel->threshold);
}


bool TRCH_EventFunction_uXltT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) < ((uint16_t)channel->threshold);
}


bool TRCH_EventFunction_sXltT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((int16_t)channel->signalX) < ((int16_t)channel->threshold);
}

bool TRCH_EventFunction_uXleT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) <= ((uint16_t)channel->threshold);
}


bool TRCH_EventFunction_sXleT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((int16_t)channel->signalX) <= ((int16_t)channel->threshold);
}


bool TRCH_EventFunction_XneqT_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) != (uint16_t)channel->threshold;
}



bool TRCH_EventFunction_XeqY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) == ((uint16_t)channel->signalY);
}


bool TRCH_EventFunction_uXgtY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) > ((uint16_t)channel->signalY);
}


bool TRCH_EventFunction_sXgtY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((int16_t)channel->signalX) > ((int16_t)channel->signalY);
}


bool TRCH_EventFunction_uXgeY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) >= ((uint16_t)channel->signalY);
}


bool TRCH_EventFunction_sXgeY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((int16_t)channel->signalX) >= ((int16_t)channel->signalY);
}


bool TRCH_EventFunction_uXltY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) < ((uint16_t)channel->signalY);
}


bool TRCH_EventFunction_sXltY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((int16_t)channel->signalX) < ((int16_t)channel->signalY);
}

bool TRCH_EventFunction_uXleY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) <= ((uint16_t)channel->signalY);
}


bool TRCH_EventFunction_sXleY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((int16_t)channel->signalX) <= ((int16_t)channel->signalY);
}


bool TRCH_EventFunction_XneqY_func16bit(TRCH_EventChannel_t* channel)
{
	return  ((uint16_t)channel->signalX) != ((uint16_t)channel->signalY);
}

/* 32bit functions */
bool TRCH_EventFunction_XeqX1_func32bit(TRCH_EventChannel_t* channel)
{
	uint32_t retval;
	
	retval = ((uint32_t)channel->signalX) == channel->signalY;
	channel->signalY = ((uint32_t)channel->signalX);
	
	return retval;
}

bool TRCH_EventFunction_uXgtX1_func32bit(TRCH_EventChannel_t* channel)
{
	uint32_t retval;

	retval = ((uint32_t)channel->signalX) > ((uint32_t)channel->signalY);
	channel->signalY = ((uint32_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_sXgtX1_func32bit(TRCH_EventChannel_t* channel)
{
	uint32_t retval;

	retval = ((int32_t)channel->signalX) > ((int32_t)channel->signalY);
	channel->signalY = ((int32_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_uXltX1_func32bit(TRCH_EventChannel_t* channel)
{
	uint32_t retval;

	retval = ((uint32_t)channel->signalX) < ((uint32_t)channel->signalY);
	channel->signalY = ((uint32_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_sXltX1_func32bit(TRCH_EventChannel_t* channel)
{
	uint32_t retval;

	retval = ((int32_t)channel->signalX) < ((int32_t)channel->signalY);
	channel->signalY = ((int32_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_XneqX1_func32bit(TRCH_EventChannel_t* channel)
{
	uint32_t retval;

	retval = ((uint32_t)channel->signalX) != (uint32_t)channel->signalY;
	channel->signalY = ((uint32_t)channel->signalX);
	
	return retval;
}


bool TRCH_EventFunction_XeqT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) == (uint32_t)channel->threshold;
}


bool TRCH_EventFunction_uXgtT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) > ((uint32_t)channel->threshold);
}


bool TRCH_EventFunction_sXgtT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((int32_t)channel->signalX) > ((int32_t)channel->threshold);
}

bool TRCH_EventFunction_uXgeT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) >= ((uint32_t)channel->threshold);
}


bool TRCH_EventFunction_sXgeT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((int32_t)channel->signalX) >= ((int32_t)channel->threshold);
}


bool TRCH_EventFunction_uXltT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) < ((uint32_t)channel->threshold);
}


bool TRCH_EventFunction_sXltT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((int32_t)channel->signalX) < ((int32_t)channel->threshold);
}

bool TRCH_EventFunction_uXleT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) <= ((uint32_t)channel->threshold);
}


bool TRCH_EventFunction_sXleT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((int32_t)channel->signalX) <= ((int32_t)channel->threshold);
}


bool TRCH_EventFunction_XneqT_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) != (uint32_t)channel->threshold;
}



bool TRCH_EventFunction_XeqY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) == ((uint32_t)channel->signalY);
}


bool TRCH_EventFunction_uXgtY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) > ((uint32_t)channel->signalY);
}


bool TRCH_EventFunction_sXgtY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((int32_t)channel->signalX) > ((int32_t)channel->signalY);
}


bool TRCH_EventFunction_uXgeY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) >= ((uint32_t)channel->signalY);
}


bool TRCH_EventFunction_sXgeY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((int32_t)channel->signalX) >= ((int32_t)channel->signalY);
}


bool TRCH_EventFunction_uXltY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) < ((uint32_t)channel->signalY);
}


bool TRCH_EventFunction_sXltY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((int32_t)channel->signalX) < ((int32_t)channel->signalY);
}

bool TRCH_EventFunction_uXleY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) <= ((uint32_t)channel->signalY);
}


bool TRCH_EventFunction_sXleY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((int32_t)channel->signalX) <= ((int32_t)channel->signalY);
}


bool TRCH_EventFunction_XneqY_func32bit(TRCH_EventChannel_t* channel)
{
	return  ((uint32_t)channel->signalX) != ((uint32_t)channel->signalY);
}


bool (*const TRCH_EventService_ProcessChannel[])(TRCH_EventChannel_t*) = 
{
/* 8bit signals */
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_XeqX1_func8bit,
	TRCH_EventFunction_uXgtX1_func8bit,
	TRCH_EventFunction_sXgtX1_func8bit,
	TRCH_EventFunction_uXltX1_func8bit,
	TRCH_EventFunction_sXltX1_func8bit,
	TRCH_EventFunction_XneqX1_func8bit,

	TRCH_EventFunction_XeqT_func8bit,
	TRCH_EventFunction_uXgtT_func8bit,
	TRCH_EventFunction_sXgtT_func8bit,
	TRCH_EventFunction_uXgeT_func8bit,
	TRCH_EventFunction_sXgeT_func8bit,
	TRCH_EventFunction_uXltT_func8bit,
	TRCH_EventFunction_sXltT_func8bit,
	TRCH_EventFunction_uXleT_func8bit,
	TRCH_EventFunction_sXleT_func8bit,
	TRCH_EventFunction_XneqT_func8bit,

	TRCH_EventFunction_XeqY_func8bit,
	TRCH_EventFunction_uXgtY_func8bit,
	TRCH_EventFunction_sXgtY_func8bit,
	TRCH_EventFunction_uXgeY_func8bit,
	TRCH_EventFunction_sXgeY_func8bit,
	TRCH_EventFunction_uXltY_func8bit,
	TRCH_EventFunction_sXltY_func8bit,
	TRCH_EventFunction_uXleY_func8bit,
	TRCH_EventFunction_sXleY_func8bit,
	TRCH_EventFunction_XneqY_func8bit,

	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,

/* 16bit signals */
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_XeqX1_func16bit,
	TRCH_EventFunction_uXgtX1_func16bit,
	TRCH_EventFunction_sXgtX1_func16bit,
	TRCH_EventFunction_uXltX1_func16bit,
	TRCH_EventFunction_sXltX1_func16bit,
	TRCH_EventFunction_XneqX1_func16bit,

	TRCH_EventFunction_XeqT_func16bit,
	TRCH_EventFunction_uXgtT_func16bit,
	TRCH_EventFunction_sXgtT_func16bit,
	TRCH_EventFunction_uXgeT_func16bit,
	TRCH_EventFunction_sXgeT_func16bit,
	TRCH_EventFunction_uXltT_func16bit,
	TRCH_EventFunction_sXltT_func16bit,
	TRCH_EventFunction_uXleT_func16bit,
	TRCH_EventFunction_sXleT_func16bit,
	TRCH_EventFunction_XneqT_func16bit,

	TRCH_EventFunction_XeqY_func16bit,
	TRCH_EventFunction_uXgtY_func16bit,
	TRCH_EventFunction_sXgtY_func16bit,
	TRCH_EventFunction_uXgeY_func16bit,
	TRCH_EventFunction_sXgeY_func16bit,
	TRCH_EventFunction_uXltY_func16bit,
	TRCH_EventFunction_sXltY_func16bit,
	TRCH_EventFunction_uXleY_func16bit,
	TRCH_EventFunction_sXleY_func16bit,
	TRCH_EventFunction_XneqY_func16bit,

	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,

/* 32bit signals */
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_XeqX1_func32bit,
	TRCH_EventFunction_uXgtX1_func32bit,
	TRCH_EventFunction_sXgtX1_func32bit,
	TRCH_EventFunction_uXltX1_func32bit,
	TRCH_EventFunction_sXltX1_func32bit,
	TRCH_EventFunction_XneqX1_func32bit,

	TRCH_EventFunction_XeqT_func32bit,
	TRCH_EventFunction_uXgtT_func32bit,
	TRCH_EventFunction_sXgtT_func32bit,
	TRCH_EventFunction_uXgeT_func32bit,
	TRCH_EventFunction_sXgeT_func32bit,
	TRCH_EventFunction_uXltT_func32bit,
	TRCH_EventFunction_sXltT_func32bit,
	TRCH_EventFunction_uXleT_func32bit,
	TRCH_EventFunction_sXleT_func32bit,
	TRCH_EventFunction_XneqT_func32bit,

	TRCH_EventFunction_XeqY_func32bit,
	TRCH_EventFunction_uXgtY_func32bit,
	TRCH_EventFunction_sXgtY_func32bit,
	TRCH_EventFunction_uXgeY_func32bit,
	TRCH_EventFunction_sXgeY_func32bit,
	TRCH_EventFunction_uXltY_func32bit,
	TRCH_EventFunction_sXltY_func32bit,
	TRCH_EventFunction_uXleY_func32bit,
	TRCH_EventFunction_sXleY_func32bit,
	TRCH_EventFunction_XneqY_func32bit,

	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
	TRCH_EventFunction_none_func,
};

__attribute__((section ("RAMCODE")))
void TRCH_EventService_Process(void)
{
	TRCH_EventChannel_t* channel;
	channel = TRCH_Event_var.eventchannel;
	
	while( channel->function != TRCH_EventFunction_END )
	{
		if( !channel->result )
		{
			
			switch(channel->signalXtype)
			{
					case TRCH_SignalType_16bit: /* use 16bit channel pointer */
						channel->signalX = (int32_t)(*(int16_t*)channel->signalXptr);
						break;

					case TRCH_SignalType_Input16bit: /* use 16bit channel input pointer */
						if( *(int16_t*)channel->signalXptr )
						{
							channel->signalX = (int32_t)(**(int16_t**)channel->signalXptr);
						}
						break;

					case TRCH_SignalType_API16bit: /* use 16bit signed values */
						channel->signalX = (int32_t)((int16_t)((TRCH_getSignalApi_ptr_t)channel->signalXptr)());
						break;
			}
			
			switch(channel->signalYtype)
			{
					case TRCH_SignalType_16bit: /* use 16bit channel pointer */
						channel->signalY = (int32_t)(*(int16_t*)channel->signalYptr);
						break;

					case TRCH_SignalType_Input16bit: /* use 16bit channel input pointer */
						if( *(uint16_t*)channel->signalYptr )
						{
							channel->signalY = (int32_t)(**(uint16_t**)channel->signalYptr);
						}
						break;

					case TRCH_SignalType_API16bit: /* use 16bit signed values */
						channel->signalY = (int32_t)((uint16_t)((TRCH_getSignalApi_ptr_t)channel->signalYptr)());
						break;
			}
			
			channel->result = TRCH_EventService_ProcessChannel[channel->function]( channel );
			if( channel->result )
			{
				if( channel->event || !TRCH_Event_EvOut) 
				{ /* if Event #0 is used by sampling service do not send a separate event message */
					TRCH_EventService_BufferAppend( channel->event );
				}
				else{/*do nothing*/}
				
				if( TRCH_Event_EvOut )
				{ /* set the event flag */
					*TRCH_Event_EvOut |= 1<< channel->event;
				}
				else{/*do nothing*/}
			}
			else{/*do nothing*/}
		}
		else
		{
			switch(channel->signalXtype)
			{
					case TRCH_SignalType_16bit: /* use 16bit channel pointer */
						channel->signalX = (int32_t)(*(int16_t*)channel->signalXptr);
						break;

					case TRCH_SignalType_Input16bit: /* use 16bit channel input pointer */
						if( *(int16_t*)channel->signalXptr )
						{
							channel->signalX = (int32_t)(**(int16_t**)channel->signalXptr);
						}
						break;

					case TRCH_SignalType_API16bit: /* use 16bit signed values */
						channel->signalX = (int32_t)((int16_t)((TRCH_getSignalApi_ptr_t)channel->signalXptr)());
						break;
			}
			
			switch(channel->signalYtype)
			{
					case TRCH_SignalType_16bit: /* use 16bit channel pointer */
						channel->signalY = (int32_t)(*(int16_t*)channel->signalYptr);
						break;

					case TRCH_SignalType_Input16bit: /* use 16bit channel input pointer */
						if( *(uint16_t*)channel->signalYptr )
						{
							channel->signalY = (int32_t)(**(uint16_t**)channel->signalYptr);
						}
						break;

					case TRCH_SignalType_API16bit: /* use 16bit signed values */
						channel->signalY = (int32_t)((uint16_t)((TRCH_getSignalApi_ptr_t)channel->signalYptr)());
						break;
			}
			
			channel->result = TRCH_EventService_ProcessChannel[channel->function]( channel );
		}
		++channel;
	}
}

