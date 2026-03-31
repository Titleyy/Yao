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
 * \file Ifx_MHA_DriverGroup_F16.h
 * /brief Interface component which provides a hardware agnostic interface to the hardware specific MHA components.
 */

#ifndef IFX_MHA_DRIVERGROUP_F16_H
#define IFX_MHA_DRIVERGROUP_F16_H
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "Ifx_MHA_BridgeDrv_F16_TLE987.h"
#include "Ifx_MHA_DriverGroup_F16_Cfg.h"
#include "Ifx_MHA_MeasurementADC_F16_TLE987.h"
#include "Ifx_MHA_PatternGen_F16_TLE987.h"
#include "Ifx_MHA_PatternGen_F16_TLE987_Cfg.h"
#include "Ifx_Math.h"
#include "stdbool.h"
#include "stdint.h"

/**
 * Contains the output variables of this component.
 */
typedef struct Ifx_MHA_DriverGroup_F16_Output
{
    /**
     * Three phase currents
     */
    Ifx_Math_3PhaseFract16 currentsUVW;

    /**
     * Shunt current measurements, scaled by the base current, represented in Q15
     */
    Ifx_Math_Fract16 shuntCurrentsQ15[2];

    /**
     * DC Link voltage, scaled by the base voltage, represented in Q15
     */
    Ifx_Math_Fract16 dcLinkVoltageQ15;
} Ifx_MHA_DriverGroup_F16_Output;

/**
 * \brief Data structure that stores all data of module instance.
 */
typedef struct Ifx_MHA_DriverGroup_F16
{
    /**
     * Instance of bridge driver for TLE987 devices
     */
    Ifx_MHA_BridgeDrv_F16_TLE987 bridgeDrv;

    /**
     * Instance of measurement ADC for TLE987 devices
     */
    Ifx_MHA_MeasurementADC_F16_TLE987 measurementADC;

    /**
     * Instance of pattern generator for TLE987 devices
     */
    Ifx_MHA_PatternGen_F16_TLE987 patternGen;

    /**
     * Output variables of the component
     */
    Ifx_MHA_DriverGroup_F16_Output p_output;

    /**
     * Counter for the current control loop execution
     */
    uint8_t currentControlCounter;
} Ifx_MHA_DriverGroup_F16;

/**
 *  \brief Clear fault status of all the underlying components on TLE987.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Indicates whether the "ClearFault" request is propagated to all underlying
 * components.
 */
static inline bool Ifx_MHA_DriverGroup_F16_clearFaults(Ifx_MHA_DriverGroup_F16* self)
{
    /* Flag whether the clear fault request has been propagated to all underlying components */
    bool clearFaultPropagated = true;

    /* Clear faults of BridgeDrv */
    Ifx_MHA_BridgeDrv_F16_TLE987_clearFault(&self->bridgeDrv);

    /* Clear faults of PatternGen */
    Ifx_MHA_PatternGen_F16_TLE987_clearFault(&self->patternGen);

    return clearFaultPropagated;
}


/**
 *  \brief Enable / disable all the underlying components on TLE987.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] enable Flag for required operation
 */
void Ifx_MHA_DriverGroup_F16_enableModules(Ifx_MHA_DriverGroup_F16* self, bool enable);

/**
 *  \brief Returns the component ID.
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 */
void Ifx_MHA_DriverGroup_F16_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Initializes all the underlying components on TLE987.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
void Ifx_MHA_DriverGroup_F16_initModules(Ifx_MHA_DriverGroup_F16* self);

/**
 *  \brief Checks if all the underlying components are in on state on TLE987.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Indicates whether all of the underlying components are in on state.
 */
static inline bool Ifx_MHA_DriverGroup_F16_checkStateOn(Ifx_MHA_DriverGroup_F16* self)
{
    /* Variable declaration */
    Ifx_MHA_MeasurementADC_F16_TLE987_Status measurementADCStatus;
    Ifx_MHA_BridgeDrv_F16_TLE987_Status      bridgeDrvStatus;
    Ifx_MHA_PatternGen_F16_TLE987_Status     patternGenStatus;
    bool                                     returnValue;

    /* Get the status of all components */
    measurementADCStatus = Ifx_MHA_MeasurementADC_F16_TLE987_getStatus(&self->measurementADC);
    bridgeDrvStatus      = Ifx_MHA_BridgeDrv_F16_TLE987_getStatus(&self->bridgeDrv);
    patternGenStatus     = Ifx_MHA_PatternGen_F16_TLE987_getStatus(&self->patternGen);

    /* Check if all components are in the on state */
    if ((measurementADCStatus.state == Ifx_MHA_MeasurementADC_F16_TLE987_State_on)
        && (bridgeDrvStatus.state == Ifx_MHA_BridgeDrv_F16_TLE987_State_on)
        && (patternGenStatus.state == Ifx_MHA_PatternGen_F16_TLE987_State_on))
    {
        returnValue = true;
    }
    else
    {
        returnValue = false;
    }

    return returnValue;
}


/**
 *  \brief Returns the component version.
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 */
void Ifx_MHA_DriverGroup_F16_getVersion(const Ifx_ComponentVersion** componentVersion);

/**
 *  \brief Checks if all the underlying components are in off state on TLE987.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Indicates whether all of the underlying components are in off state.
 */
static inline bool Ifx_MHA_DriverGroup_F16_checkStateOff(Ifx_MHA_DriverGroup_F16* self)
{
    /* Variable declaration */
    Ifx_MHA_MeasurementADC_F16_TLE987_Status measurementADCStatus;
    Ifx_MHA_BridgeDrv_F16_TLE987_Status      bridgeDrvStatus;
    Ifx_MHA_PatternGen_F16_TLE987_Status     patternGenStatus;
    bool                                     returnValue;

    /* Get the status of all components */
    measurementADCStatus = Ifx_MHA_MeasurementADC_F16_TLE987_getStatus(&self->measurementADC);
    bridgeDrvStatus      = Ifx_MHA_BridgeDrv_F16_TLE987_getStatus(&self->bridgeDrv);
    patternGenStatus     = Ifx_MHA_PatternGen_F16_TLE987_getStatus(&self->patternGen);

    /* Check if all components are in the off state */
    if ((measurementADCStatus.state == Ifx_MHA_MeasurementADC_F16_TLE987_State_off)
        && (bridgeDrvStatus.state == Ifx_MHA_BridgeDrv_F16_TLE987_State_off)
        && (patternGenStatus.state == Ifx_MHA_PatternGen_F16_TLE987_State_off))
    {
        returnValue = true;
    }
    else
    {
        returnValue = false;
    }

    return returnValue;
}


/**
 *  \brief Fault check for all the underlying components on TLE987.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Indicates whether any of the underlying components is in fault state.
 */
bool Ifx_MHA_DriverGroup_F16_checkFaults(Ifx_MHA_DriverGroup_F16* self);

/**
 *  \brief Checks if any of the underlying components are in FaultIsPending state on
 *  TLE987.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Indicates whether any of the underlying components is in ClearFaultIsPending
 * state.
 */
static inline bool Ifx_MHA_DriverGroup_F16_anyClearFaultIsPending(Ifx_MHA_DriverGroup_F16* self)
{
    /* Holds the overall result of the underlying components clearFaultIsPending */
    bool clearFaultIsPending = false;

    /* MHA components fault is pending */
    if (Ifx_MHA_BridgeDrv_F16_TLE987_clearFaultIsPending(&(self->bridgeDrv)) == true)
    {
        clearFaultIsPending = true;
    }
    else if (Ifx_MHA_PatternGen_F16_TLE987_clearFaultIsPending(&(self->patternGen)) == true)
    {
        clearFaultIsPending = true;
    }
    else
    {
        /* Do nothing */
    }

    /* Return whether clear fault of any underlying component is still pending */
    return clearFaultIsPending;
}


/**
 *  \brief Set function for comparator values on TLE987.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] compareValues_tick Compare values for the switches: first three up count, second three down count
 *
 *  \param [in] triggerTime_tick Array of first and second time trigger, in clock ticks
 */
static inline void Ifx_MHA_DriverGroup_F16_setCompVal(Ifx_MHA_DriverGroup_F16* self, uint16_t* compareValues_tick,
                                                      uint16_t* triggerTime_tick)
{
    Ifx_MHA_PatternGen_F16_TLE987_execute(&(self->patternGen), compareValues_tick, triggerTime_tick);
}


/**
 *  \brief Execute the BridgeDriver cyclic functions to detect and react to potential
 *  hardware faults.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MHA_DriverGroup_F16_executeBridgeDriver(Ifx_MHA_DriverGroup_F16* self)
{
    /* Execute state machine */
    Ifx_MHA_BridgeDrv_F16_TLE987_execute(&(self->bridgeDrv));
}


/**
 *  \brief Execute the ADC state machine. The phase currents and DC Link voltage measured
 *  in the last cycle are retrieved from the ADC and then the 3 phase currents are
 *  reconstructed.
 *
 *  Note 1: Must be called in a fixed cycle after the ADC measurements are finished
 *  (begin of current control loop).
 *  Note 2: It must be ensured that the reconstruction info fits the current
 *  measurement point. The reconstruction information of the last current control
 *  cycle must be provided.
 *  Note 3: ADC state machine must be in state on for valid measurements. See
 *  Ifx_MHA_MeasurementADC_F16_[platform]_execute for details.
 *
 *  Inputs and outputs of this API with [valid ranges]:
 *  <ul>
 *  <li>[inout] .measurementADC Reference to measurement ADC instance [!= null]</li>
 *  <li>[inout] .measurementADC.p_status.state [== State_on]</li>
 *  <li>[out] .p_output.currentsUVW Reconstructed three phase currents, when ADC is
 *  on</li>
 *  <li>[out] .p_output.dcLinkVoltageQ15 Measured DC Link voltage, when ADC is
 *  on</li>
 *  <li>[out] .p_output.shuntCurrentsQ15 Measured shunt currents, when ADC is
 *  on</li>
 *  </ul>
 *
 *  Note 4: This implementation variant is for TLE987x with single shunt topology.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] currentReconstructionInfo Current reconstruction info
 */
void Ifx_MHA_DriverGroup_F16_measureAndReconstruct(Ifx_MHA_DriverGroup_F16* self, Ifx_Math_CurrentReconstruction_info
                                                   currentReconstructionInfo);

/**
 *  \brief Callback function to get ADC measurements and update PWM pattern.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MHA_DriverGroup_F16_onPeriodMatchCallback(Ifx_MHA_DriverGroup_F16* self)
{
    /* Call onPeriodMatch callback to get ADC measurements */
    Ifx_MHA_MeasurementADC_F16_TLE987_periodMatch(&(self->measurementADC));

    /* Call onPeriodMatch callback to update PWM pattern */
    Ifx_MHA_PatternGen_F16_TLE987_onPeriodMatch(&(self->patternGen));
}


/**
 *  \brief Get the output variables of the component.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [out] output Structure containing the output of the component
 */
static inline void Ifx_MHA_DriverGroup_F16_getOutput(Ifx_MHA_DriverGroup_F16       * self,
                                                     Ifx_MHA_DriverGroup_F16_Output* output)
{
    *output = self->p_output;
}


/**
 *  \brief Current control callback to trigger new period.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Returns notification to trigger fast control-loop.
 */
static inline bool Ifx_MHA_DriverGroup_F16_onOneMatchCallback(Ifx_MHA_DriverGroup_F16* self)
{
#if (IFX_MHA_DRIVERGROUP_F16_CFG_CURRENT_LOOP_FACTOR > 1)
    bool retVal = false;
#else
    bool retVal = true;
#endif

    /* Update compare values */
    Ifx_MHA_PatternGen_F16_TLE987_onOneMatch(&(self->patternGen));

#if (IFX_MHA_DRIVERGROUP_F16_CFG_CURRENT_LOOP_FACTOR > 1)

    /* Execute current control only in the defined cycles */
    if (self->currentControlCounter < (uint8_t)(IFX_MHA_DRIVERGROUP_F16_CFG_CURRENT_LOOP_FACTOR - 1))
    {
        /* Increment counter */
        self->currentControlCounter++;
    }
    else
    {
        /* Update measurement values */
        Ifx_MHA_MeasurementADC_F16_TLE987_oneMatch(&(self->measurementADC));

        /* Reset Pattern Generator cycle counter */
        Ifx_MHA_PatternGen_F16_TLE987_reset(&(self->patternGen));

        /* Reset counter and set the notification flag */
        self->currentControlCounter = 0U;
        retVal                      = true;
    }

#else

    /* Update compare values */
    Ifx_MHA_MeasurementADC_F16_TLE987_oneMatch(&(self->measurementADC));
#endif

    return retVal;
}


#endif /*IFX_MHA_DRIVERGROUP_F16_H*/
