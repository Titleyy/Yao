/**********************************************************************************************************************
 * File:  linklayer.c	
 * RqID:  
 * Brief: Linklayer for target interface
 * $Id: 6a4ead122458e5c3f367b2711714b6140507da0d $
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
#include <stdint.h>
#include <stdbool.h>
#include "linklayer.h"

/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
void DTI_linklayer_init( void )
{
	/* do nothing */
}

bool DTI_linklayer_is_valid_FCS( const DTI_frame_t *frame )
{
	uint32_t i;
	DTI_frame_FCS_t fcs;
	
	fcs = 0;
	for( i = 0; i < (uint32_t)frame->payloadsize + 1; ++i)
	{
		fcs += 	frame->data8[i];
	}
	fcs += frame->FTS;

	return fcs == frame->FCS;
}

void DTI_linklayer_TX(DTI_frame_t *framebuffer)
{
	uint32_t byte;

	framebuffer->FTS = DTI_getFTSvalue();
	framebuffer->FCS = 0;
	++framebuffer->frameCount;
	framebuffer->FCNT = framebuffer->frameCount & 0x03;
//	framebuffer->FCTRL = 0;

	/* begin frame transmission */
	DTI_linklayer_transmit_start();

	/* transmit header and payload data block */
	for (uint32_t i = 0; i < framebuffer->payloadsize + 1UL; ++i) //TODO: size calculation could be done at initialization (api for set payloadsize)
	{
		byte = (*((uint8_t*)framebuffer + i));
		DTI_linklayer_transmit(byte);
		framebuffer->FCS += byte; // calculate checksum
	}

	/* transmit frame time stamp FTS */
	DTI_linklayer_transmit(framebuffer->FTS);
	framebuffer->FCS += framebuffer->FTS;

	/* transmit frame checksum FCS */
	DTI_linklayer_transmit(framebuffer->FCS);

	/* finalize frame transmission */
	DTI_linklayer_transmit_stop();
}

DTI_linklayer_UART_RX_state_t DTI_linklayer_RX(DTI_frame_t *framebuffer)
{
	uint32_t byte; 
	static uint8_t *pointer;
	static DTI_linklayer_UART_RX_state_t state = DTI_UART_RX_state_WaitForStart;
	DTI_linklayer_UART_RX_state_t returnvalue = state;
	
	if(DTI_phylayer_bytesToRead()) 
	{
		byte = DTI_phylayer_read();
		if( state == DTI_UART_RX_state_WaitForStart )
		{
				pointer = (uint8_t*)framebuffer;

				while(1)
				{ /* search for byte after start of frame */
					if( byte != 0x7E )
					{
						returnvalue = state = DTI_UART_RX_state_ReceivingFrame;
						break;
					}
					else {/* continue */}
					
					if(DTI_phylayer_bytesToRead() > 1UL)
					{
						byte = DTI_phylayer_read();
					}
					else
					{ 
						break;
					}
				}
		}

		if (state == DTI_UART_RX_state_ReceivingFrame7D)
		{ /* conclude the reception of an escaped character */
			*pointer++ = (byte ^ 0x20);
			state = DTI_UART_RX_state_ReceivingFrame;
		}
		else
		if( state == DTI_UART_RX_state_ReceivingFrame )
		{
			while(1)
			{ 
				if(( byte == 0x7E ) || (pointer > ((uint8_t*)framebuffer) + DTI_FRAME_SIZE_MAX))
				{
					state = DTI_UART_RX_state_FrameReceived;
					break;
				}
				else {/* continue */}
					
				if( byte == 0x7D)
				{
					if (DTI_phylayer_bytesToRead())
					{ 
						byte = DTI_phylayer_read();
						*pointer++ = (byte ^ 0x20);
					}
					else
					{ /* prepare the reception of an escaped character */
						state = DTI_UART_RX_state_ReceivingFrame7D;
						break;
					}

				}
				else
				{
					*pointer++ = byte;
				}

				if (DTI_phylayer_bytesToRead())
				{
					byte = DTI_phylayer_read();
				}
				else
				{
					break;
				}
			}
		}
	
		if( state == DTI_UART_RX_state_FrameReceived )
		{
			framebuffer->payloadsize = (uint8_t)(( pointer - (uint8_t*)framebuffer ) - (1 /* frame header */ + sizeof(DTI_frame_FTS_t) + sizeof(DTI_frame_FCS_t)));
			framebuffer->FCS = framebuffer->data8[framebuffer->payloadsize + 2]; /* copy FCS from beyond last data byte + 1 */
			framebuffer->FTS = framebuffer->data8[framebuffer->payloadsize + 1]; /* copy FTS from beyond last data byte */
			
			if( DTI_linklayer_is_valid_FCS(framebuffer) )
			{ /* frame is valid */
				if( framebuffer->FCNT == (framebuffer->frameCount & 0x03) )
				{ /* framecount is valid */
					++framebuffer->frameCount;
				}
				else
				{ /* framecount ERROR */
					framebuffer->frameCount &= ~0x03UL;
					framebuffer->frameCount += framebuffer->FCNT - 3; /* this makes sure that the new framecount number is less than the actual frame count number */
					++framebuffer->errorCount;
				}
				
				returnvalue = DTI_UART_RX_state_FrameReceived;
			}
			else
			{ /* frame is invalid */
				returnvalue = DTI_UART_RX_state_WaitForStart;
				++framebuffer->errorCount;
			}
			
			state = DTI_UART_RX_state_WaitForStart;
		}
	}
	else {/* receive buffer is empty */ }

	return (returnvalue);
}
