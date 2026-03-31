/**********************************************************************************************************************
 * File:  messageHandler_api.h	
 * RqID:  
 * Brief: API of Messager Handler for target interface
 * $Id: 6c4085eff461787ff103a6517aecd37e1b527405 $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-25:
 *     - Initial version
 *
 */

#ifndef _dti_tracehandler_api_h
#define _dti_tracehandler_api_h
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include <stdint.h>

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/
typedef enum {
	/* service number definition */
	TRCH_SERVICE_TRACEHANDLER = 0,        /* The trace handler itself */
	TRCH_SERVICE_PROGRAMMING = 1,         /* Programming service */
	TRCH_SERVICE_STREAMING = 2,           /* Streaming TRACE service */
	TRCH_SERVICE_EVENT = 3,               /* Event TRACE service */
	TRCH_SERVICE_SAMPLING = 4,            /* Sampling TRACE service */
	TRCH_SERVICE_PARAMETER = 5,           /* Parameter TRACE service */
	TRCH_SERVICE_MAX = 6,                 /* Number of available services */
	/* other values for trace service */
	TRCH_SERVICE_RESPONSE_ASYNC = 14,
	TRCH_SERVICE_RESPONSE_ERROR = 15,
} TRCH_SERVICE_e;

typedef enum {
	TRCH_ERRORCODES_Success = 0x0000,
	
	TRCH_ERRORCODES_ServiceNotAvailable = 0x0001,
	TRCH_ERRORCODES_ServiceNotEnabled = 0x0002,
	TRCH_ERRORCODES_ServiceAlreadyEnabled = 0x0003,
	TRCH_ERRORCODES_ServiceAlreadyDisabled = 0x0004,
	TRCH_ERRORCODES_ServiceEnableFailed = 0x0005,
	TRCH_ERRORCODES_ServiceDisableFailed = 0x0006,
	TRCH_ERRORCODES_StatisticNotAvailable = 0x0007,
	TRCH_ERRORCODES_IdNotAvailable = 0x0008,
	
	TRCH_ERRORCODES_Streaming_ChannelNotAvailable = 0x0201,
	TRCH_ERRORCODES_Streaming_SignalNotAvailable = 0x0202,
	TRCH_ERRORCODES_Streaming_MaxNumberChannelsExceeded = 0x0203,
	TRCH_ERRORCODES_Streaming_ResolutionNotSupported = 0x0204,
	TRCH_ERRORCODES_Streaming_MaxNumberClockSourcesExceeded = 0x0205,

	TRCH_ERRORCODES_Event_EventNotAvailable = 0x0301,
	TRCH_ERRORCODES_Event_SignalNotAvailable = 0x0302,
	TRCH_ERRORCODES_Event_FunctionNotAvailable = 0x0303,
	TRCH_ERRORCODES_Event_MaxNumberEventsExceeded = 0x0304,
	TRCH_ERRORCODES_Event_BufferOverrun = 0x0305,
	TRCH_ERRORCODES_Event_ZeroEventsNotAllowed = 0x0306,               /* added 2020-11-23 */
	
	TRCH_ERRORCODES_Sampling_ChannelNotAvailable = 0x0401,
	TRCH_ERRORCODES_Sampling_SignalNotAvailable = 0x0402,
	TRCH_ERRORCODES_Sampling_MaxNumberChannelsExceeded = 0x0403,
	TRCH_ERRORCODES_Sampling_MaxBuffersizeExceeded = 0x0404,
	TRCH_ERRORCODES_Sampling_MaxNumberClockSourcesExceeded = 0x0405,
	TRCH_ERRORCODES_Sampling_MaxNumberUsableEventsExceeded = 0x0406,
	TRCH_ERRORCODES_Sampling_TriggermodeNotSupported = 0x0407,

	TRCH_ERRORCODES_Parameter_SubscriptionListEmpty = 0x0501,
	TRCH_ERRORCODES_Parameter_ChannelNotAvailable = 0x0502,
	TRCH_ERRORCODES_Parameter_ParameterNotAvailable = 0x0503,
	TRCH_ERRORCODES_Parameter_MaxNumberChannelsExceeded = 0x0504,

	TRCH_ERRORCODES_Aborted = 0xFFFC,
	TRCH_ERRORCODES_UnknownError = 0xFFFD,
	TRCH_ERRORCODES_CommandNotAvailable = 0xFFFE,
	TRCH_ERRORCODES_UnkownCommand = 0xFFFF
} TRCH_ERRORCODES_e;

typedef enum {
	TRCH_EventFunction_none,
	TRCH_EventFunction_XeqX1,
	TRCH_EventFunction_uXgtX1,
	TRCH_EventFunction_sXgtX1,
	TRCH_EventFunction_uXltX1,
	TRCH_EventFunction_sXltX1,
	TRCH_EventFunction_XneqX1,

	TRCH_EventFunction_XeqT,
	TRCH_EventFunction_uXgtT,
	TRCH_EventFunction_sXgtT,
	TRCH_EventFunction_uXgeT,
	TRCH_EventFunction_sXgeT,
	TRCH_EventFunction_uXltT,
	TRCH_EventFunction_sXltT,
	TRCH_EventFunction_uXleT,
	TRCH_EventFunction_sXleT,
	TRCH_EventFunction_XneqT,

	TRCH_EventFunction_XeqY,
	TRCH_EventFunction_uXgtY,
	TRCH_EventFunction_sXgtY,
	TRCH_EventFunction_uXgeY,
	TRCH_EventFunction_sXgeY,
	TRCH_EventFunction_uXltY,
	TRCH_EventFunction_sXltY,
	TRCH_EventFunction_uXleY,
	TRCH_EventFunction_sXleY,
	TRCH_EventFunction_XneqY,
	
	TRCH_EventFunction_END = 0xFF
} TRCH_EventFunction_e ;

 typedef  void (*TRCH_ClockCallback_t)(void);
 
/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					

/***********************************************************************************************************************
* EXTERN DECLARATIONS
***********************************************************************************************************************/

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/
void TRCH_Init(void);
uint32_t TRCH_ProcessTrace(void);
void TRCH_EventService_Process(void);
void TRCH_ParameterService_Process(void);

/***********************************************************************************************************************
 * static inline Functions
 **********************************************************************************************************************/
static inline void TRCH_Polling_Init(void)
{
	TRCH_Init();
}

static inline uint32_t TRCH_Polling_ProcessTrace(void)
{
	return TRCH_ProcessTrace();
}

extern TRCH_ClockCallback_t TRCH_StreamingService_ClockCallback[];
extern TRCH_ClockCallback_t TRCH_SamplingService_ClockCallback[];

/* Implementation of Callback Functions */
static inline void TRCH_ClockSource0(void)
{
	if( TRCH_SamplingService_ClockCallback[0] )
	{
		TRCH_EventService_Process();
		TRCH_SamplingService_ClockCallback[0]();
	}
	if( TRCH_StreamingService_ClockCallback[0] )
	{
		TRCH_StreamingService_ClockCallback[0]();
	}
}

static inline void TRCH_ClockSource1(void)
{
	if( TRCH_SamplingService_ClockCallback[1] )
	{
		TRCH_EventService_Process();
		TRCH_SamplingService_ClockCallback[1]();
	}
	if( TRCH_StreamingService_ClockCallback[1] )
	{
		TRCH_StreamingService_ClockCallback[1]();
	}
}

static inline void TRCH_ClockSource2(void)
{
	if( TRCH_SamplingService_ClockCallback[2] )
	{
		TRCH_EventService_Process();
		TRCH_SamplingService_ClockCallback[2]();
	}
	if( TRCH_StreamingService_ClockCallback[2] )
	{
		TRCH_StreamingService_ClockCallback[2]();
	}
}

static inline void TRCH_ClockSource3(void)
{
	if( TRCH_SamplingService_ClockCallback[3] )
	{
		TRCH_EventService_Process();
		TRCH_SamplingService_ClockCallback[3]();
	}
	if( TRCH_StreamingService_ClockCallback[3] )
	{
		TRCH_StreamingService_ClockCallback[3]();
	}
}

#endif /* _dti_tracehandler_api_h */
