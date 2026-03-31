/*
 * Copyright (c) 2024 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG, its affiliates or its licensees. If and as long as no
 * such terms of use are agreed, use of this file is subject to the Evaluation Software License Agreement distributed
 * along with this file within the software delivery package.
 *
 */

#ifndef IFX_MDA_STARTANGLEIDENT_F16_CFG_H
#define IFX_MDA_STARTANGLEIDENT_F16_CFG_H

/* The static configuration of component Ifx_MDA_StartAngleIdent_F16 is currently not supported by Motix Solution
 * Designer, so this static configuration header is necessary to be able to successfully build the entire motor control
 * library. */

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_AVERAGING_CYCLES                   (0x8)  /*decimal 8*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_DEADTIME_TICK                      (0x14) /*decimal 20*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_DRIVERDELAY_TICK                   (0x8)  /*decimal 8*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_ENABLE_FAULT_OUT                   (0x0)  /*decimal 0*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_FAULT_OUT                          usrFaultCallback

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_FAULT_OUT_BEHAVIOR                 (0x0)    /*decimal 0*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_FAULT_REACTION_VOLTAGE_FLUCTUATION (0x3)    /*decimal 3*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_MEASUREMENTTIME_TICK               (0x30)   /*decimal 48*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_NEGATIVE_PULSE_LENGTH_CYCLES       (0x2)    /*decimal 2*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_PERIOD_TICK                        (0x3E7)  /*decimal 999*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_POSITIVE_PULSE_LENGTH_CYCLES       (0x2)    /*decimal 2*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_RINGINGTIME_TICK                   (0x14)   /*decimal 20*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_VOLTAGE_FLUCTUATION_RANGE_Q15      (0x1999) /*decimal 6553*/

#define IFX_MDA_STARTANGLEIDENT_F16_CFG_ZERO_PULSE_LENGTH_CYCLES           (0x6)    /*decimal 6*/

#endif /* IFX_MDA_STARTANGLEIDENT_F16_CFG_H */
