/**********************************************************************************************************************
 * File:  arbiter.h	
 * RqID:  
 * Brief: Port arbiter for target interface
 * $Id: 7df5731bf67991d160b1bc263b1da75a703c154b $
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
 
#ifndef _DTI_arbiter_h
#define _DTI_arbiter_h

#ifdef __cplusplus 
extern "C" {
#endif

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include <stdint.h>
#include <stdbool.h>
#include "platform.h"
#include "dti_conf.h"

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/
typedef enum
{
	DTI_port0,
	DTI_port1,
	DTI_port2,
	DTI_port3,
	DTI_portLAST = DTI_port3,
	DTI_portMAX
} DTI_ports_e;

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
typedef struct DTI_port
{
	struct 
	{
		volatile uint16_t size:14;
		volatile uint16_t busy:1;
		volatile uint16_t ready:1;
	};
	volatile uint16_t length;
	void *buffer;
	void (*callback)(struct DTI_port*);
} DTI_port_t; 

typedef uint8_t DTI_arbiter_data_t;

typedef struct {
	struct {
		uint8_t APNR : 2;
		uint8_t AO : 1;
		uint8_t MO : 1;
	};
	DTI_arbiter_data_t ADATA;
} DTI_arbiter_t;

typedef struct {
	DTI_arbiter_t *ptr_DD;
	DTI_arbiter_t *ptr_DU;
	DTI_port_t *port_tx;
	DTI_port_t *port_rx;
	DTI_arbiter_data_t *data_tx;
	DTI_arbiter_data_t *data_rx;
	DTI_port_t P_TX[DTI_portMAX];
	DTI_port_t P_RX[DTI_portMAX];
} DTI_arbiter_cnf_t;

/***********************************************************************************************************************
* EXTERN DECLARATIONS
***********************************************************************************************************************/
extern DTI_arbiter_cnf_t DTI_arbiter_DU;
extern DTI_arbiter_cnf_t DTI_arbiter_DD;

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/
void DTI_arbiter_init(void);
void DTI_Arbitration_TX( DTI_arbiter_t *tx );
bool DTI_arbiter_Serialize_TX( DTI_arbiter_t *tx, uint32_t size );
void DTI_arbiter_Deserialize_RX( DTI_arbiter_t *rx, uint32_t size);

void DTI_arbiter_setPortTX(DTI_ports_e port, const void* buffer, uint16_t size ); /* Buffer and size */
void DTI_arbiter_setPortRX(DTI_ports_e port, void* buffer, uint16_t size ); /* Buffer, size and Ready to receive */
void DTI_arbiter_setCallbackPortTX(DTI_ports_e port, void (*callback)(DTI_port_t*) );
void DTI_arbiter_setCallbackPortRX(DTI_ports_e port, void (*callback)(DTI_port_t*) );
bool DTI_arbiter_setReadyPortTX( DTI_ports_e port ); /* Ready to send */
bool DTI_arbiter_setSizePortTX( DTI_ports_e port, uint16_t size ); /* Size and Ready to send */
bool DTI_arbiter_setReadyPortRX( DTI_ports_e port ); /* Ready to receive */
bool DTI_arbiter_isBusyPortTX( DTI_ports_e port );
bool DTI_arbiter_isBusyPortRX( DTI_ports_e port );
int32_t DTI_arbiter_getLengthPortRX( DTI_ports_e port );

bool DTI_arbiter_isOngoingRX(void);
bool DTI_arbiter_isOngoingTX(void);

#ifdef __cplusplus 
}
#endif

#endif /*_DTI_arbiter_h*/
