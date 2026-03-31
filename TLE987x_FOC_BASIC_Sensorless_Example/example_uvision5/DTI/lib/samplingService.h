/**********************************************************************************************************************
 * File:  samplingService.h
 * RqID:  
 * Brief: Sampling Service as part of Trace Handler for target interface
 * $Id: 1dc4fab74fcd9c7e91eb8beec60b9a6bc4bfa60f $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-15:
 *     - Initial version
 *
 */
  
#ifndef __samplingService_h																					
#define __samplingService_h

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include "traceHandler_api.h"
#include "traceHandler.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#define TRCH_SERVICE_SAMPLING_SUBBLOCKSIZE (16)
#define TRCH_SERVICE_SAMPLING_DATABLOCK_MAX (TRCH_SERVICE_SAMPLING_DATABUFFER_MAX / TRCH_SERVICE_SAMPLING_SUBBLOCKSIZE )
#define TRCH_SERVICE_SAMPLING_TRIGGERPOS_100 (0x100UL)

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					

/***********************************************************************************************************************
* EXTERN DECLARATIONS
***********************************************************************************************************************/

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/
TRCH_ERRORCODES_e TRCH_SamplingService(TRCH_buffer_t*,TRCH_buffer_t*);
TRCH_ERRORCODES_e TRCH_SamplingService_init(TRCH_buffer_t*,TRCH_buffer_t*);
TRCH_ERRORCODES_e TRCH_SamplingService_deinit(TRCH_buffer_t*,TRCH_buffer_t*);

void TRCH_SamplingService_ProcessSampling(void);
uint8_t TRCH_SamplingService_getNumBlocks(void);
void TRCH_SamplingService_releaseBuffer(void);

void TRCH_SamplingService_ReleaseSampling(void);
uint32_t TRCH_SamplingService_isReady(void);
TRCH_ERRORCODES_e TRCH_SamplingService_getBufferEntry( TRCH_buffer_t* response, uint8_t subblock);

/***********************************************************************************************************************
 * static inline Functions
 **********************************************************************************************************************/



#endif /* __samplingService_h */
