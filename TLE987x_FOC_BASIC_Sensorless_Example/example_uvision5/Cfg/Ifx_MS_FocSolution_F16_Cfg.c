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
 * \file Ifx_MS_FocSolution_F16_Cfg.c
 * \brief Component configuration generated from:
 *          python file: spec/Ifx_MS_FocSolution_F16.py
 *          class:       Ifx_MS_FocSolution_F16
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_MS_FocSolution_F16.h"

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
Ifx_MS_FocSolution_F16_Param g_focSolutionParam =
    {
        .startUpCurrentQ15                 = 6553,   /**< startUpCurrent_DU */
        .startUpCurrentRampUpRateQ15       = 39,     /**< startUpCurrentRampUpRate_DU */
        .speedPiPropGain                   = 24000,  /**< speedPiPropGain_DU */
        .speedPiIntegGainSamplingTime      = 12,     /**< speedPiIntegGainSamplingTime_DU */
        .refCurrentUpperLimitQ15           = 13107,  /**< refCurrentUpperLimit_DU */
        .refCurrentLowerLimitQ15           = -13107, /**< refCurrentLowerLimit_DU */
        .refSpeedUpperLimitQ15             = 16384,  /**< refSpeedUpperLimit_DU */
        .refSpeedLowerLimitQ15             = -16384, /**< refSpeedLowerLimit_DU */
        .minimumSpeedThresholdQ15          = 0,      /**< minimumSpeedThreshold_DU */
        .speedRampUpRateOpenLoopQ30        = 402921, /**< speedRampUpRateOpenLoop_DU */
        .speedRampDownRateOpenLoopQ30      = 402921, /**< speedRampDownRateOpenLoop_DU */
        .speedRampUpRateClosedLoopQ30      = 402921, /**< speedRampUpRateClosedLoop_DU */
        .speedRampDownRateClosedLoopQ30    = 402921, /**< speedRampDownRateClosedLoop_DU */
        .qCurrentAtTransitionQ15           = 6553,   /**< qCurrentAtTransition_DU */
        .transitionAngleTolerance          = 1820,   /**< transitionAngleTolerance_DU */
        .transitionTimeLimit_cycles        = 67,     /**< transitionTimeLimit_DU */
        .transitionDownDCurrentScalingQ14  = 8192,   /**< transitionDownDCurrentScaling_DU */
        .transitionSpeedUpQ15              = 6553,   /**< transitionSpeedUp_DU */
        .transitionSpeedBandQ15            = 2458,   /**< transitionSpeedBand_DU */
        .inverseTorqueConstant             = 53658,  /**< inverseTorqueConstant_DU */
        .frictionConstant                  = 0,      /**< frictionConstant_DU */
        .rotorInertiaOverSamplingTime      = 23365,  /**< rotorInertiaOverSamplingTime_DU */
        .initStartupCurrentQ15             = 0,      /**< initStartupCurrent_DU */
        .speedPiAntiWindupGainSamplingTime = 60040,  /**< speedPiAntiWindupGain_DU */
    };
/* *INDENT-ON* */
