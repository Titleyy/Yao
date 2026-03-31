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
 * \file Ifx_MDA_FluxEstimator_F16_Cfg.c
 * \brief Component configuration generated from:
 *          python file: spec/Ifx_MDA_FluxEstimator_F16.py
 *          class:       Ifx_MDA_FluxEstimator_F16
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_MDA_FluxEstimator_F16.h"

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
Ifx_MDA_FluxEstimator_F16_Param g_fluxEstimatorParam =
    {
        .samplingTime_us             = 150,     /**< samplingTime_DU */
        .alphaFilter_timeConstant_us = 9947,    /**< alphaFilterTimeConstant_DU */
        .betaFilter_timeConstant_us  = 9947,    /**< betaFilterTimeConstant_DU */
        .speedFilter_timeConstant_us = 9947,    /**< speedFilterTimeConstant_DU */
        .pllFilter_propGain          = 2000000, /**< pllFilterPropGain_DU */
        .phaseResQ15                 = 5120,    /**< phaseRes_DU */
        .phaseIndAdjustedQ15         = 720,     /**< phaseIndAdjusted_DU */
        .alphaGainAdjustedQ14        = 16383,   /**< alphaGainAdjusted_DU */
        .betaGainAdjustedQ14         = 16383,   /**< betaGainAdjusted_DU */
        .systemBaseTimeQ30           = 4027671, /**< systemBaseTime_DU */
    };
/* *INDENT-ON* */
