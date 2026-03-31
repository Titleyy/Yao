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
* File:  TSdriver.h
* RqID:
* Brief: Time stamp driver for target interface
* $Id: 456cbcaf22a54f9ab1810c7d9113bd6e5c027670 $
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

#ifndef _targetinterface_TSdriver_h
#define _targetinterface_TSdriver_h

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/
#include <stdint.h>

extern uint32_t getRuntimeCounter();
extern uint32_t DTI_TSdriver_getTimeStamp(void);
extern uint32_t DTI_TSdriver_getTimeStampScale(void);
extern uint32_t DTI_TSdriver_getTargetTime(void);
extern uint32_t DTI_TSdriver_getTargetTimeScale(void);

#endif /* _targetinterface_TSdriver_h */
