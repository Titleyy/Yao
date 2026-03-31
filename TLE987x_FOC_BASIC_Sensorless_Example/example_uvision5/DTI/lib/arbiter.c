/**********************************************************************************************************************
 * File:  arbiter.c	
 * RqID:  
 * Brief: Port arbiter for target interface
 * $Id: 6a15ae1cfe56ce17627bbd279e75145990cf341c $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-25:
 *     - Changed behavior of Ready-Flag: Ready is cleard after all buffer data has been sent to driver
 * 2020-April-30:
 *     - Initial version
 *
 */
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include <stddef.h>
#include "arbiter.h"
#include "router.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#if ( DTI_portMAX > 4)
#error (Maximum number of arbiter ports is 4: DTI_portMAX too large ) 
#endif

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
DTI_arbiter_cnf_t DTI_arbiter;

extern DTI_arbiter_cnf_t DTI_arbiter_DU;
extern DTI_arbiter_cnf_t DTI_arbiter_DD;

/***********************************************************************************************************************
 * FUNCTIONS
 **********************************************************************************************************************/	
void DTI_arbiter_init(void)
{
	DTI_arbiter.ptr_DD = DTI_router_arbiterptr_DD();
	DTI_arbiter.ptr_DU = DTI_router_arbiterptr_DU();
}

void DTI_arbiter_SerializeShort_TX( DTI_arbiter_t *tx )
{
	DTI_port_t *port = DTI_arbiter.port_tx;
	static uint32_t AO = 0;
	
	tx->AO = AO;
	
	if( port )
	{
		if( !(port->length) )
		{
			port->busy = 0;
			AO = 0;
			tx->AO = AO;
			DTI_arbiter.port_tx = NULL; /* end of transmission */
			port->ready = 0;
		}
		else
		{
			--port->length;
			tx->ADATA = *DTI_arbiter.data_tx++;
			AO = 1;
		}
	}
	else { /* do nothing */ }
}

bool DTI_arbiter_SerializeLong_TX(DTI_arbiterLong_t *tx, uint32_t size)
{
	DTI_port_t *port = DTI_arbiter.port_tx;
	static uint32_t AO = 0;
	uint32_t loop;

	tx->AO = AO;

	if (port)
	{
		if (!(port->length))
		{
			port->busy = 0;
			AO = 0;
			tx->AO = AO;
			DTI_arbiter.port_tx = NULL; /* end of transmission */
			port->ready = 0;
		}
		else
		{
			if(!AO)
			{
				tx->ADATA[0] = (DTI_arbiter_data_t) port->length;
				AO = 1;
				loop = 1;
			}
			else
			{
				loop = 0;
				
			}
			
			for (/* loop is set above */; loop < size; ++loop)
			{
				if (port->length)
				{
					--port->length;
					tx->ADATA[loop] = *DTI_arbiter.data_tx++;
				}
				else
				{
					tx->ADATA[loop] = 0xff;
				}
			}

			if (!port->length) /* don't use "else" as port->length could have been set to 0 above */
			{
				port->busy = 0;
				AO = 0;
				DTI_arbiter.port_tx = NULL; /* end of transmission */
				port->ready = 0; 
			}
		}
	}
	else { /* do nothing */ }

	return port;
}

bool DTI_arbiter_Serialize_TX(DTI_arbiter_t *tx, uint32_t size)
{
	if( tx->MO )
	{
		DTI_arbiter_SerializeShort_TX(tx);
	}
	else
	{
		return DTI_arbiter_SerializeLong_TX((DTI_arbiterLong_t*) tx, size);
	}
	return true;
}

void DTI_Arbitration_TX( DTI_arbiter_t *tx )
{
	static uint8_t portnumber;
	static DTI_port_t* port;
	
	/* Check if there is data transmission ongoing */
	if( !(DTI_arbiter.port_tx) )
	{
		/* Set port pointer */
		port = &DTI_arbiter.P_TX[portnumber];

		/* Check if this port is ready to send data */
		if( port->ready )
		{
			port->length = port->size;
			port->busy = 1;
			tx->APNR = portnumber;
			DTI_arbiter.port_tx = port;
			DTI_arbiter.data_tx = port->buffer;
		}
		else
		{/* there is no data to send by this port */
			if( port->callback ) /* call the callback function if defined */
			{
				port->callback( port );
			}
			else {/* do nothing */ }
		}
			
		/* Ports are checked round robin */
		if( portnumber == DTI_portLAST )
		{
			portnumber = DTI_port0;
		}
		else 
		{
			++portnumber;
		}
	}
	else
	{/* processing is ongoing */}
}

void DTI_arbiter_DeserializeShort_RX( DTI_arbiter_t *rx )
{
	DTI_port_t* port;
	
	if( !(rx->AO) )
	{ /* begin of rx data or idle */
		port = &DTI_arbiter.P_RX[rx->APNR];
		if( port && port->ready )
		{
			port->length = 1UL; 
			DTI_arbiter.port_rx = port;
			DTI_arbiter.data_rx = port->buffer;
		}
		else
		{ 
			if(DTI_arbiter.port_rx)
			{
				DTI_arbiter.port_rx->busy = 0; /* in case of idle, clear busy flag */
				DTI_arbiter.port_rx = NULL;
				DTI_arbiter.data_rx = NULL;
				if( !port->ready && port->callback ) /* call the callback function if defined and port not ready */
				{ /* port not ready means, the previously received data has not been processed yet */
					port->callback( port );
				}
			}
		}
	}
	else
	{ /* ongoing transmission */
		port = DTI_arbiter.port_rx;
		if( port )
		{ 
			++port->length;
			port->busy = 1UL;
			port->ready = 0UL;
		}
		else
		{/* the port was not ready, this packet is lost */}
	}
	
	if( port && DTI_arbiter.data_rx)
	{
		if( port->length <= port->size )
		{
			*DTI_arbiter.data_rx++ = rx->ADATA;
		}
	}
	else { /* do nothing */ }
}

void DTI_arbiter_DeserializeLong_RX(DTI_arbiterLong_t *rx, uint32_t size )
{
	DTI_port_t* port;
	static uint32_t datalength;

	if (!(rx->AO))
	{ /* begin of rx data */
		port = &DTI_arbiter.P_RX[rx->APNR];
		if (port && port->ready)
		{
			datalength = rx->ADATA[0];
			port->length = 1;

			if (datalength <= port->size)
			{ /* OK, the data will fit into the buffer */
				DTI_arbiter.port_rx = port;
				DTI_arbiter.data_rx = port->buffer;
				port->busy = 1UL;
				port->ready = 0UL;
				for (uint32_t i = 1; i < size; ++i)
				{
					*DTI_arbiter.data_rx++ = rx->ADATA[i];
					if (port->length == datalength)
					{ /* all data received */
						DTI_arbiter.port_rx->busy = 0;
						DTI_arbiter.port_rx = NULL;
						DTI_arbiter.data_rx = NULL;
						if( port->callback ) /* call the callback function if defined */
						{
							port->callback( port );
						}
						break;
					}
					++port->length;
				}
			}
			else 
			{ /* ERROR, the data will exceed the buffersize */
				/* ignore this data packet */
				if (DTI_arbiter.port_rx)
				{
					DTI_arbiter.port_rx->busy = 0;
					DTI_arbiter.port_rx = NULL;
					DTI_arbiter.data_rx = NULL;
				}
			}
		}
		else
		{
			if (DTI_arbiter.port_rx)
			{
				DTI_arbiter.port_rx->busy = 0; /* in case of buffer not ready or no buffer available: clear busy flag */
				DTI_arbiter.port_rx = NULL;
				DTI_arbiter.data_rx = NULL;
			}
		}
	}
	else
	{ /* ongoing transmission */
		port = DTI_arbiter.port_rx;
		if (port)
		{
			for (uint32_t i = 0; i < size; ++i)
			{
				*DTI_arbiter.data_rx++ = rx->ADATA[i];
				if (port->length == datalength)
				{ /* all data received */
					DTI_arbiter.port_rx->busy = 0; /* done: clear busy flag */
					DTI_arbiter.port_rx = NULL;
					DTI_arbiter.data_rx = NULL;
					if( port->callback ) /* call the callback function if defined */
					{
						port->callback( port );
					}
					break;
				}
				++port->length;
			}
		}
		else
		{
			/* the port was not ready, this packet is lost */
		}
	}
}

void DTI_arbiter_Deserialize_RX(DTI_arbiter_t *rx, uint32_t size)
{
	if (rx->MO)
	{
		DTI_arbiter_DeserializeShort_RX(rx);
	}
	else
	{
		DTI_arbiter_DeserializeLong_RX((DTI_arbiterLong_t*)rx, size);
	}
}


void DTI_arbiter_setPortTX(DTI_ports_e port, const void* buffer, uint16_t size )
{
	DTI_arbiter.P_TX[port].size = size;
	DTI_arbiter.P_TX[port].callback = NULL;
	DTI_arbiter.P_TX[port].buffer = (void*) buffer;
}

void DTI_arbiter_setCallbackPortTX(DTI_ports_e port, void (*callback)(DTI_port_t*) )
{
	DTI_arbiter.P_TX[port].callback = callback;
}

void DTI_arbiter_setPortRX(DTI_ports_e port, void* buffer, uint16_t size )
{
	DTI_arbiter.P_RX[port].ready = true;
	DTI_arbiter.P_RX[port].size = size;
	DTI_arbiter.P_RX[port].callback = NULL;
	DTI_arbiter.P_RX[port].buffer = buffer;
}

void DTI_arbiter_setCallbackPortRX(DTI_ports_e port, void (*callback)(DTI_port_t*))
{
	DTI_arbiter.P_RX[port].callback = callback;
}

bool DTI_arbiter_setSizePortTX( DTI_ports_e port, uint16_t size )
{
	int32_t  retval = false;
	if( DTI_arbiter.P_TX[port].buffer )
	{
		if( size && !DTI_arbiter.P_TX[port].busy )
		{
			DTI_arbiter.P_TX[port].size = size;
			DTI_arbiter.P_TX[port].busy = true;
			DTI_arbiter.P_TX[port].ready = true;
			retval = true;
		}
		else
		{
			retval = false;
		}
	}
	else
	{ /* port is not configured */
		retval = false;
	}
	return retval;
}

bool DTI_arbiter_setReadyPortTX( DTI_ports_e port )
{
	int32_t  retval = false;
	if( DTI_arbiter.P_TX[port].buffer )
	{
		if( !DTI_arbiter.P_TX[port].busy )
		{
			DTI_arbiter.P_TX[port].busy = true;
			DTI_arbiter.P_TX[port].ready = true;
			retval = true;
		}
		else
		{ /* port is busy */
			retval = false;
		}
	}
	else
	{ /* port is not configured */
		retval = false;
	}
	return retval;
}

bool DTI_arbiter_setReadyPortRX( DTI_ports_e port )
{
	int32_t  retval = false;
	if( DTI_arbiter.P_RX[port].buffer )
	{
		if( !DTI_arbiter.P_RX[port].busy )
		{
			DTI_arbiter.P_RX[port].ready = true;
			DTI_arbiter.port_rx = 0; /* interrupt a just started reception */
			/* anything else to clear??? */
			retval = true;
		}
		else
		{ /* Port is busy */
			retval = false;
		}
	}
	else
	{ /* port is not configured */
		retval = false;
	}
	return retval;
}

/* busy means: not ready; and ready means "set ready" by user */
bool DTI_arbiter_isBusyPortTX( DTI_ports_e port )
{
	return DTI_arbiter.P_TX[port].ready;
}


/* busy means: not ready */
bool DTI_arbiter_isBusyPortRX( DTI_ports_e port )
{
	bool retval;
	
	if( !DTI_arbiter.P_RX[port].busy )
	{
		retval = (DTI_arbiter.P_RX[port].length == 0);
	}
	else
	{
		retval = true;
	}
	return retval;
}

int32_t DTI_arbiter_getLengthPortRX( DTI_ports_e port )
{
	int32_t  retval = -1;
	
	if( DTI_arbiter.P_RX[port].buffer )
	{
		if (!DTI_arbiter.P_RX[port].ready)
		{
			if (!DTI_arbiter.P_RX[port].busy)
			{
				retval = DTI_arbiter.P_RX[port].length;
			}
			else
			{ /* receiving data is ongoing */
				retval = -3;
			}
		}
		else
		{ /* no data received yet */
			retval = -2;
		}
	}
	else
	{ /* port is not configured */
		retval = -1;
	}
	return retval;
}

bool DTI_arbiter_isOngoingRX(void)
{
	return ((volatile DTI_port_t *) DTI_arbiter.port_rx) != NULL;
}

bool DTI_arbiter_isOngoingTX(void)
{
	return ((volatile DTI_port_t *) DTI_arbiter.port_tx) != NULL;
}
