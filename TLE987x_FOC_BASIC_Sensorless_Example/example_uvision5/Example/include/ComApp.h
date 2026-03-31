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
 * \file ComApp.h
 * \brief Communication stack instantiation
 */

#ifndef COMAPP_H
#define COMAPP_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include <stdint.h>
#include "Ifx_Math.h"
#include "ident.h"

/******************************************************************************/
/*-----------------------Exported Variables/Constants-------------------------*/
/******************************************************************************/
extern volatile Ifx_Math_Fract16 referenceSpeed_rpm;
extern volatile bool             enablePowerStage;
extern volatile bool             enableControl;
extern volatile uint8_t          clrFaultFoc;
extern Ifx_Math_Fract16          phaseResistanceQ15;
extern volatile uint8_t          controlMode;
extern volatile bool             enableDirectInterface;
extern volatile bool             enableSpeedPreControl;

/******************************************************************************/
/*-------------------------Global Function Prototypes-------------------------*/
/******************************************************************************/

/**
 * \brief Initialize the comstack
 */
extern void ComApp_init(void);

/**
 * \brief Execute all tasks of the communication stack in time base slow.
 */
extern void ComApp_executeTaskSlow(void);

/**
 * \brief Execute all tasks of the communication stack in pulse width modulation.
 */
extern void ComApp_executeTaskPwm(void);

/**
 * \brief Execute all tasks of the communication stack in time base fast.
 */
extern void ComApp_executeTaskFast(void);

/**
 * \brief Execute all tasks of the communication stack in time base mid.
 * \return false - no communication timeout or com_g_firstPackageReceived is false.
 * true when communication timeout happens, no package is received for COM_CFG_TIMEOUT_US.
 */
extern bool ComApp_executeTaskMid(void);

/**
 * \brief Return DTI communication timeout status
 * \return false - no communication timeout or com_g_firstPackageReceived is false.
 * true when communication timeout happens, no package is received for COM_CFG_TIMEOUT_US.
 */
extern bool ComApp_getComTimeout(void);

#endif /* COMAPP_H */
