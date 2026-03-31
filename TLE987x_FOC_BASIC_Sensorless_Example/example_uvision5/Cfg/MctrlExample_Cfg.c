/*
 * Copyright (c) 2024 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG, its affiliates or its licensees. If and as long as no
 * such terms of use are agreed, use of this file is subject to the Evaluation Software License Agreement distributed
 * along with this file within the software delivery package.
 *
 */

/**
 * \file MctrlExample_Cfg.c
 * \brief Component configuration generated from:
 *          python file: spec/MctrlExample.py
 *          class:       MctrlExample
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "MctrlExample.h"
#include "stdbool.h"

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
MctrlExample_Param g_mctrlExampleParam =
    {
        .enableControl            = false, /**< enableControl_DU */
        .enablePowerStage         = false, /**< enablePowerStage_DU */
        .rotorAlignmentTime_ticks = 666,   /**< rotorAlignmentTime_DU */
        .controlMode              = 1,     /**< controlMode_DU */
        .referenceSpeed_rpm       = 0,     /**< referenceSpeed_DU */
        .clrFaultFoc              = false, /**< clrFaultFoc_DU */
    };
/* *INDENT-ON* */
