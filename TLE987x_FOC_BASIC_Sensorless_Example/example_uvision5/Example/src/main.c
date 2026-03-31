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
/* CMSIS */
#include "cmsis_compiler.h"

/* Peripheral configuration */
#include "tle_device.h"

/* Motor Control Embedded Software Library */
#include "Ifx_FHA_TbStreaming.h"
#include "Ifx_MS_FocSolution_F16.h"
#include "Ifx_MS_FocSolution_F16_Cfg.h"
#include "Ifx_MHA_DriverGroup_F16.h"
#include "Ifx_Math_DivSat.h"

/* Application includes */
#include "ComApp.h"
#include "MctrlExample.h"
#include "SignalList.h"

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
/* 0: streaming is stopped; 1: streaming is active  */
__USED volatile uint8_t u8StreamingON = 1;

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/

#if (IFX_MS_FOCSOLUTION_F16_CFG_CURRENT_LOOP_FACTOR > 1)

/* Fast time base execution call back, PendSV_Handler software interrupt defined in start up file */
void PendSV_Handler(void)
{
    Ifx_MS_FocSolution_F16_executeControlMode(&FocDemoClosedLoop);

    if (u8StreamingON != 0)
    {
        /* Streaming is invoked just after the controller is executed in order to send the newest values immediately */
        Ifx_FHA_TbStreaming_tick();
    }
}


#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_CURRENT_LOOP_FACTOR > 1) */
/* PWM time base CCU6 one match callback */
void Ifx_FOC_oneMatchCallback(void)
{
    static int8_t com_counter;
    ComApp_executeTaskPwm();

    if (com_counter >= 3)
    {
        ComApp_executeTaskFast();
        com_counter = 0;
    }
    else
    {
        ++com_counter;
    }

#if (IFX_MS_FOCSOLUTION_F16_CFG_CURRENT_LOOP_FACTOR > 1)
    bool retVal = false;
    retVal = Ifx_MHA_DriverGroup_F16_onOneMatchCallback(&FocDemoClosedLoop.driverGroup);

    if (retVal)
    {
        CPU->ICSR.bit.PENDSVSET = 1U;
    }

#else

    (void)Ifx_MHA_DriverGroup_F16_onOneMatchCallback(&FocDemoClosedLoop.driverGroup);
    Ifx_MS_FocSolution_F16_executeControlMode(&FocDemoClosedLoop);

    if (u8StreamingON != 0)
    {
        Ifx_FHA_TbStreaming_tick();
    }

#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_CURRENT_LOOP_FACTOR > 1) */
}


/* PWM time base CCU6 period match callback */
void Ifx_FOC_periodMatchCallback(void)
{
    Ifx_MHA_DriverGroup_F16_onPeriodMatchCallback(&FocDemoClosedLoop.driverGroup);
}


/* Mid time base speed loop execution call back which is called by timer GPT2.T6 */
void Ifx_FOC_speedLoop_callback(void)
{
    MctrlExample_executeTaskMid();
    ComApp_executeTaskMid();
}


int main(void)
{
    /* Initialize peripherals based on IFXConfigWizard configuration */
    TLE_Init();

    /* Initalize TraceBox streaming*/
    Ifx_FHA_TbStreaming_init();
    SignalList_init();

    /* Start Timer3 high and low byte timer */
    TIMER3_T3HL_Start();
#if (IFX_MS_FOCSOLUTION_F16_CFG_CURRENT_LOOP_FACTOR > 1)

    /* Set PendSV interrupt priority
     * Prepares fast time base.
     */
    NVIC_SetPriority(PendSV_IRQn, 2);
#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_CURRENT_LOOP_FACTOR > 1) */
    MctrlExample_init();
    ComApp_init();

    /* Start speed loop timer.
     * Mid time base is running. */
    GPT12E_T6_Start();

    /* At the end of configuration start CCU6 T12 to trigger one match and period match
     * PWM time base is running and also triggers the fast time base (PendSV). */
    CCU6_StartTmr_T12();

    /* Start slow time base or main loop*/
    for ( ; ;)
    {
        (void)WDT1_Service();
        MctrlExample_executeTaskSlow();
        ComApp_executeTaskSlow();
    }
}
