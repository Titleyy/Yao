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
 * \file Ifx_MHA_BridgeDrv_F16_TLE987_Cfg.h
 * \brief Component configuration generated from:
 *          python file: spec/Ifx_MHA_BridgeDrv_F16_TLE987.py
 *          class:       Ifx_MHA_BridgeDrv_F16_TLE987
 */

#ifndef IFX_MHA_BRIDGEDRV_F16_TLE987_CFG_H
#define IFX_MHA_BRIDGEDRV_F16_TLE987_CFG_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "stdbool.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/** transitionTimeCycles_DU */
#define IFX_MHA_BRIDGEDRV_F16_TLE987_CFG_TRANSITION_TIME_CYCLES (67)
/** enableFaultOut_DU */
#define IFX_MHA_BRIDGEDRV_F16_TLE987_CFG_ENABLE_FAULT_OUT (false)
/** faultReactionOvervolt_DU */
#define IFX_MHA_BRIDGEDRV_F16_TLE987_CFG_FAULT_REACTION_OVERVOLT (3)
/** faultReactionUndervolt_DU */
#define IFX_MHA_BRIDGEDRV_F16_TLE987_CFG_FAULT_REACTION_UNDERVOLT (3)
/** faultOut_DU */
#define IFX_MHA_BRIDGEDRV_F16_TLE987_CFG_FAULT_OUT usrFaultCallback

/******************************************************************************/
/*-------------------Global Exported Variables/Constants----------------------*/
/******************************************************************************/

#endif /* IFX_MHA_BRIDGEDRV_F16_TLE987_CFG_H */
