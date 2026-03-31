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
 * \file Ifx_CHA_DTI.h
 * \brief Header file declaring the API for "Debug and tune interface" (DTI)
 * as it is needed for parameter handling of MExT motor control library.
 */

#ifndef IFX_CHA_DTI_H
#define IFX_CHA_DTI_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_Common.h"

/* API Callback functions of DTI stack */
#include "parameterHandler_api.h"

/******************************************************************************/
/*-------------------------Public Unit Specification--------------------------*/
/******************************************************************************/

/**
 *  \brief Returns the component ID
 *
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 *
 */
void Ifx_CHA_DTI_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Returns the component version
 *
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 *
 */
void Ifx_CHA_DTI_getVersion(const Ifx_ComponentVersion** componentVersion);

/* API functions of DTI stack */
void Ifx_CHA_DTI_init(void);
void Ifx_CHA_DTI_process(void);

/* Copied here from TSDriver.h
 * This is the only API callback function from TSDriver that must be implemented in the user application. */
uint32_t getRuntimeCounter(void);

#endif /* IFX_CHA_DTI_H */
