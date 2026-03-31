/**********************************************************************************************************************
 * File:  targetInterface.h	
 * RqID:  
 * Brief: target interface header file provides APIs to services
 * $Id: 7384ee8d414ed316412b6f8fafd763e2001bfab1 $
 * No safety related function
 **********************************************************************************************************************/
/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-25:
 *     - Initial version
 *
 */
 
#ifndef _dti_dti_h
#define _dti_dti_h

 /***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/																				
#include "platform.h"
#include "dti_conf.h"
#include "arbiter.h"
#include "router.h"
#include "linklayer.h"

/**********************************************************************************************************************
 * MACROS
 **********************************************************************************************************************/
#define DTI_VERSION(major, minor, patch) ((uint16_t)(((major<<12) & 0xF000UL) + ((minor<<6) & 0x0FC0UL) + (patch & 0x003F)))

/**********************************************************************************************************************
 * DATA STRUCTURES
 **********************************************************************************************************************/					
typedef struct {
	uint32_t ErrorCount;
	uint32_t FrameCount;
	uint8_t  ClockDivider;
}DTI_vt;

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/
void DTI_setMD_DU(DTI_router_MD_e md);
void DTI_sendSyncFrame_DU(void);
void DTI_setChannelPtr_TX(DTI_channels_e channel, DTI_stream_channel_t *ptr, TRCH_SignalType_t type);
void DTI_setNumberChannels_DU(uint32_t numCh, uint32_t resolution);
void DTI_setClockDivider_DU(uint32_t clkdiv);
uint8_t DTI_getClockDivider_DU(void);

#endif /*_dti_dti_h*/
