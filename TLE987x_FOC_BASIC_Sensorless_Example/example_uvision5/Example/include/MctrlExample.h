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
 * \file MotorControl.h
 * \brief Motorcontrol stack instantiation
 */

#ifndef MCTRLEXAMPLE_H
#define MCTRLEXAMPLE_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "MctrlExample_Cfg.h"
#include "Ifx_MS_FocSolution_F16.h"
#include "Ifx_MAS_Modulator_F16.h"
#include "Ifx_MDA_FluxEstimator_F16.h"
#include "Ifx_MDA_VToFController_F16.h"
#include "Ifx_MDA_FocController_F16.h"
#include "Ifx_MDA_IToFController_F16.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/******************************************************************************/
/*------------------------------Type Definitions------------------------------*/
/******************************************************************************/
typedef struct MctrlExample_Param
{
    Ifx_Math_Fract16                   referenceSpeed_rpm;
    bool                               enablePowerStage;
    bool                               enableControl;
    Ifx_MS_FocSolution_F16_ControlMode controlMode;

    /* Set to 1 to clear FOC fault */
    bool clrFaultFoc;

    /* Set time for rotor alignment */
    uint16_t rotorAlignmentTime_ticks;
}MctrlExample_Param;
typedef struct MctrlExample
{
    MctrlExample_Param* param;
    uint16_t            p_rotorAlignCounter;
}MctrlExample;

/******************************************************************************/
/*-------------------------------Enumerations---------------------------------*/
/******************************************************************************/
/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/******************************************************************************/
/*-------------------Global Exported Variables/Constants----------------------*/
/******************************************************************************/
/** Self struct for mctrl stack. */
extern Ifx_MS_FocSolution_F16 FocDemoClosedLoop;

/** Exports for component parameter instances used in mctrl stack */
extern MctrlExample_Param     g_mctrlExampleParam;

#if MCTRLEXAMPLE_CFG_USE_DEFAULT_PARAM_STRUCT == 1

/* Redirect instances to default parameter structs. Temporary solution to use the same ParamTable for the default param
 * structs which enable MTB regression tests as well as for the generated ones. */
#define g_focSolutionParam    Ifx_MS_FocSolution_F16_g_defaultParam
#define g_modulatorParam      Ifx_MAS_Modulator_F16_g_defaultParam
#define g_fluxEstimatorParam  Ifx_MDA_FluxEstimator_F16_g_defaultParam
#define g_vToFControllerParam Ifx_MDA_VToFController_F16_g_defaultParam
#define g_focControllerParam  Ifx_MDA_FocController_F16_g_defaultParam
#define g_iToFParam           Ifx_MDA_IToFController_F16_g_defaultParam

#else

extern Ifx_MS_FocSolution_F16_Param     g_focSolutionParam;
extern Ifx_MAS_Modulator_F16_Param      g_modulatorParam;
extern Ifx_MDA_FluxEstimator_F16_Param  g_fluxEstimatorParam;
extern Ifx_MDA_FocController_F16_Param  g_focControllerParam;
extern Ifx_MDA_IToFController_F16_Param g_iToFParam;
extern Ifx_MDA_VToFController_F16_Param g_vToFControllerParam;

#endif /* MCTRLEXAMPLE_CFG_USE_DEFAULT_PARAM_STRUCT == 1 */
/******************************************************************************/
/*-------------------------Inline Function Prototypes-------------------------*/
/******************************************************************************/
/******************************************************************************/
/*-------------------------Global Function Prototypes-------------------------*/
/******************************************************************************/

/**
 * \brief Init the motor control stack.
 */
extern void MctrlExample_init(void);

/**
 * \brief Execute all tasks of the motor control stack in time base slow.
 */
extern void MctrlExample_executeTaskSlow(void);

/**
 * \brief Execute all tasks of the motor control stack in time base mid.
 */
extern void MctrlExample_executeTaskMid(void);

/**
 * \brief Set reference speed upper limit.
 */
extern void MctrlExample_setRefSpeedUpperLimit(Ifx_Math_Fract16 upperLimit);

/**
 * \brief Set reference speed lower limit.
 */
extern void MctrlExample_setRefSpeedLowerLimit(Ifx_Math_Fract16 lowerLimit);

/**
 * \brief Returns wether the motor is running
 * \return true: Power stage enable or control enable request is pending or foc solution state is not off.
 * Also fault state or power stage enabling is in progress is considered as motor running.
 * false: No request is pending and foc solution state is off
 */
extern bool MctrlExample_isMotorRunning(void);

/******************************************************************************/
/*---------------------Inline Function Implementations------------------------*/
/******************************************************************************/
#endif /* MCTRLEXAMPLE_H */
