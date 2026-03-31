/**********************************************************************************************************************
 * File:  router.c	
 * RqID:  
 * Brief: Data router for target interface
 * $Id: 93cd4877fd651c60759d4dd1e84a21f4bef6125b $
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
#include "dti_conf.h"
#include "router.h"
#include "linklayer.h"

/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
DTI_router_cnf_t DTI_router;

/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
 void DTI_router_init(void)
{
	DTI_router.ptr_DU = DTI_linklayer_dataptr_DU();
	DTI_router.ptr_DD = DTI_linklayer_dataptr_DD();

	DTI_router.ptr_DU->arbiter.MO = DTI_router_noStreaming ;
	DTI_router.ptr_DD->arbiter.MO = DTI_router_noStreaming;
}

void DTI_router_setMD_DU(DTI_router_MD_e md)
{
	DTI_router.ptr_DU->arbiter.MO = md;
}

void DTI_router_setMD_DD(DTI_router_MD_e md)
{
	DTI_router.ptr_DD->arbiter.MO = md;
}

bool DTI_Routing_DU(void)
{
	uint32_t returnvalue;

#ifdef DTI_TARGET
	returnvalue = DTI_router_chvalues_TX(DTI_router.ptr_DU, DTI_router.ch_TX, DTI_router.stream_resolution);
	returnvalue |= DTI_arbiter_Serialize_TX(&DTI_router.ptr_DU->arbiter, DTI_linklayer_getPayloadSize_DU());
#else
	returnvalue = true;
	DTI_arbiter_Deserialize_RX( &DTI_router.ptr_DU->arbiter, DTI_linklayer_getPayloadSize_DU());
	if (DTI_router.ptr_DU->arbiter.MO)
	{
		DTI_router_chvalues_RX(DTI_router.ptr_DU, &DTI_router);
	}
#endif

	return returnvalue;
}

bool DTI_Routing_DD(void)
{
	uint32_t returnvalue;
	
#ifdef DTI_TARGET
	returnvalue = true;
	DTI_arbiter_Deserialize_RX(&DTI_router.ptr_DD->arbiter, DTI_linklayer_getPayloadSize_DD());
	if (DTI_router.ptr_DD->arbiter.MO)
	{
		DTI_router_chvalues_RX(DTI_router.ptr_DD, &DTI_router);
	}
#else
	returnvalue = DTI_router_chvalues_TX(DTI_router.ptr_DD, DTI_router.ch_TX, DTI_router.stream_resolution);
	returnvalue |= DTI_arbiter_Serialize_TX(&DTI_router.ptr_DD->arbiter, DTI_linklayer_getPayloadSize_DD());
#endif
	return returnvalue;
}

DTI_arbiter_t* DTI_router_arbiterptr_DU(void)
{
	return &DTI_router.ptr_DU->arbiter;
}

DTI_arbiter_t* DTI_router_arbiterptr_DD(void)
{
	return &DTI_router.ptr_DD->arbiter;
}

void DTI_router_setChannelPtr_TX(DTI_channels_e channel, DTI_stream_channel_t *ptr, TRCH_SignalType_t type)
{
	DTI_router.ch_TX[channel].ptr = ptr;
	DTI_router.ch_TX[channel].type = type;
}

void DTI_router_setStreamPtr_RX(DTI_stream_t *ptr)
{
	DTI_router.stream_RX = ptr;
}

void DTI_router_setCallbackStreamRX( void (*callback)(void) )
{
	DTI_router.callback_RX = callback;
}

bool DTI_router_chvalues_TX(DTI_router_t *router, DTI_stream_channelmap_t *ch_tx, uint8_t resolution)
{
	bool returnvalue;
	uint8_t *destinationptr;

	returnvalue = ch_tx->ptr; /* return false, if there is no data to stream */
	destinationptr = (uint8_t*)&router->CH[0]; /* use this memory bytewise */
	
	switch ( resolution )
	{
		case 8:
			while( ch_tx->ptr )
			{
				*destinationptr = (get_chvalue(ch_tx)) >> 8;
				++destinationptr;
				++ch_tx;
			}
			break;
		
		case 12:
			while( ch_tx->ptr )
			{
				*((uint16_t*)destinationptr) = ((uint16_t)get_chvalue(ch_tx))>>4;                   // ch[0]            -> destination[0], destination[1]
				++ch_tx;                                                                   // 
				
				if( ch_tx->ptr )
				{
					++destinationptr;
					*destinationptr |= (0xF0U & (((uint16_t)get_chvalue(ch_tx))>>8));                   // ch[1] highbyte.4 -> destination[1] remaining 4bit 
					++destinationptr;
					*destinationptr = (((uint16_t)get_chvalue(ch_tx))>>4);                            // ch[1] lowbyte    -> destination[2]
					++ch_tx;
				}
				if( ch_tx->ptr )
				{
					++destinationptr;
					*destinationptr = (((uint16_t)get_chvalue(ch_tx))>>4);                            // ch[2] lowbyte    -> destination[3]
					++destinationptr;
					*destinationptr = (((uint16_t)get_chvalue(ch_tx))>>12);                           // ch[2] highbyte   -> destination[4]
					++ch_tx;
				}
				if( ch_tx->ptr )
				{
					*destinationptr |= (0xF0U & ((get_chvalue(ch_tx))>>8));                   // ch[3] highbyte.4 -> destination[4] remaining 4bit 
					++destinationptr;
					*destinationptr = (((uint16_t)get_chvalue(ch_tx))>>4);                            // ch[3] lowbyte    -> destination[5]
					++destinationptr;
					++ch_tx;
				}
			}
			break;
		
		case 16:
			while( ch_tx->ptr )
			{
				*((uint16_t*)destinationptr) = get_chvalue(ch_tx);
				destinationptr += 2;
				++ch_tx;
			}
			break;
		
		case 32: // ARNO TODO: some workaround for 32bit channels - maybe not good
			while( ch_tx->ptr )
			{
				uint32_t value = get_chvalue(ch_tx);
				*((uint16_t*)destinationptr) = value & 0x0000FFFFUL;
				destinationptr += 2;
				*((uint16_t*)destinationptr) = value >> 16;
				destinationptr += 2;
				++ch_tx;
			}
			break;
		
		default:
			/* do nothing */
			break;
	}
	
	return returnvalue;
}

void DTI_router_chvalues_RX(DTI_router_t *router, DTI_router_cnf_t* router_cnf)
{
	uint8_t *sourceptr;
	DTI_stream_channel_t *ch_rx;
	uint8_t numberCh;
	bool supportedResolution;

	sourceptr = (uint8_t*)&router->CH[0];  /* use this memory bytewise */
	ch_rx = router_cnf->stream_RX->ch;
	numberCh = router_cnf->stream_numCh;
	supportedResolution = true;
	
	switch ( router_cnf->stream_resolution )
	{
		case 8:
			while( numberCh-- )
			{
				*ch_rx = ((uint16_t)(*sourceptr))<<8;
				sourceptr += 1;
				++ch_rx;
			}
			break;

		case 12:
			while( numberCh-- )
			{
				*ch_rx = (*(uint16_t*)sourceptr)<<4;                                                   // ch[0]            <- source[0], source[1]
				++ch_rx;
				
				if( numberCh-- )
				{
					++sourceptr;
					*ch_rx = (((uint16_t)(*sourceptr))&0x00F0U) << 8;                                    // ch[1] highbyte.4 <- source[1] highest 4bit 
					++sourceptr;
					*ch_rx |= ((uint16_t)(*sourceptr)) << 4;                                            // ch[1] low part   <- source[2]
					++ch_rx;
				}
				if( numberCh-- )
				{
					++sourceptr;
					*ch_rx = ((uint16_t)(*sourceptr))<<4;                                               // ch[2] low part   <- source[3]
					++sourceptr;
					*ch_rx |= ((uint16_t)(*sourceptr))<<12;                                             // ch[2] highbyte   <- source[4]
					++ch_rx;
				}
				if( numberCh-- )
				{
					*ch_rx = (((uint16_t)(*sourceptr))&0x00F0U) << 8;                                      // ch[3] highbyte.4 <- source[4] highest 4bit 
					++sourceptr;
					*ch_rx |= ((uint16_t)(*sourceptr)) << 4;                                            // ch[3] low part   <- source[5]
					++sourceptr;
					++ch_rx;
				}
			}
			break;
			
			
		case 16:
			while( numberCh-- )
			{
				*ch_rx = *((uint16_t*)sourceptr);
				sourceptr += 2;
				++ch_rx;
			}
			break;

		case 32: // ARNO TODO: some workaround for 32bit channels - maybe not good
			while( numberCh-- )
			{
				*ch_rx = *((uint16_t*)sourceptr);
				sourceptr += 2;
				++ch_rx;
				*ch_rx = *((uint16_t*)sourceptr);
				sourceptr += 2;
				++ch_rx;
			}
			break;

		default:
			/* this resolution is not supported */
			supportedResolution = false;
			break;
	}
	
	if( DTI_router.callback_RX && supportedResolution )
	{
		DTI_router.callback_RX();
	}
	else {/* do nothing */}
}

uint8_t DTI_router_getNumberChannels(void)
{
	return DTI_router.stream_numCh;
}

uint8_t DTI_router_getStreamResolution(void)
{
	return DTI_router.stream_resolution;
}


void DTI_router_setNumberChannels_DU(uint32_t numCh, uint32_t stream_resolution)
{

	switch (stream_resolution)
	{
		case 8:
			DTI_linklayer_setPayloadSize_DU(sizeof(DTI_arbiter_data_t) + sizeof(DTI_stream_channel_t) * (numCh / 2));
			DTI_router.stream_resolution = stream_resolution;
			DTI_router.stream_numCh = numCh;
		break;
		
		case 12:
			DTI_linklayer_setPayloadSize_DU(sizeof(DTI_arbiter_data_t) + sizeof(DTI_stream_channel_t) * (numCh - numCh/4));
			DTI_router.stream_resolution = stream_resolution;
			DTI_router.stream_numCh = numCh;
		break;
		
		case 16:
			DTI_linklayer_setPayloadSize_DU(sizeof(DTI_arbiter_data_t) + sizeof(DTI_stream_channel_t) * numCh);
			DTI_router.stream_resolution = stream_resolution;
			DTI_router.stream_numCh = numCh;
		break;
		
		case 32:
			DTI_linklayer_setPayloadSize_DU(sizeof(DTI_arbiter_data_t) + sizeof(DTI_stream_channel_t) * numCh * 2);
			DTI_router.stream_resolution = stream_resolution;
			DTI_router.stream_numCh = numCh * 2;
		break;
		
		default:
			/* do nothing */
			break;
	}
}
