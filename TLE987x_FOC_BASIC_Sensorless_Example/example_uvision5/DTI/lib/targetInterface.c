/**********************************************************************************************************************
 * File:  targetInterface.c	
 * RqID:  
 * Brief: target interface functions
 * $Id: 1481585b9c19c506cdef56e1e1789a61c32bcbd1 $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-25:
 *     - renamed interface_api.c into targetInterface.c,	because this is not the API
 *     - clean from test code
 * 2020-April-30:
 *     - Initial version
 *
 */
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include "targetInterface.h"
#include "dti.h"
//#include "DTI/PHYdriver.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
DTI_linklayer_t DTI_buffer = {
	.frame_DU.payloadsize = 1 + 2*DTI_STREAM_CHANNEL_MAX,
	.frame_DD.payloadsize = 1 + 2*DTI_STREAM_CHANNEL_MAX
};

DTI_vt DTI_var = {
	.ErrorCount = 0,
	.FrameCount = 0,
	.ClockDivider = 0, /* interface processed in background at every call, clock rate call backs cleared */
};

/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
void DTI_interface_init( void )
{
	DTI_arbiter_init();
	DTI_router_init();
	DTI_linklayer_init();
	DTI_phylayer_init();
}

void DTI_setMD_DU(DTI_router_MD_e md)
{
	DTI_router_setMD_DU( md );
}

void DTI_setChannelPtr_TX(DTI_channels_e channel, DTI_stream_channel_t *ptr, TRCH_SignalType_t type)
{
	DTI_router_setChannelPtr_TX( channel, ptr, type);
}

void DTI_setNumberChannels_DU(uint32_t numCh, uint32_t resolution)
{
	DTI_router_setNumberChannels_DU( numCh, resolution );
}

void DTI_setClockDivider_DU(uint32_t clkdiv)
{
	DTI_var.ClockDivider = clkdiv;
}

uint8_t DTI_getClockDivider_DU(void)
{
	return DTI_var.ClockDivider;
}


void DTI_interface_DU( void )
{
	DTI_Arbitration_TX( DTI_router_arbiterptr_DU() ); /* optionally as task */
	
	if( DTI_Routing_DU() )
	{
		DTI_linklayer_TX(&DTI_buffer.frame_DU);
	}
	else {/*do nothing*/}
}

void DTI_sendSyncFrame_DU(void)
{
	DTI_buffer.frame_DU.FCTRL = DTI_SyncFrame;
	DTI_buffer.frame_DU.data8[1]  = DTI_TSdriver_getTimeStampScale();
	DTI_buffer.frame_DU.data16[1] = DTI_TSdriver_getTargetTimeScale();
	DTI_buffer.frame_DU.data32[1] = DTI_TSdriver_getTargetTime();
	DTI_linklayer_TX(&DTI_buffer.frame_DU);
	
	DTI_buffer.frame_DU.FCTRL = DTI_DataTransmission;
}

void DTI_interface_DD( void )
{
	if( DTI_UART_RX_state_FrameReceived == DTI_linklayer_RX(&DTI_buffer.frame_DD) )
	{ 
		DTI_Routing_DD();
	}
	else {/*do nothing*/}
}

void DTI_interface_ProcessStreaming(void)
{
	static int32_t counter = 1;
	
	if( (--counter) <= 0 )
	{
		counter = DTI_var.ClockDivider;
		DTI_interface_DU(); /* send response */
		DTI_interface_DD(); /* receive request */
	}
	else {/*do nothing*/}
}

void DTI_interface_Process(void)
{
	if( !DTI_var.ClockDivider )
	{
		DTI_interface_DU(); /* send response */
		DTI_interface_DD(); /* receive request */
	}
	else {/*do nothing*/}
}
