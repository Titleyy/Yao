/**********************************************************************************************************************
 * File:  sdoService.c	
 * RqID:  
 * Brief: SDO Service for target interface
 * $Id: 29a1cc440407533deb7a86106e0b14be19288eba $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-April-30:
 *     - Initial version
 * 2020-September-30:
 *     - SDO Service: Fix number of bytes in SDOSERVICE_download_initiate()
 *
 */
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				

#include <stddef.h>
#include "sdoService.h"
//#include "../parameterHandler.h"
#include "dti_conf.h"
#include "parameterHandler_api.h"

/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
SDOSERVICE_vt SDOSERVICE_var;

/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
void SDOSERVICE_download_initiate( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response )
{
	uint32_t value;
	uint32_t idx_target;
	uint16_t num_bytes;
	
	response->head = request->head; /* copy index and subindex together */
	SDOSERVICE_var.dataptr = NULL;

	if( (request->initiate.cmd & (SDOSERVICE_CMD_E_MSK|SDOSERVICE_CMD_S_MSK)) == ((SDOSERVICE_CMD_E_EXPEDITED<<SDOSERVICE_CMD_E_POS) | (SDOSERVICE_CMD_S_SIZE<<SDOSERVICE_CMD_S_POS)) )
	{ /* expedited transfer */
		num_bytes = 4 - ((request->initiate.cmd & SDOSERVICE_CMD_INITIATE_N_MSK) >> SDOSERVICE_CMD_INITIATE_N_POS);
		idx_target = request->initiate.subindex + (request->initiate.index_lo<<8) + (request->initiate.index_hi << 16);
		value = request->initiate.data32;
		
//		if( !set(idx_target, num_bytes, &value, getParameterFifoSelection()) )
		if( !PARH_set(idx_target, num_bytes, &value) )
		{
			if( value == request->initiate.data32 )
			{ /* value OK */
				response->cmd = SDOSERVICE_CMD_SCS_DOWNLOAD_INITIATED << SDOSERVICE_CMD_SCS_POS;
				response->initiate.data32 = 0;
			}
			else
			{ /* value invalid */
				response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
				response->initiate.data32 = SDOSERVICE_AB_INVALID_VALUE;
			}
		}
		else
		{ /* set not successful */
			response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
			response->initiate.data32 = SDOSERVICE_AB_UNSUPPORTED_ACCESS;
		}
	}
	else if( (request->initiate.cmd & (SDOSERVICE_CMD_E_MSK|SDOSERVICE_CMD_S_MSK)) == ((SDOSERVICE_CMD_E_NORMAL<<SDOSERVICE_CMD_E_POS) | (SDOSERVICE_CMD_S_SIZE<<SDOSERVICE_CMD_S_POS)) )
	{ /* normal transfer */
		idx_target = request->initiate.subindex + (request->initiate.index_lo<<8) + (request->initiate.index_hi << 16);
		if( request->initiate.data32 > PARH_paramMaxNumberBytes(idx_target) )
		{ /* requested number of bytes exceeds allowed size */
			response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
			response->initiate.data32 = SDOSERVICE_AB_DATA_LONG;
		}
		else
		{
			switch( PARH_paramAddress(idx_target, (void*) &SDOSERVICE_var.dataptr ))
			{ 
				case PARH_Errors_No_Error: /* OK, segmented transfer is initiated */
					SDOSERVICE_var.bytecount = request->initiate.data32;
					response->cmd = SDOSERVICE_CMD_SCS_DOWNLOAD_INITIATED << SDOSERVICE_CMD_SCS_POS;
					SDOSERVICE_var.head = response->head; /* index and subindex (not required) */
					SDOSERVICE_var.cmd = SDOSERVICE_CMD_CCS_DOWNLOAD_SEGMENT<<SDOSERVICE_CMD_SCS_POS; /* expected command and toggle-bit value */
					response->initiate.data32 = 0;
					break;
				
				case PARH_Errors_Idx_Unknown:
					response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
					response->initiate.data32 = SDOSERVICE_AB_NOT_EXIST;
					break;
				
				case PARH_Errors_Not_Allowed:
					response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
					response->initiate.data32 = SDOSERVICE_AB_UNSUPPORTED_ACCESS;
					break;
		 
				default: /* parameter error, most likely it does not exist */ 
					response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
					response->initiate.data32 = SDOSERVICE_AB_NOT_EXIST;
				break;
			}
		}
	}
	else 
	{ /* not supported */ 
			response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
			response->initiate.data32 = SDOSERVICE_AB_CMD;
	}
}

void SDOSERVICE_download_segment( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response )
{
	uint32_t loop;
	uint32_t number_bytes;
	
	if( SDOSERVICE_var.dataptr )
	{
		if( (request->cmd & ~(SDOSERVICE_CMD_SEGMENTED_N_MSK | SDOSERVICE_CMD_C_MSK) ) == SDOSERVICE_var.cmd )
		{
			number_bytes = 7-((request->cmd & SDOSERVICE_CMD_SEGMENTED_N_MSK)>>SDOSERVICE_CMD_SEGMENTED_N_POS);
			if( SDOSERVICE_var.bytecount < number_bytes )
			{ /* too many bytes of data */
				response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
				response->initiate.data32 = SDOSERVICE_AB_OUT_OF_MEM;
			}
			else
			{
				response->head = 0;
				response->tail= 0;
				response->cmd = (SDOSERVICE_CMD_SCS_DOWNLOAD_SEGMENT<<SDOSERVICE_CMD_SCS_POS)| (SDOSERVICE_var.cmd & SDOSERVICE_CMD_T_MSK); /* response with same toggle-bit value */
				SDOSERVICE_var.cmd ^= SDOSERVICE_CMD_T_MSK; /* toggle expected toogle-bit for next segment */
				loop = 0;
				while( SDOSERVICE_var.bytecount && (loop < number_bytes) )
				{
					--SDOSERVICE_var.bytecount;
					*SDOSERVICE_var.dataptr++ = request->segment.data8[loop++];
				}
				if( SDOSERVICE_var.bytecount == 0 || (request->cmd & SDOSERVICE_CMD_C_MSK) )
				{
					SDOSERVICE_var.dataptr = NULL;
				}
				else{/* do nothing, default is "more segments to process" */}				
			}
		}
		else
		{ /* wrong toggle-bit value */
				response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
				response->initiate.data32 = SDOSERVICE_AB_TOGGLE_BIT;
		}
	}
	else
	{ /* segmented transfer was not initiated */
			response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
			response->initiate.data32 = SDOSERVICE_AB_CMD;
	}
}


void SDOSERVICE_upload_initiate( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response )
{
	uint32_t value;
	uint32_t idx_target;
	uint32_t num_bytes;
	
	num_bytes = 0;
	SDOSERVICE_var.dataptr = NULL;
	
	response->head = request->head; /* copy index and subindex together */
	idx_target = request->initiate.subindex + (request->initiate.index_lo<<8) + (request->initiate.index_hi << 16);
	
	if( (request->initiate.cmd & SDOSERVICE_CMD_S_MSK) == (SDOSERVICE_CMD_S_NOSIZE<<SDOSERVICE_CMD_S_POS) )
	{ /* standard SDO Upload transfer */
		if( (0UL == request->initiate.data32) && (request->initiate.cmd & (SDOSERVICE_CMD_INITIATE_N_MSK|SDOSERVICE_CMD_E_MSK)) == (SDOSERVICE_CMD_E_NORMAL << SDOSERVICE_CMD_E_POS) )
		{ /* standard transfer */
			num_bytes = PARH_paramMaxNumberBytes(idx_target);
		}
		else
		{ /* not supported */
			num_bytes = 0;
			response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
			response->initiate.data32 = SDOSERVICE_AB_NOT_EXIST;
		}
	}
	else
	{ /* SAMBA extension, client can request the number of bytes to be transferred */
		num_bytes = request->initiate.data32;
		if( num_bytes > PARH_paramMaxNumberBytes(idx_target) )
		{
			num_bytes = 0;
			response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
			response->initiate.data32 = SDOSERVICE_AB_DATA_LONG;
		}	
		else {/* do nothing*/}
	}
	
	if( num_bytes )
	{ /* transfer is not aborted, prepare successful response */
		if( num_bytes < 5 )
		{ /* response as expedited transfer */
			if( !PARH_get(idx_target, num_bytes, &value) )
			{
				response->cmd = 
						(SDOSERVICE_CMD_SCS_UPLOAD_INITIATED << SDOSERVICE_CMD_SCS_POS) | 
						(((4-num_bytes) << SDOSERVICE_CMD_INITIATE_N_POS) & SDOSERVICE_CMD_INITIATE_N_MSK ) |
						(SDOSERVICE_CMD_E_EXPEDITED << SDOSERVICE_CMD_E_POS) |
						(SDOSERVICE_CMD_S_SIZE << SDOSERVICE_CMD_S_POS) ;
				response->initiate.data32 = value & (0xffffffff >> (8*(4-num_bytes)));
			}
			else 
			{ /* get parameter was not successful */
				response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
				response->initiate.data32 = SDOSERVICE_AB_UNSUPPORTED_ACCESS;
			}
		} 
		else
		{ /* response as normal transfer*/
			/* get pointer and check if available */
			switch( PARH_paramAddress(idx_target, (void*)&SDOSERVICE_var.dataptr ))
			{ 
				case PARH_Errors_No_Error: /* OK, segmented transfer is initiated */
					SDOSERVICE_var.bytecount = num_bytes; /* store number of bytes to be transfered in total */
					SDOSERVICE_var.head = response->head; /* store index and subindex (not required) */
					SDOSERVICE_var.cmd = SDOSERVICE_CMD_CCS_UPLOAD_SEGMENT<<SDOSERVICE_CMD_SCS_POS; /* store expected command and toggle-bit value (t=0) */
					response->cmd = 
						(SDOSERVICE_CMD_SCS_UPLOAD_INITIATED << SDOSERVICE_CMD_SCS_POS) | 
						(SDOSERVICE_CMD_E_NORMAL << SDOSERVICE_CMD_E_POS) |
						(SDOSERVICE_CMD_S_SIZE << SDOSERVICE_CMD_S_POS) ;
					response->initiate.data32 = num_bytes;
					break;
				
				case PARH_Errors_Idx_Unknown:
					response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
					response->initiate.data32 = SDOSERVICE_AB_NOT_EXIST;
					break;
				
				case PARH_Errors_Not_Allowed:
					response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
					response->initiate.data32 = SDOSERVICE_AB_UNSUPPORTED_ACCESS;
					break;
		 
				default: /* parameter error, most likely it does not exist */ 
					response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
					response->initiate.data32 = SDOSERVICE_AB_NOT_EXIST;
				break;
			}
		}
	}
}

void SDOSERVICE_upload_segment( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response )
{
	uint32_t loop;

	if( SDOSERVICE_var.dataptr )
	{
		if( request->cmd == SDOSERVICE_var.cmd )
		{
			response->cmd = SDOSERVICE_CMD_SCS_UPLOAD_SEGMENT | (SDOSERVICE_var.cmd & SDOSERVICE_CMD_T_MSK); /* response with same toggle-bit value */
			SDOSERVICE_var.cmd ^= SDOSERVICE_CMD_T_MSK; /* toggle expected toogle-bit for next segment */
			loop = 0;
			while( SDOSERVICE_var.bytecount && (loop < 7) )
			{
				--SDOSERVICE_var.bytecount;
				response->segment.data8[loop++] = *SDOSERVICE_var.dataptr++ ;
			}
			if( SDOSERVICE_var.bytecount == 0 )
			{
				response-> cmd |= (SDOSERVICE_CMD_C_NO_MORE_SEGMENTS<<SDOSERVICE_CMD_C_POS) | (((7-loop)<<SDOSERVICE_CMD_SEGMENTED_N_POS)&SDOSERVICE_CMD_SEGMENTED_N_MSK);
				
				while( loop < 7 )
				{ /* clear remaining data bytes */
					response->segment.data8[loop++] = 0x00; 
				}
				SDOSERVICE_var.dataptr = NULL;
			}
			else{/* do nothing, default is "more segments to process" */}				
		}
		else
		{ /* wrong toggle-bit value */
				response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
				response->initiate.data32 = SDOSERVICE_AB_TOGGLE_BIT;
		}
	}
	else
	{ /* segmented transfer was not initiated */
			response->cmd = SDOSERVICE_CMD_SCS_ABORT << SDOSERVICE_CMD_SCS_POS;
			response->initiate.data32 = SDOSERVICE_AB_CMD;
	}
}

void SDOSERVICE_download_block( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response )
{
}

void SDOSERVICE_upload_block( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response )
{
}

void SDOSERVICE_abort( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response )
{
}

void SDOSERVICE_error( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response )
{
}

void (*const SDOSERVICE_CMD_CCS[])( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response ) =
{
	SDOSERVICE_download_segment,
	SDOSERVICE_download_initiate,
	SDOSERVICE_upload_initiate,
	SDOSERVICE_upload_segment,
	SDOSERVICE_abort,
	SDOSERVICE_upload_block,
	SDOSERVICE_download_block,
	SDOSERVICE_error /* this one should never happen */
};

void SDOSERVICE_ProcessRequest( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response )
{
	SDOSERVICE_CMD_CCS[(request->cmd & SDOSERVICE_CMD_CCS_MSK) >> SDOSERVICE_CMD_CCS_POS ]( request, response );
}

