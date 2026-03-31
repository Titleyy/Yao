/**********************************************************************************************************************
 * File:  router.h	
 * RqID:  
 * Brief: Data router for target interface
 * $Id: 98aef3090cd8437937618b8cdac75b2998767f73 $
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


#ifndef _DTI_router_h
#define _DTI_router_h

#ifdef __cplusplus 
extern "C" {
#endif

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include "arbiter.h"
#include "dti_conf.h"
#include "signalHandler_api.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#define DTI_ARBITERLONG_MAX (sizeof(DTI_arbiter_data_t) + sizeof(DTI_stream_channel_t)/sizeof(DTI_arbiter_data_t) * DTI_STREAM_CHANNEL_MAX)

/**********************************************************************************************************************
* ENUMS and TYPEDEFS
**********************************************************************************************************************/
typedef enum
{
	DTI_channel0 = 0
} DTI_channels_e;

typedef enum {
	DTI_router_noStreaming   = 0,
	DTI_router_withStreaming = 1
} DTI_router_MD_e;

typedef uint16_t DTI_stream_channel_t;

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
typedef struct {
	struct {
		uint8_t APNR : 2;
		uint8_t AO : 1;
		uint8_t MO : 1;
	};
	DTI_arbiter_data_t ADATA[DTI_ARBITERLONG_MAX];
} DTI_arbiterLong_t;

typedef union {
	struct{
		DTI_arbiter_t arbiter;
		DTI_stream_channel_t CH[DTI_STREAM_CHANNEL_MAX];
	};
	DTI_arbiterLong_t arbiterLong;
} DTI_router_t;

typedef struct {
	DTI_stream_channel_t ch[DTI_STREAM_CHANNEL_MAX]; /* buffer for channel values */
	uint32_t time;
	uint8_t FTS;
} DTI_stream_t;

typedef struct {
		DTI_stream_channel_t* ptr;
		TRCH_SignalType_t type;
} DTI_stream_channelmap_t;

typedef struct {
	DTI_router_t *ptr_DD; /* pointer to DD buffer */
	DTI_router_t *ptr_DU; /* pointer to DU buffer */
	DTI_stream_t *stream_RX; /* pointer to the RX stream buffer (destination) */
	void (*callback_RX)(void); /* function is called after stream data has been processed */
	DTI_stream_channelmap_t ch_TX[DTI_STREAM_CHANNEL_MAX+1]; /* zero terminated array of channelmap structures (source) */
	uint32_t arbiterdatasize; /* size of received and transmitted data block in arbiter mode without streaming */
	uint8_t stream_resolution; /* number of bits per streaming channel */
	uint8_t stream_numCh; /* number of streaming channels */
} DTI_router_cnf_t;


/***********************************************************************************************************************
 * static inline functions
 **********************************************************************************************************************/
static inline uint32_t get_chvalue(DTI_stream_channelmap_t *ch)
{
	uint32_t returnvalue;
	
	switch( ch->type )
	{
		case TRCH_SignalType_8bit:
			 returnvalue = *(uint8_t*)ch->ptr;
			break;
		
		case TRCH_SignalType_16bit:
			returnvalue =  *(uint16_t*)ch->ptr;
			break;
		
		case TRCH_SignalType_32bit:
			returnvalue = *(uint32_t*)ch->ptr;
			break;
		
		default:
			break;
	}

	return returnvalue;
}

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/
void DTI_router_init(void);

DTI_arbiter_t* DTI_router_arbiterptr_DU(void);
DTI_arbiter_t* DTI_router_arbiterptr_DD(void);

void DTI_router_setMD_DU(DTI_router_MD_e md);
void DTI_router_setMD_DD(DTI_router_MD_e md);
void DTI_router_setNumberChannels_DU(uint32_t numCh, uint32_t resolution);
uint8_t DTI_router_getNumberChannels(void);
uint8_t DTI_router_getStreamResolution(void);

void DTI_router_setChannelPtr_TX(DTI_channels_e channel, DTI_stream_channel_t *ptr, TRCH_SignalType_t type);
void DTI_router_setStreamPtr_RX(DTI_stream_t *ptr);

void DTI_router_setCallbackStreamRX( void (*callback)(void) );

bool DTI_Routing_DU(void);
bool DTI_Routing_DD(void);

/* input: a pointer to an array of pointers which point to the data */
bool DTI_router_chvalues_TX(DTI_router_t *router, DTI_stream_channelmap_t *ch_tx, uint8_t resolution);
/* input: a pointer to a buffer which is ready to store the received data */
void DTI_router_chvalues_RX(DTI_router_t *router, DTI_router_cnf_t *router_cnf);

#ifdef __cplusplus 
}
#endif

#endif /*_DTI_router_h*/
