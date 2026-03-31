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
 * \file Ifx_Common.h
 * Module containing common type definitions
 */

#ifndef IFX_COMMON_H
#define IFX_COMMON_H
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"

/**
 *  \brief Returns the component ID
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 */
void Ifx_Common_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Returns the component version
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 */
void Ifx_Common_getVersion(const Ifx_ComponentVersion** componentVersion);

#endif /*IFX_COMMON_H*/
