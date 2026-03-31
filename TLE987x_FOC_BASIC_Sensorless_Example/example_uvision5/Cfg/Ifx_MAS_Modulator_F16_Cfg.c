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
 * \file Ifx_MAS_Modulator_F16_Cfg.c
 * \brief Component configuration generated from:
 *          python file: spec/Ifx_MAS_Modulator_F16.py
 *          class:       Ifx_MAS_Modulator_F16
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_MAS_Modulator_F16.h"

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
Ifx_MAS_Modulator_F16_Param g_modulatorParam =
    {
        .measurementTime_tick                  = 48,    /**< measurementTime_DU */
        .deadTime_tick                         = 20,    /**< deadTime_DU */
        .driverDelay_tick                      = 8,     /**< driverDelay_DU */
        .ringingTime_tick                      = 20,    /**< ringingTime_DU */
        .period_tick                           = 2000,  /**< period_DU */
        .measurementPoint                      = 0,     /**< measurementPoint_DU */
        .maxAmplitudeQ15                       = 13654, /**< maxAmplitude_DU */
        .biDirectionalShiftingThresholdHighQ15 = 3277,  /**< biDirectionalShiftingThresholdHigh_DU */
        .biDirectionalShiftingThresholdLowQ15  = 2621,  /**< biDirectionalShiftingThresholdLow_DU */
    };
/* *INDENT-ON* */
