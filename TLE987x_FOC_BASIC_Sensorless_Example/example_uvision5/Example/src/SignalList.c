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
#include "Ifx_MS_FocSolution_F16.h"
#include "Ifx_MHA_DriverGroup_F16.h"
#include "Ifx_Math_DivSat.h"
#include "Ifx_FHA_TbStreaming.h"
#include "Ifx_FHA_TbStreaming_TLE987_Cfg.h"

/* Application includes */
#include "MctrlExample.h"
#include "SignalList.h"

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void SignalList_init(void)
{
    /*Signal 0 is configured in tbstreaming init to send a sample counter.*/
    Ifx_FHA_TbStreaming_setTxPtr16b(1,
        (uint16_t*)&FocDemoClosedLoop.driverGroup.measurementADC.p_rawCurrentMeasurements[FocDemoClosedLoop.
                                                                                          driverGroup.
                                                                                          measurementADC.
                                                                                          p_rawCurrentMeasurementsReadIndex
        ][1]);
    Ifx_FHA_TbStreaming_setTxPtr16b(2, (uint16_t*)&FocDemoClosedLoop.driverGroup.p_output.currentsUVW.u);
    Ifx_FHA_TbStreaming_setTxPtr16b(3, (uint16_t*)&FocDemoClosedLoop.driverGroup.p_output.currentsUVW.v);
    Ifx_FHA_TbStreaming_setTxPtr16b(4, (uint16_t*)&FocDemoClosedLoop.driverGroup.p_output.currentsUVW.w);
    Ifx_FHA_TbStreaming_setTxPtr16b(5, (uint16_t*)&FocDemoClosedLoop.currentsAlphaBeta.imag);
    Ifx_FHA_TbStreaming_setTxPtr16b(6, (uint16_t*)&FocDemoClosedLoop.currentsAlphaBeta.real);
    Ifx_FHA_TbStreaming_setTxPtr16b(7, (uint16_t*)&FocDemoClosedLoop.focController.currentDQ.imag);
    Ifx_FHA_TbStreaming_setTxPtr16b(8, (uint16_t*)&FocDemoClosedLoop.focController.currentDQ.real);
    Ifx_FHA_TbStreaming_setTxPtr16b(9, (uint16_t*)&FocDemoClosedLoop.fluxEstimator.p_output.anglePLL);
    Ifx_FHA_TbStreaming_setTxPtr16b(10, (uint16_t*)&FocDemoClosedLoop.fluxEstimator.p_output.speedQ15);
    Ifx_FHA_TbStreaming_setTxPtr16b(11, (uint16_t*)&FocDemoClosedLoop.rateLimitInSpeedQ15);
    Ifx_FHA_TbStreaming_setTxPtr16b(12, (uint16_t*)&FocDemoClosedLoop.dqCommand.imag);
    Ifx_FHA_TbStreaming_setTxPtr16b(13, (uint16_t*)&FocDemoClosedLoop.dqCommand.real);
    Ifx_FHA_TbStreaming_setTxPtr16b(14, (uint16_t*)&FocDemoClosedLoop.focController.voltageDQ.imag);
    Ifx_FHA_TbStreaming_setTxPtr16b(15, (uint16_t*)&FocDemoClosedLoop.focController.voltageDQ.real);

    /* 8-Bit signals */
    Ifx_FHA_TbStreaming_setTxPtr8b(16, (uint8_t*)&FocDemoClosedLoop.p_status.actualControlMode);
    Ifx_FHA_TbStreaming_setTxPtr8b(17, (uint8_t*)&FocDemoClosedLoop.p_status.state);
    Ifx_FHA_TbStreaming_setTxPtr8b(18, (uint8_t*)&FocDemoClosedLoop.p_status.subState);
    Ifx_FHA_TbStreaming_setTxPtr8b(19, (uint8_t*)&FocDemoClosedLoop.driverGroup.bridgeDrv.p_status.state);
    Ifx_FHA_TbStreaming_setTxPtr8b(20, (uint8_t*)&FocDemoClosedLoop.driverGroup.measurementADC.p_status.state);
    Ifx_FHA_TbStreaming_setTxPtr8b(21, (uint8_t*)&FocDemoClosedLoop.driverGroup.patternGen.p_status.state);
    Ifx_FHA_TbStreaming_setTxPtr8b(22, (uint8_t*)&FocDemoClosedLoop.modulator.p_status.state);
    Ifx_FHA_TbStreaming_setTxPtr8b(23, (uint8_t*)&FocDemoClosedLoop.modulator.p_status.subState);
}
