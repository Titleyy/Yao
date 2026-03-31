/**********************************************************************************************************************
 * File:  sdoService.h	
 * RqID:  
 * Brief: SDO Service for target interface
 * $Id: 88a469db0fbcfbfba20dbe08a35c4477e46cb03e $
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
 
#ifndef _debug_sdoservice_h
#define _debug_sdoservice_h

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include <stdint.h>
#include <stdbool.h>
#include "platform.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
/* Client command specifier (request) */
#define SDOSERVICE_CMD_CCS_DOWNLOAD_INITIATE          1U
#define SDOSERVICE_CMD_CCS_DOWNLOAD_SEGMENT           0U
#define SDOSERVICE_CMD_CCS_UPLOAD_INITIATE            2U
#define SDOSERVICE_CMD_CCS_UPLOAD_SEGMENT             3U
#define SDOSERVICE_CMD_CCS_DOWNLOAD_BLOCK             6U
#define SDOSERVICE_CMD_CCS_UPLOAD_BLOCK               5U
#define SDOSERVICE_CMD_CCS_ABORT                      4U

#define SDOSERVICE_CMD_CCS_MSK                        0x00E0U
#define SDOSERVICE_CMD_CCS_POS                        5U


/* Server command specifier (response) */
#define SDOSERVICE_CMD_SCS_UPLOAD_INITIATED           2U
#define SDOSERVICE_CMD_SCS_UPLOAD_SEGMENT             0U
#define SDOSERVICE_CMD_SCS_DOWNLOAD_INITIATED         3U
#define SDOSERVICE_CMD_SCS_DOWNLOAD_SEGMENT           1U
#define SDOSERVICE_CMD_SCS_DOWNLOAD_BLOCK             5U
#define SDOSERVICE_CMD_SCS_UPLOAD_BLOCK               6U
#define SDOSERVICE_CMD_SCS_ABORT                      4U

#define SDOSERVICE_CMD_SCS_MSK                        0x00E0U
#define SDOSERVICE_CMD_SCS_POS                        5U


/* Number of Bytes in initiate Command*/
#define SDOSERVICE_CMD_N_1Byte                        3U
#define SDOSERVICE_CMD_N_2Byte                        2U
#define SDOSERVICE_CMD_N_3Byte                        1U
#define SDOSERVICE_CMD_N_4Byte                        0U

#define SDOSERVICE_CMD_INITIATE_N_MSK                 0x000CU
#define SDOSERVICE_CMD_INITIATE_N_POS                 2U

/* Number of Bytes in segmented command*/
#define SDOSERVICE_CMD_SEGMENTED_N_MSK                0x000EU
#define SDOSERVICE_CMD_SEGMENTED_N_POS                1U


/* Transfer Type */
#define SDOSERVICE_CMD_E_NORMAL                       0U
#define SDOSERVICE_CMD_E_EXPEDITED                    1U

#define SDOSERVICE_CMD_E_MSK                          0x0002U
#define SDOSERVICE_CMD_E_POS                          1U


/* Size Indicator */
#define SDOSERVICE_CMD_S_NOSIZE                       0U
#define SDOSERVICE_CMD_S_SIZE                         1U

#define SDOSERVICE_CMD_S_MSK                          0x0001U
#define SDOSERVICE_CMD_S_POS                          0U

/* Toggle Bit */
#define SDOSERVICE_CMD_T_MSK                          0x0010U
#define SDOSERVICE_CMD_T_POS                          4U

/* More Segments */
#define SDOSERVICE_CMD_C_MORE_SEGMENTS                0U
#define SDOSERVICE_CMD_C_NO_MORE_SEGMENTS             1U

#define SDOSERVICE_CMD_C_MSK                          0x0001U
#define SDOSERVICE_CMD_C_POS                          0U

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/
/* SDO abort codes */
typedef enum{
    SDOSERVICE_AB_NONE                  = 0x00000000UL, /**< 0x00000000, No abort */
    SDOSERVICE_AB_TOGGLE_BIT            = 0x05030000UL, /**< 0x05030000, Toggle bit not altered */
    SDOSERVICE_AB_TIMEOUT               = 0x05040000UL, /**< 0x05040000, SDO protocol timed out */
    SDOSERVICE_AB_CMD                   = 0x05040001UL, /**< 0x05040001, Command specifier not valid or unknown */
    SDOSERVICE_AB_BLOCK_SIZE            = 0x05040002UL, /**< 0x05040002, Invalid block size in block mode */
    SDOSERVICE_AB_SEQ_NUM               = 0x05040003UL, /**< 0x05040003, Invalid sequence number in block mode */
    SDOSERVICE_AB_CRC                   = 0x05040004UL, /**< 0x05040004, CRC error (block mode only) */
    SDOSERVICE_AB_OUT_OF_MEM            = 0x05040005UL, /**< 0x05040005, Out of memory */
    SDOSERVICE_AB_UNSUPPORTED_ACCESS    = 0x06010000UL, /**< 0x06010000, Unsupported access to an object */
    SDOSERVICE_AB_WRITEONLY             = 0x06010001UL, /**< 0x06010001, Attempt to read a write only object */
    SDOSERVICE_AB_READONLY              = 0x06010002UL, /**< 0x06010002, Attempt to write a read only object */
    SDOSERVICE_AB_NOT_EXIST             = 0x06020000UL, /**< 0x06020000, Object does not exist */
    SDOSERVICE_AB_NO_MAP                = 0x06040041UL, /**< 0x06040041, Object cannot be mapped to the PDO */
    SDOSERVICE_AB_MAP_LEN               = 0x06040042UL, /**< 0x06040042, Number and length of object to be mapped exceeds PDO length */
    SDOSERVICE_AB_PRAM_INCOMPAT         = 0x06040043UL, /**< 0x06040043, General parameter incompatibility reasons */
    SDOSERVICE_AB_DEVICE_INCOMPAT       = 0x06040047UL, /**< 0x06040047, General internal incompatibility in device */
    SDOSERVICE_AB_HW                    = 0x06060000UL, /**< 0x06060000, Access failed due to hardware error */
    SDOSERVICE_AB_TYPE_MISMATCH         = 0x06070010UL, /**< 0x06070010, Data type does not match, length of service parameter does not match */
    SDOSERVICE_AB_DATA_LONG             = 0x06070012UL, /**< 0x06070012, Data type does not match, length of service parameter too high */
    SDOSERVICE_AB_DATA_SHORT            = 0x06070013UL, /**< 0x06070013, Data type does not match, length of service parameter too short */
    SDOSERVICE_AB_SUB_UNKNOWN           = 0x06090011UL, /**< 0x06090011, Sub index does not exist */
    SDOSERVICE_AB_INVALID_VALUE         = 0x06090030UL, /**< 0x06090030, Invalid value for parameter (download only). */
    SDOSERVICE_AB_VALUE_HIGH            = 0x06090031UL, /**< 0x06090031, Value range of parameter written too high */
    SDOSERVICE_AB_VALUE_LOW             = 0x06090032UL, /**< 0x06090032, Value range of parameter written too low */
    SDOSERVICE_AB_MAX_LESS_MIN          = 0x06090036UL, /**< 0x06090036, Maximum value is less than minimum value. */
    SDOSERVICE_AB_NO_RESOURCE           = 0x060A0023UL, /**< 0x060A0023, Resource not available: SDO connection */
    SDOSERVICE_AB_GENERAL               = 0x08000000UL, /**< 0x08000000, General error */
    SDOSERVICE_AB_DATA_TRANSF           = 0x08000020UL, /**< 0x08000020, Data cannot be transferred or stored to application */
    SDOSERVICE_AB_DATA_LOC_CTRL         = 0x08000021UL, /**< 0x08000021, Data cannot be transferred or stored to application because of local control */
    SDOSERVICE_AB_DATA_DEV_STATE        = 0x08000022UL, /**< 0x08000022, Data cannot be transferred or stored to application because of present device state */
    SDOSERVICE_AB_DATA_OD               = 0x08000023UL, /**< 0x08000023, Object dictionary not present or dynamic generation fails */
    SDOSERVICE_AB_NO_DATA               = 0x08000024UL  /**< 0x08000024, No data available */
}SDOSERVICE_abortCode_t;

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
typedef union {
	struct {
		uint8_t subindex;
		uint8_t index_lo;
		uint8_t index_hi;
		uint8_t reserved;
	};
	uint32_t idx_target;
} SDOSERVICE_index_t;

typedef struct {
	uint8_t cmd;
	uint8_t index_lo;
	uint8_t index_hi;
	uint8_t subindex;
	union {
		uint8_t data8[4];
		uint16_t data16[2];
		uint32_t data32;
	};
} SDOSERVICE_initiate_t;

typedef struct {
	uint8_t cmd;
	uint8_t data8[7];
} SDOSERVICE_segment_t;

typedef union{
	struct {
		uint8_t cmd;
		uint8_t reserved[7];
	};	
	SDOSERVICE_initiate_t initiate;
	SDOSERVICE_segment_t segment;
	struct {
		uint32_t head;
		uint32_t tail;
	};
//	uint64_t buffer;
} SDOSERVICE_buffer_t;

typedef struct {
	union {
		struct {
			uint8_t cmd;
			uint8_t index_lo;
			uint8_t index_hi;
			uint8_t subindex;
		};
		uint32_t head; /* required for segmented upload and download with togglebit */
	};
	uint32_t bytecount;
	uint8_t* dataptr;
} SDOSERVICE_vt;

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/
void SDOSERVICE_ProcessRequest( SDOSERVICE_buffer_t *request, SDOSERVICE_buffer_t *response );

#endif /* _debug_sdoservice_h */
