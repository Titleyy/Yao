/*
 * Copyright (c) 2024 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG, its affiliates or its licensees. If and as long as no
 * such terms of use are agreed, use of this file is subject to the Evaluation Software License Agreement distributed
 * along with this file within the software delivery package.
 *
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "MctrlExample.h"
#include "ComApp.h"
#include "Ifx_Math_Arithmetic_F16.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/******************************************************************************/
/*------------------------------Type Definitions------------------------------*/
/******************************************************************************/
/******************************************************************************/
/*-------------------------------Enumerations---------------------------------*/
/******************************************************************************/
/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/
/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/******************************************************************************/
/*-----------------------Exported Variables/Constants-------------------------*/
/******************************************************************************/
__USED Ifx_MS_FocSolution_F16 FocDemoClosedLoop;

/* *INDENT-OFF* */
Ifx_MS_FocSolution_F16_ParamComposition g_paramComposition = {
    .fluxEstimatorParam  = &g_fluxEstimatorParam,
    .focControllerParam  = &g_focControllerParam,
    .iToFControllerParam = &g_iToFParam,
    .modulatorParam      = &g_modulatorParam,
    .vToFControllerParam = &g_vToFControllerParam
};

__USED MctrlExample g_mctrlExample =
{
    .param = &g_mctrlExampleParam,
};
/* *INDENT-ON* */
/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void MctrlExample_init(void)
{
    g_mctrlExample.p_rotorAlignCounter = g_mctrlExample.param->rotorAlignmentTime_ticks;
#if MCTRLEXAMPLE_CFG_USE_DEFAULT_PARAM_STRUCT == 1
    Ifx_MS_FocSolution_F16_init(&FocDemoClosedLoop, &Ifx_MS_FocSolution_F16_g_defaultParam,
        &Ifx_MS_FocSolution_F16_g_defaultParamComposition);
#else
    Ifx_MS_FocSolution_F16_init(&FocDemoClosedLoop, &g_focSolutionParam, &g_paramComposition);
#endif /* MCTRLEXAMPLE_CFG_USE_DEFAULT_PARAM_STRUCT == 1 */
}


void MctrlExample_executeTaskSlow(void)
{
    /* Call clear fault API if requested */
    if (g_mctrlExample.param->clrFaultFoc == true)
    {
        Ifx_MS_FocSolution_F16_clearFault(&FocDemoClosedLoop);
        g_mctrlExample.param->clrFaultFoc = false;
    }

    /* Call FOC enable */
    Ifx_MS_FocSolution_F16_enablePowerStage(&FocDemoClosedLoop, (bool)g_mctrlExample.param->enablePowerStage);
    Ifx_MS_FocSolution_F16_setControlMode(&FocDemoClosedLoop,
        (Ifx_MS_FocSolution_F16_ControlMode)g_mctrlExample.param->controlMode);
}


void MctrlExample_executeTaskMid(void)
{
    Ifx_Math_Fract16    referenceSpeedN     = 0;
    Ifx_Math_CmpFract16 currentsDqReference = {0, 0};
    bool                motorStopTimeout    = false;
    bool                enableControl       = g_mctrlExample.param->enableControl;
    motorStopTimeout = ComApp_getComTimeout();

    if (motorStopTimeout == true)
    {
        enableControl = false;
    }

    /* If speed control or power stage disabled */
    if ((enableControl == false)
        || (g_mctrlExample.param->enablePowerStage == false))
    {
        /* Set alignment counter to init. value */
        g_mctrlExample.p_rotorAlignCounter = g_mctrlExample.param->rotorAlignmentTime_ticks;
    }

    /* If speed control enabled */
    else if (enableControl == true)
    {
        /* Rotor alignment finished */
        if (g_mctrlExample.p_rotorAlignCounter == 0)
        {
            /* Calculate the normalized speed from the ref. and base speed */
            referenceSpeedN = Ifx_Math_DivSat_F16(g_mctrlExample.param->referenceSpeed_rpm,
                MCTRLEXAMPLE_CFG_BASE_MECH_SPEED_RPM);
        }

        /* Rotor alignment ongoing */
        else
        {
            /* During rotor alignment ref. speed stays 0 */
            /* Decrement rotor alignment counter */
            g_mctrlExample.p_rotorAlignCounter = g_mctrlExample.p_rotorAlignCounter - 1;
        }
    }

    Ifx_MS_FocSolution_F16_executeSpeedControl(&FocDemoClosedLoop, referenceSpeedN, currentsDqReference);
    Ifx_MS_FocSolution_F16_enableControl(&FocDemoClosedLoop, enableControl);
}


bool MctrlExample_isMotorRunning(void)
{
    bool result;

    if ((FocDemoClosedLoop.p_status.state == Ifx_MS_FocSolution_F16_State_off)
        && (FocDemoClosedLoop.p_enableControl == false)
        && (FocDemoClosedLoop.p_enablePowerStage == false))
    {
        result = false;
    }
    else
    {
        result = true;
    }

    return result;
}
