/**********************************************************************************************************************
 * File:  signalHandler_api.h	
 * RqID:  
 * Brief: Signal Handler prototypes for DTI trace handler
 * $Id: d10203c2c1ad8e0e3bad3ad1df20b1232b40b954 $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-12:
 *     - Initial version
 *
 */
 
#ifndef __signal_handler_api_h 
#define __signal_handler_api_h 
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include <stdint.h>

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/
typedef enum {
	TRCH_SignalType_8bit,
	TRCH_SignalType_16bit,
	TRCH_SignalType_16bitH,
	TRCH_SignalType_32bit,

	TRCH_SignalType_Input8bit,
	TRCH_SignalType_Input16bit,
	TRCH_SignalType_Input16bitH,
	TRCH_SignalType_Input32bit,
	
	TRCH_SignalType_API8bit,
	TRCH_SignalType_API16bit,
	TRCH_SignalType_API16bitH,
	TRCH_SignalType_API32bit,
	
	TRCH_SignalType_none
} TRCH_SignalType_t;

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/
const void* TRCH_getSignalptr( uint32_t idx_target );
TRCH_SignalType_t TRCH_getSignalType( uint32_t trch_signal  );

typedef uint32_t (*TRCH_getSignalApi_ptr_t)(void);
extern const TRCH_getSignalApi_ptr_t TRCH_getSignalFunctions[]; /* array of function pointers to api functions */

uint32_t TRCH_SamplingService_isSampling(void);

#endif /* __signal_handler_api_h */
