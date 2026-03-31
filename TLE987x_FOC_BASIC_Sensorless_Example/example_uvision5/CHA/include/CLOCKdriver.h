/*
 * Copyright (c) 2024 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG, its affiliates or its licensees. If and as long as no
 * such terms of use are agreed, use of this file is subject to the Evaluation Software License Agreement distributed
 * along with this file within the software delivery package.
 *
 */

/**********************************************************************************************************************
* File:  Clockdriver.h
* RqID:
* Brief: Clock driver for target interface
* No safety related function
**********************************************************************************************************************/

/**********************************************************************************************************************
 * Change History
 * --------------
 *
 * 2020-May-20:
 *     - Initial version
 *
 */

#ifndef _dti_CLOCKdriver_h
#define _dti_CLOCKdriver_h

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include "traceHandler_api.h"

/**********************************************************************************************************************
* MACROS
**********************************************************************************************************************/
#define TRCH_SERVICE_CLOCKSOURCE_MAX (1)

/**********************************************************************************************************************
* ENUMS
**********************************************************************************************************************/

/**********************************************************************************************************************
* DATA STRUCTURES
**********************************************************************************************************************/

/***********************************************************************************************************************
* EXTERN DECLARATIONS
***********************************************************************************************************************/
extern TRCH_ClockCallback_t TRCH_SamplingService_ClockCallback[TRCH_SERVICE_CLOCKSOURCE_MAX];
extern TRCH_ClockCallback_t TRCH_StreamingService_ClockCallback[TRCH_SERVICE_CLOCKSOURCE_MAX];

/***********************************************************************************************************************
 * API Prototypes
 **********************************************************************************************************************/

/***********************************************************************************************************************
 * static inline Functions
 **********************************************************************************************************************/

/* Scaling: Time between samples [s] = clkdiv * ClockScale / (64 kHz * TimerScale)
 *          ClockScale is the period value of the timer
 *          TimerScale is the clock of the timer / 64kHz
 */
static inline uint16_t TRCH_SamplingService_getClockScale(uint32_t source)
{
    return 0xCCCCU;
}


static inline uint16_t TRCH_SamplingService_getTimerScale(uint32_t source)
{
    return 0xDDDDU;
}


#endif /* _dti_CLOCKdriver_h */
