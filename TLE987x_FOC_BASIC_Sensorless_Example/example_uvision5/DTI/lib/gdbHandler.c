/**********************************************************************************************************************
 * File:  traceHandler.c	
 * RqID:  
 * Brief: Tracve handler for target interface
 * $Id: 8179e5bdba18d831073240f850f06919f2c708c6 $
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
#include "platform.h"
#include "dti.h"
#include "targetInterface.h"
#include "gdbstub/gdbstub.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/					

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					

/**********************************************************************************************************************
* LOCAL ENUMS
**********************************************************************************************************************/

/***********************************************************************************************************************
 * LOCAL DATA
 **********************************************************************************************************************/
const char gdb_NACK[] = "-";
GDBdata_t GDBdata_mem;

/***********************************************************************************************************************
 * LOCAL ROUTINES
 **********************************************************************************************************************/

/******************************************************************************************/
void GDBHandler_Init(void)
{
	GDBdata_ptr = &GDBdata_mem;
		
	GDBdata_ptr->cpu_stop_signal = GDB_SIG_NULL;
	GDBdata_ptr->thread_hc=1;
	GDBdata_ptr->thread_hg=1;
	GDBdata_ptr->response_ack = true;
	GDBdata_ptr->gdb_debug_status = NOT_DEBUGGING;

	GDBdata_ptr->gdb_buffer_ACK = '+';
	GDBdata_ptr->gdb_buffer_start = '$';

	DTI_arbiter_setPortRX(DTI_PORT_GDBHander, &GDBdata_ptr->gdb_input_buffer, sizeof(GDBdata_ptr->gdb_input_buffer)); 
	DTI_arbiter_setPortTX(DTI_PORT_GDBHander, &GDBdata_ptr->gdb_buffer_ACK, sizeof(GDBdata_ptr->gdb_buffer)+4); 
}

uint32_t GDBHandler_ProcessStub(void)
{
	static uint8_t tx_message_length = 0;
	static uint8_t acknowledged = 1;
	int32_t rx_message_length, loop;

	if( !DTI_arbiter_isBusyPortTX( DTI_PORT_GDBHander ) )
	{
		if( handle_pending_event() )
		{
			acknowledged = 0; /* this needs to be acknowledged */
			if( isResponseAck() )
			{
				tx_message_length = GDBdata_ptr->gbd_packetlen + 1;
				DTI_arbiter_setPortTX(DTI_PORT_GDBHander, &GDBdata_ptr->gdb_buffer_ACK, tx_message_length  ); 
			}
			else
			{
				tx_message_length = GDBdata_ptr->gbd_packetlen;
				DTI_arbiter_setPortTX(DTI_PORT_GDBHander, &GDBdata_ptr->gdb_buffer_start, tx_message_length ); 
			}
			DTI_arbiter_setReadyPortTX( DTI_PORT_GDBHander); /* Ready to send */
		}
		else
		{
			rx_message_length = DTI_arbiter_getLengthPortRX(DTI_PORT_GDBHander);
			if( rx_message_length > 0 )	
			{
				GDBdata_ptr->gdb_input_buffer[rx_message_length ] = 0; /* add zero termination */
				if( GDBdata_ptr->gdb_input_buffer[0] != '+' )
				{
					if( GDBdata_ptr->gdb_input_buffer[0] != '-' )
					{
						if( GDBdata_ptr->gdb_input_buffer[0] != '$' )
						{ /* the command is not valid, hence request to send again */
							if( GDBdata_ptr->gdb_input_buffer[0] != 0 )
							{
								tx_message_length = 2;
								DTI_arbiter_setPortTX(DTI_PORT_GDBHander, gdb_NACK, tx_message_length ); 
							}
							else {/* ignore the 0 */}
						}
						else
						{
							if( acknowledged || isIn_no_ack_mode())
							{
								acknowledged = 0; /* this just received command is to be acknowledged */
								loop = 1;
								while(loop<rx_message_length)
								{ /* search for '#' */
									if(GDBdata_ptr->gdb_input_buffer[loop] == '#')
									{
										GDBdata_ptr->gdb_input_buffer[loop] = 0;
										//ARNO: TODO checksum handling here 
										// if checksum is notOK, send '-'
										// else ...
										GDBdata_ptr->response_ack = true;
										
										if( protocol_parse(GDBdata_ptr->gdb_input_buffer+1) > 0)
										{
											if( isResponseAck() )
											{
												tx_message_length = GDBdata_ptr->gbd_packetlen + 1;
												DTI_arbiter_setPortTX(DTI_PORT_GDBHander, &GDBdata_ptr->gdb_buffer_ACK, tx_message_length  ); 
											}
											else
											{
												tx_message_length = GDBdata_ptr->gbd_packetlen;
												DTI_arbiter_setPortTX(DTI_PORT_GDBHander, &GDBdata_ptr->gdb_buffer_start, tx_message_length ); 
											}
										}
										else
										{
											acknowledged = 1; /* nothing to acknowledge */
											tx_message_length = 0;
										}
										break;
									}
									++loop;
								}
							}
							else
							{ /* the previous command was not acknowledged, hence request to send again */
								tx_message_length = 2;
								DTI_arbiter_setPortTX(DTI_PORT_GDBHander, gdb_NACK, tx_message_length ); 
							}
						}
					}
					DTI_arbiter_setSizePortTX( DTI_PORT_GDBHander, tx_message_length ); /* Size and Ready to send */
				}
				else
				{ /* '+' received */
					/* this is the acknowledge */
					acknowledged = 1;
					tx_message_length = 0;
				}
				DTI_arbiter_setReadyPortRX(DTI_PORT_GDBHander);
			}
		}
	}
	else {/* wait for the transmission being finalized */}
	
	return tx_message_length;
}
