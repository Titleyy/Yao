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
 * \file Ifx_MDA_FocController_F16_Cfg.c
 * \brief Component configuration generated from:
 *          python file: spec/Ifx_MDA_FocController_F16.py
 *          class:       Ifx_MDA_FocController_F16
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_MDA_FocController_F16.h"

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
Ifx_MDA_FocController_F16_Param g_focControllerParam =
    {
        .currentDPiIntegGainSamplingTime      = 511,    /**< currentDPiIntegGainSamplingTime_DU */
        .currentDPiLowerLimitQ15              = -10000, /**< currentDPiLowerLimit_DU */
        .currentDPiUpperLimitQ15              = 10000,  /**< currentDPiUpperLimit_DU */
        .currentDPiPropGain                   = 4778,   /**< currentDPiPropGain_DU */
        .currentDPiAntiWindupGainSamplingTime = 49,     /**< currentDPiAntiWindupGainSamplingTime_DU */
        .currentQPiIntegGainSamplingTime      = 511,    /**< currentQPiIntegGainSamplingTime_DU */
        .currentQPiLowerLimitQ15              = -10000, /**< currentQPiLowerLimit_DU */
        .currentQPiUpperLimitQ15              = 10000,  /**< currentQPiUpperLimit_DU */
        .currentQPiPropGain                   = 4778,   /**< currentQPiPropGain_DU */
        .currentQPiAntiWindupGainSamplingTime = 49,     /**< currentQPiAntiWindupGainSamplingTime_DU */
    };
/* *INDENT-ON* */
