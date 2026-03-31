/**********************************************************************************************************************
* File:  parameterHandler_api.h
 * RqID:  
 * Brief: Parameter Handler prototypes for DTI 
 * $Id: 3276b023a2e06c3b9b20d37e4359da4ad18f5060 $
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
 
#ifndef __parameter_handler_api_h 
#define __parameter_handler_api_h 
 
/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include <stdint.h>

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/
typedef enum PARH_Errors {
	PARH_Errors_No_Error = 0,
	PARH_Errors_Idx_Unknown,
	PARH_Errors_Not_Allowed,
	PARH_Errors_Value_Out_Of_Range,
	PARH_Errors_Error
}	PARH_Errors;

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/
/*!
 * Read parameter
 * \param[in] index is the same as idx_target XML description
 * \param[out] a buffer where to put the result.
 * \param[in] num_bytes how many bytes needs to be transferred from the starting element.
 * @return error code e.g. parameter not found etc..
 *
 * Examples:
 * Read full data struct:
 * PARH_get(xml idx_target,size of data struct cputed from xml,ptr where to store the data)
 *
 * Read 2 elements of an array of uint32 elements
 * PARH_get(idx_target, 2*4, buffer )
 *
 * Read attribute in a data structure that is a an array with 10 elements of uint16
 * PARH_get(field idx_target, 2*10, ptr to result buffer)
 */
PARH_Errors PARH_get(uint32_t index, uint16_t num_bytes, void *result);

/*!
 * Write parameter
 * \param[in] index is the same as idx_target XML description
 * \param[in] input_buffer data to be written.
 * \param[in] num_bytes how many bytes are valid in the buffer
 * \return error code e.g. parameter not found, not writable etc...
 */
PARH_Errors PARH_set(uint32_t index, uint16_t num_bytes, void *value);

/*!
 * Parameter Address
 * \param[in] index is the same as idx_target XML description
 * \param[out] void pointer to start address of selected parameter
 * \return error code e.g. parameter not found, not writable etc...
 */
PARH_Errors PARH_paramAddress(uint32_t index, void** pointer);

/*!
 * Maximum number of bytes allowed to access for this parameter
 * \param[in] index is the same as idx_target XML description
 * \return uint32_t maximum number of bytes, 0 if not allowed
 */
uint32_t PARH_paramMaxNumberBytes(uint32_t index);

#endif /* __parameter_handler_api_h */
