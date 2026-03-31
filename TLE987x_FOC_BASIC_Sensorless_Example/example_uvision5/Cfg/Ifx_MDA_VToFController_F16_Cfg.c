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
 * \file Ifx_MDA_VToFController_F16_Cfg.c
 * \brief Component configuration generated from:
 *          python file: spec/Ifx_MDA_VToFController_F16.py
 *          class:       Ifx_MDA_VToFController_F16
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_MDA_VToFController_F16.h"

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
Ifx_MDA_VToFController_F16_Param g_vToFControllerParam =
    {
        .angleIncrementQ14          = 655,   /**< angleIncrement_DU */
        .VToFLUTnorm.ratedSpeedQ15  = 16374, /**< ratedSpeed_DU */
        .VToFLUTnorm.cornerSpeedQ15 = 2035,  /**< cornerSpeed_DU */
        .VToFLUTnorm.ratedVoltQ15   = 16384, /**< ratedVolt_DU */
        .VToFLUTnorm.cornerVoltQ15  = 1026,  /**< cornerVolt_DU */
        .VToFLUTnorm.minVoltQ15     = 1026,  /**< minVolt_DU */
    };
/* *INDENT-ON* */
