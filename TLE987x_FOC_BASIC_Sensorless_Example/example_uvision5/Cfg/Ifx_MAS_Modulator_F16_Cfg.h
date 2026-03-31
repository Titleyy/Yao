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
 * \file Ifx_MAS_Modulator_F16_Cfg.h
 * \brief Component configuration generated from:
 *          python file: spec/Ifx_MAS_Modulator_F16.py
 *          class:       Ifx_MAS_Modulator_F16
 */

#ifndef IFX_MAS_MODULATOR_F16_CFG_H
#define IFX_MAS_MODULATOR_F16_CFG_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "stdbool.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/** minOnTime_DU */
#define IFX_MAS_MODULATOR_F16_CFG_MIN_ON_TIME_TICK (0)
/** tableSin60Sqrt3LUTSize_DU */
#define IFX_MAS_MODULATOR_F16_CFG_TABLESIN60SQRT3_LUT_SIZE (12)
/** currentMeasurementTopology_DU */
#define IFX_MAS_MODULATOR_F16_CFG_CURRENT_MEASUREMENT_TOPOLOGY (0)
/** brakeOutBehavior_DU */
#define IFX_MAS_MODULATOR_F16_CFG_BRAKE_OUT_BEHAVIOR (0)
/** faultOutBehavior_DU */
#define IFX_MAS_MODULATOR_F16_CFG_FAULT_OUT_BEHAVIOR (0)
/** faultReactionMaxAmplitude_DU */
#define IFX_MAS_MODULATOR_F16_CFG_FAULT_REACTION_MAX_AMPLITUDE (1)
/** faultReactionOvermodulation_DU */
#define IFX_MAS_MODULATOR_F16_CFG_FAULT_REACTION_OVERMODULATION (1)
/** enableFaultOut_DU */
#define IFX_MAS_MODULATOR_F16_CFG_ENABLE_FAULT_OUT (false)
/** faultOut_DU */
#define IFX_MAS_MODULATOR_F16_CFG_FAULT_OUT usrCallback

/******************************************************************************/
/*-------------------Global Exported Variables/Constants----------------------*/
/******************************************************************************/

#endif /* IFX_MAS_MODULATOR_F16_CFG_H */
