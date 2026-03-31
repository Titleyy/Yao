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
* File:  dti_conf.h
*
* Brief: Configuration for DTI target interface
**********************************************************************************************************************/

#ifndef _DTI_interface_conf_h
#define _DTI_interface_conf_h

/***********************************************************************************************************************
 * HEADER FILES
 **********************************************************************************************************************/
#include <stdint.h>

/**********************************************************************************************************************
* MACROS
**********************************************************************************************************************/
#define DTI_TARGET
#define DTI_interface_BAUDRATE               (115200)

#define TRCH_SERVICE_EVENT_CHANNEL_MAX       (1)
#define TRCH_SERVICE_EVENT_BUFFER_MAX        (1)

#define TRCH_SERVICE_PARAMETER_CHANNEL_MAX   (1)
#define TRCH_SERVICE_PARAMETER_QUEUE_MAX     (1)

#define TRCH_SERVICE_SAMPLING_DATABUFFER_MAX (1)
#define TRCH_SERVICE_SAMPLING_CHANNEL_MAX    (0)

#define DTI_STREAM_CHANNEL_MAX               (8)

#define DTI_PORT_TraceHander                 (DTI_port0)
#define DTI_PORT_MessageHander               (DTI_port1)
#define DTI_PORT_GDBHander                   (DTI_port2)
#define DTI_PORT_Binary                      (DTI_port3)

#define TRCH_SERVICE_STREAMING_available     (false)
#define TRCH_SERVICE_EVENT_available         (false)
#define TRCH_SERVICE_SAMPLING_available      (false)
#define TRCH_SERVICE_PARAMETER_available     (false)
#define TRCH_SERVICE_PROGRAMMING_available   (false)

#endif /*_DTI_interface_conf_h*/
