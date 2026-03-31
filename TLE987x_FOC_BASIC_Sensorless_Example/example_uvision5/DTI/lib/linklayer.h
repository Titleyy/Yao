/**********************************************************************************************************************
 * File:  linklayer.h	
 * RqID:  
 * Brief: Linklayer for target interface
 * $Id: 57b5144c3ed97b3f25e52fe55823f48f59f2448f $
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

#ifndef _DTI_linklayer_h
#define _DTI_linklayer_h

#ifdef __cplusplus 
extern "C" {
#endif

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include <stdint.h>
#include "router.h"
#include "TSdriver.h"
#include "PHYdriver.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#define DTI_FRAME_DATA_SIZE_MAX ( \
	  sizeof(DTI_arbiter_t) \
	+ sizeof(DTI_stream_channel_t) * DTI_STREAM_CHANNEL_MAX \
)

	#define DTI_FRAME_SIZE_MAX ( \
	+ DTI_FRAME_DATA_SIZE_MAX \
	+ sizeof(DTI_frame_FTS_t) \
	+ sizeof(DTI_frame_FCS_t) \
)

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/
typedef enum {
	DTI_FCTRL_connect,
	DTI_FCTRL_data,
	DTI_FCTRL_2,
	DTI_FCTRL_3,
} DTI_linklayer_FCTRL_e;


typedef enum {
	DTI_UART_RX_state_FrameReceived,
	DTI_UART_RX_state_WaitForStart,
	DTI_UART_RX_state_ReceivingFrame,
	DTI_UART_RX_state_ReceivingFrame7D,
} DTI_linklayer_UART_RX_state_t;

typedef enum {
	DTI_DataTransmission,
	DTI_LineTest,
	DTI_SyncFrame,
	DTI_unused
} DTI_FCTRL_e;
/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
typedef uint8_t DTI_frame_FTS_t;
typedef uint8_t DTI_frame_FCS_t;

typedef struct
{
	union {
		uint8_t data8[DTI_FRAME_DATA_SIZE_MAX];
		uint16_t data16[(DTI_FRAME_DATA_SIZE_MAX+1)/2];
		uint32_t data32[(DTI_FRAME_DATA_SIZE_MAX)/4];
		struct {
			uint8_t reserved:4;
			uint8_t FCTRL:2;
			uint8_t FCNT:2;
		};
	};
	DTI_frame_FTS_t FTS;
	DTI_frame_FCS_t FCS;
	uint8_t payloadsize;
	uint32_t errorCount;
	uint32_t frameCount;
} DTI_frame_t;

typedef struct {
	DTI_frame_t frame_DU;
	DTI_frame_t frame_DD;
} DTI_linklayer_t;

/***********************************************************************************************************************
* EXTERN DECLARATIONS
***********************************************************************************************************************/
extern DTI_linklayer_t DTI_buffer;

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/
void DTI_linklayer_init( void );
void DTI_linklayer_TX(DTI_frame_t *framebuffer);
DTI_linklayer_UART_RX_state_t DTI_linklayer_RX(DTI_frame_t *framebuffer);
bool DTI_linklayer_is_valid_FCS( const DTI_frame_t *frame );

/***********************************************************************************************************************
 * static inline Functions
 **********************************************************************************************************************/
static inline void* DTI_linklayer_dataptr_DU(void)
{
	return &DTI_buffer.frame_DU.data8;
}

static inline void* DTI_linklayer_dataptr_DD(void)
{
	return &DTI_buffer.frame_DD.data8;
}

static inline uint32_t DTI_linklayer_getPayloadSize_DU(void)
{
	return DTI_buffer.frame_DU.payloadsize;
}

static inline void DTI_linklayer_setPayloadSize_DU(uint32_t size)
{
	DTI_buffer.frame_DU.payloadsize = size;
}

static inline uint32_t DTI_linklayer_getPayloadSize_DD(void)
{
	return DTI_buffer.frame_DD.payloadsize;
}

static inline void DTI_linklayer_setPayloadSize_DD(uint32_t size)
{
	DTI_buffer.frame_DD.payloadsize = size;
}

static inline DTI_frame_FTS_t DTI_getFTSvalue(void)
{
	return DTI_TSdriver_getTimeStamp();
}

static inline uint32_t DTI_linklayer_getFCNT( DTI_frame_t *frame)
{
	return frame->FCNT;
}

static inline DTI_frame_FTS_t DTI_linklayer_getFTS(DTI_frame_t *frame)
{
	return frame->FTS;
}

static inline void DTI_linklayer_transmit(uint32_t data)
{
	if ((data == 0x7E) || (data == 0x7D))
	{
		DTI_phylayer_write(0x7D);
		DTI_phylayer_write(data ^ 0x20);
	}
	else
	{
		DTI_phylayer_write(data);
	}
}

static inline void DTI_linklayer_transmit_start(void)
{
	DTI_phylayer_write(0x7E);
}

static inline void DTI_linklayer_transmit_stop(void)
{
	DTI_phylayer_write(0x7E);
}

#ifdef __cplusplus 
}
#endif

#endif /*_DTI_linklayer_h*/

