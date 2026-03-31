/*
 * Copyright (c) 2024 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG, its affiliates or its licensees. If and as long as no
 * such terms of use are agreed, use of this file is subject to the Evaluation Software License Agreement distributed
 * along with this file within the software delivery package.
 *
 */

/* Include all header files of this component to make sure that they are statically checked */
#include "Ifx_Common.h"
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "Ifx_Math.h"
#include "Ifx_Math_UsrOpt.h"

/* Macros to define the component ID */
#define IFX_COMMON_COMPONENTID_SOURCEID     ((uint8_t)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_COMMON_COMPONENTID_LIBRARYID    ((uint16_t)Ifx_ComponentID_LibraryID_math)
#define IFX_COMMON_COMPONENTID_MODULEID     (4U)
#define IFX_COMMON_COMPONENTID_COMPONENTID1 (1U)
#define IFX_COMMON_COMPONENTID_COMPONENTID2 ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_COMMON_COMPONENTVERSION_MAJOR   (2U)
#define IFX_COMMON_COMPONENTVERSION_MINOR   (0U)
#define IFX_COMMON_COMPONENTVERSION_PATCH   (0U)
#define IFX_COMMON_COMPONENTVERSION_T       (4U)
#define IFX_COMMON_COMPONENTVERSION_REV     (0U)

/* *INDENT-OFF* */
/* Component ID */
static const Ifx_ComponentID      Ifx_Common_componentID = {
    .sourceID       = IFX_COMMON_COMPONENTID_SOURCEID,
    .libraryID      = IFX_COMMON_COMPONENTID_LIBRARYID,
    .moduleID       = IFX_COMMON_COMPONENTID_MODULEID,
    .componentID1   = IFX_COMMON_COMPONENTID_COMPONENTID1,
    .componentID2   = IFX_COMMON_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_Common_componentVersion = {
    .major = IFX_COMMON_COMPONENTVERSION_MAJOR,
    .minor = IFX_COMMON_COMPONENTVERSION_MINOR,
    .patch = IFX_COMMON_COMPONENTVERSION_PATCH,
    .t     = IFX_COMMON_COMPONENTVERSION_T,
    .rev   = IFX_COMMON_COMPONENTVERSION_REV
};
/* *INDENT-ON* */

/* polyspace-begin MISRA-C3:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */

/* Function to get the component ID */
void Ifx_Common_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_Common_componentID;
}


/* Function to get the component version */
void Ifx_Common_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_Common_componentVersion;
}


/* polyspace-end MISRA-C3:D4.14 [Justified:Low] "The caller function has to guarantee that NULL is not passed as
 * argument." */
