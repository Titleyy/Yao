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
#include "Ifx_MHA_DriverGroup_F16.h"
#include "Ifx_MHA_DriverGroup_F16_Cfg.h"

/* Math library */
#include "Ifx_Math_CurrentReconstruction.h"

/* SDK */
#include "gpt12e.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/* Macros to define the component ID */
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTID_SOURCEID     ((uint8_t)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTID_LIBRARYID    ((uint16_t)Ifx_ComponentID_LibraryID_mctrlHardwareAbstraction)
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTID_MODULEID     (3U)
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTID_COMPONENTID1 (2U)
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTID_COMPONENTID2 ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_MAJOR   (2U)
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_MINOR   (0U)
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_PATCH   (0U)
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_T       (4U)
#define IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_REV     (0U)

/* Value when a 16 bit timer overruns (e.g. GPT12E T6) */
#define IFX_MHA_DRIVERGROUP_F16_GPT12E_T6_OVERRUN        (UINT16_MAX + 1)

/******************************************************************************/
/*-----------------------Static Function Prototypes---------------------------*/
/******************************************************************************/

/**
 * \brief Request to enable/disable MeasurementADC
 */
static void Ifx_MHA_DriverGroup_F16_enableMeasurementADC(Ifx_MHA_DriverGroup_F16* self, bool enable);

/**
 * \brief Request to enable/disable BridgeDrv
 */
static void Ifx_MHA_DriverGroup_F16_enableBridgeDrv(Ifx_MHA_DriverGroup_F16* self, bool enable);

/**
 * \brief Request to enable/disable PatternGen
 */
static void Ifx_MHA_DriverGroup_F16_enablePatternGen(Ifx_MHA_DriverGroup_F16* self, bool enable);

/**
 * \brief Set the speed loop period in us.
 * Calculates and sets the needed reload value for GPT2 T6.
 * In case speed loop period is too long the reload value is limited to minimum 0.
 * \param self unused
 * \param speedLoopPeriod_us A period in us that is in range of the 16bit timer GPT2_T6. Range:
 * [1..UINT16_MAX/GPT2_T6_CLK]
 *
 */
static void Ifx_MHA_DriverGroup_F16_setSpeedLoopPeriod(Ifx_MHA_DriverGroup_F16* self, uint16_t speedLoopPeriod_us);

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Component ID */
static const Ifx_ComponentID      Ifx_MHA_DriverGroup_F16_componentID = {
    .sourceID     = IFX_MHA_DRIVERGROUP_F16_COMPONENTID_SOURCEID,
    .libraryID    = IFX_MHA_DRIVERGROUP_F16_COMPONENTID_LIBRARYID,
    .moduleID     = IFX_MHA_DRIVERGROUP_F16_COMPONENTID_MODULEID,
    .componentID1 = IFX_MHA_DRIVERGROUP_F16_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_MHA_DRIVERGROUP_F16_COMPONENTID_COMPONENTID2,
};


/* Component Version */
static const Ifx_ComponentVersion Ifx_MHA_DriverGroup_F16_componentVersion = {
    .major = IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_MAJOR,
    .minor = IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_MINOR,
    .patch = IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_PATCH,
    .t     = IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_T,
    .rev   = IFX_MHA_DRIVERGROUP_F16_COMPONENTVERSION_REV
};
/* *INDENT-ON* */
/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
/* Function to get the component ID */
void Ifx_MHA_DriverGroup_F16_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MHA_DriverGroup_F16_componentID;
}


/* Function to get the component version */
void Ifx_MHA_DriverGroup_F16_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MHA_DriverGroup_F16_componentVersion;
}


void Ifx_MHA_DriverGroup_F16_initModules(Ifx_MHA_DriverGroup_F16* self)
{
    /* Initialize hardware abstraction modules */
    Ifx_MHA_MeasurementADC_F16_TLE987_init(&(self->measurementADC));
    Ifx_MHA_BridgeDrv_F16_TLE987_init(&(self->bridgeDrv));
    Ifx_MHA_PatternGen_F16_TLE987_init(&(self->patternGen));
    Ifx_MHA_DriverGroup_F16_setSpeedLoopPeriod(self, IFX_MHA_DRIVERGROUP_F16_CFG_SPEED_LOOP_PERIOD_US);
}


static void Ifx_MHA_DriverGroup_F16_setSpeedLoopPeriod(Ifx_MHA_DriverGroup_F16* self, uint16_t speedLoopPeriod_us)
{
    (void)self;
    uint16_t reloadValue           = 0U;
    uint32_t speedLoopPeriod_ticks = ((uint32_t)GPT2_T6_CLK * (uint32_t)speedLoopPeriod_us);

    /* Calculate T6 reload value in ticks for up counting timer configuration. */
    if (speedLoopPeriod_ticks < IFX_MHA_DRIVERGROUP_F16_GPT12E_T6_OVERRUN)
    {
        reloadValue = (uint16_t)((uint32_t)IFX_MHA_DRIVERGROUP_F16_GPT12E_T6_OVERRUN - speedLoopPeriod_ticks);
    }

    GPT12E_T6_Reload_Value_Set(reloadValue);
}


void Ifx_MHA_DriverGroup_F16_enableModules(Ifx_MHA_DriverGroup_F16* self, bool enable)
{
    /* Enable/disable pattern generator */
    Ifx_MHA_DriverGroup_F16_enablePatternGen(self, enable);

    /* Enable/disable measurement ADC */
    Ifx_MHA_DriverGroup_F16_enableMeasurementADC(self, enable);

    /* Enable/disable bridge driver */
    Ifx_MHA_DriverGroup_F16_enableBridgeDrv(self, enable);
}


static void Ifx_MHA_DriverGroup_F16_enableMeasurementADC(Ifx_MHA_DriverGroup_F16* self, bool enable)
{
    /* Variables to store the status of the pattern generator and bridge driver */
    Ifx_MHA_BridgeDrv_F16_TLE987_Status  bridgeDrvStatus;
    Ifx_MHA_PatternGen_F16_TLE987_Status patternGenStatus;

    /* Get the bridge driver status */
    bridgeDrvStatus = Ifx_MHA_BridgeDrv_F16_TLE987_getStatus(&(self->bridgeDrv));

    /* Get the pattern generator status */
    patternGenStatus = Ifx_MHA_PatternGen_F16_TLE987_getStatus(&self->patternGen);

    if ((enable == true)
        && (patternGenStatus.state == Ifx_MHA_PatternGen_F16_TLE987_State_on))
    {
        Ifx_MHA_MeasurementADC_F16_TLE987_enable(&(self->measurementADC), true);
    }
    else if ((enable == false)
             && (bridgeDrvStatus.state == Ifx_MHA_BridgeDrv_F16_TLE987_State_off))
    {
        Ifx_MHA_MeasurementADC_F16_TLE987_enable(&(self->measurementADC), false);
    }
    else
    {
        /* Do nothing */
    }
}


static void Ifx_MHA_DriverGroup_F16_enablePatternGen(Ifx_MHA_DriverGroup_F16* self, bool enable)
{
    /* Variables to store the status of the measurement ADC */
    Ifx_MHA_MeasurementADC_F16_TLE987_Status measurementADCStatus;

    /* Get the bridge driver status */
    Ifx_MHA_BridgeDrv_F16_TLE987_Status      bridgeDrvStatus;

    /* Get the measurement ADC status */
    measurementADCStatus = Ifx_MHA_MeasurementADC_F16_TLE987_getStatus(&self->measurementADC);
    bridgeDrvStatus      = Ifx_MHA_BridgeDrv_F16_TLE987_getStatus(&(self->bridgeDrv));

    if (enable == true)
    {
        Ifx_MHA_PatternGen_F16_TLE987_enable(&(self->patternGen), true);
    }
    else if ((bridgeDrvStatus.state == Ifx_MHA_BridgeDrv_F16_TLE987_State_off)
             && (measurementADCStatus.state == Ifx_MHA_MeasurementADC_F16_TLE987_State_off))
    {
        Ifx_MHA_PatternGen_F16_TLE987_enable(&(self->patternGen), false);
    }
    else
    {
        /* Do nothing */
    }
}


static void Ifx_MHA_DriverGroup_F16_enableBridgeDrv(Ifx_MHA_DriverGroup_F16* self, bool enable)
{
    /* Variable to store the status of bridge driver */
    Ifx_MHA_PatternGen_F16_TLE987_Status     patternGenStatus;
    Ifx_MHA_MeasurementADC_F16_TLE987_Status measurementADCStatus;

    /* Get the pattern generator status */
    patternGenStatus = Ifx_MHA_PatternGen_F16_TLE987_getStatus(&self->patternGen);

    /* Get the measurement ADC status */
    measurementADCStatus = Ifx_MHA_MeasurementADC_F16_TLE987_getStatus(&self->measurementADC);

    if (enable == false)
    {
        Ifx_MHA_BridgeDrv_F16_TLE987_enable(&(self->bridgeDrv), false);
    }
    else if ((enable == true)
             && (patternGenStatus.state == Ifx_MHA_PatternGen_F16_TLE987_State_on)
             && (measurementADCStatus.state == Ifx_MHA_MeasurementADC_F16_TLE987_State_on))
    {
        Ifx_MHA_BridgeDrv_F16_TLE987_enable(&(self->bridgeDrv), true);
    }
    else
    {
        /* Do nothing */
    }
}


bool Ifx_MHA_DriverGroup_F16_checkFaults(Ifx_MHA_DriverGroup_F16* self)
{
    /* local variable to check fault status */
    bool faultStatusRet = false;

    /* Bridge driver fault */
    if (Ifx_MHA_BridgeDrv_F16_TLE987_getStatus(&(self->bridgeDrv)).state == Ifx_MHA_BridgeDrv_F16_TLE987_State_fault)
    {
        faultStatusRet = true;
    }

    /* Pattern generator fault */
    else if (Ifx_MHA_PatternGen_F16_TLE987_getStatus(&(self->patternGen)).state ==
             Ifx_MHA_PatternGen_F16_TLE987_State_fault)
    {
        faultStatusRet = true;
    }
    else
    {
        faultStatusRet = false;
    }

    /* Return the fault status */
    return faultStatusRet;
}


void Ifx_MHA_DriverGroup_F16_measureAndReconstruct(Ifx_MHA_DriverGroup_F16* self, Ifx_Math_CurrentReconstruction_info
                                                   currentReconstructionInfo)
{
    /* Variable declaration */
    Ifx_MHA_MeasurementADC_F16_TLE987_Output measurementADCOutput;

    /* Get measured shunt currents and DC link voltage */
    Ifx_MHA_MeasurementADC_F16_TLE987_execute(&(self->measurementADC));
    Ifx_MHA_MeasurementADC_F16_TLE987_getOutput(&(self->measurementADC), &measurementADCOutput);
    self->p_output.dcLinkVoltageQ15    = measurementADCOutput.dcLinkVoltageQ15;
    self->p_output.shuntCurrentsQ15[0] = measurementADCOutput.shuntCurrentsQ15[0];
    self->p_output.shuntCurrentsQ15[1] = measurementADCOutput.shuntCurrentsQ15[1];

    /* Reconstruct three phase currents from measured shunt currents */
    self->p_output.currentsUVW = Ifx_Math_CurrentReconstruction_F16(currentReconstructionInfo,
        measurementADCOutput.shuntCurrentsQ15);
}
