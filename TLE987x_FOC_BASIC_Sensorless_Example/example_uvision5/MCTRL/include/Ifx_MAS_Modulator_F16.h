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
 * \file Ifx_MAS_Modulator_F16.h
 * \brief This module gives the declaration of the functions used to implement a modulator.
 */

#ifndef IFX_MAS_MODULATOR_F16_H
#define IFX_MAS_MODULATOR_F16_H
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "Ifx_MAS_Modulator_F16_Cfg.h"
#include "Ifx_Math.h"
#include "stdbool.h"

/**
 * Macro for fault reaction when fault source is disabled
 */
#define IFX_MAS_MODULATOR_F16_FAULT_REACTION_DISABLE                (0)

/**
 * Macro for fault reaction when fault source is enabled
 */
#define IFX_MAS_MODULATOR_F16_FAULT_REACTION_ENABLE                 (1)

/**
 * Macro for fault reaction when fault source is report only
 */
#define IFX_MAS_MODULATOR_F16_FAULT_REACTION_REPORT_ONLY            (2)

/**
 * Macro for fault reaction when fault source is report and react
 */
#define IFX_MAS_MODULATOR_F16_FAULT_REACTION_REPORT_REACT           (3)

/**
 * Macro for active short low output behavior (0% duty cycle to generate zero vector[0,0,0]) in case of a fault or
 * enabled brake
 */
#define IFX_MAS_MODULATOR_F16_OUTPUT_BEHAVIOR_ACTIVE_SHORT_LOW      (0)

/**
 * Macro for active short high output behavior (100% duty cycle to generate zero vector[1,1,1]) in case of a fault or
 * enabled brake
 */
#define IFX_MAS_MODULATOR_F16_OUTPUT_BEHAVIOR_ACTIVE_SHORT_HIGH     (1)

/**
 * Macro for active short high low output behavior (50% duty cycle to generate both zero vectors [0,0,0] and [1,1,1]) in
 * case of a fault or enabled brake
 */
#define IFX_MAS_MODULATOR_F16_OUTPUT_BEHAVIOR_ACTIVE_SHORT_HIGH_LOW (2)

/**
 * Configuration for the current measurement
 */
typedef struct Ifx_MAS_Modulator_F16_currentMeasurementConfig
{
    /**
     * Delta to be added if measuring from the beginning
     */
    int16_t p_deltaBegin_tick;

    /**
     * Delta to be subtracted if measuring from the end
     */
    int16_t p_deltaEnd_tick;
} Ifx_MAS_Modulator_F16_currentMeasurementConfig;

/**
 * Contains the module output variables.
 */
typedef struct Ifx_MAS_Modulator_F16_Output
{
    /**
     * Compare values for the switches: first three up count, second three down count
     */
    uint16_t compareValues_tick[6];

    /**
     * Array of  trigger events
     */
    uint16_t triggerTime_tick[2];

    /**
     * The actual applied voltage vector
     */
    Ifx_Math_PolarFract16 actualVoltage;

    /**
     * Structure to have all different input variables for current reconstruction
     */
    Ifx_Math_CurrentReconstruction_info currentReconstructionInfo;
} Ifx_MAS_Modulator_F16_Output;

/**
 * Point of triggering for current measurement
 */
typedef enum Ifx_MAS_Modulator_F16_measurementPoint
{
    Ifx_MAS_Modulator_F16_measurementPoint_beginning = 0, /**<Beginning of the pulse*/
    Ifx_MAS_Modulator_F16_measurementPoint_end       = 1, /**<End of the pulse*/
} Ifx_MAS_Modulator_F16_measurementPoint;

/**
 * States of the modulator state machine
 */
typedef enum Ifx_MAS_Modulator_F16_State
{
    Ifx_MAS_Modulator_F16_State_init  = 0, /**<Modulator is initialized, compare values are set to zero and the period
                                            * parameter is initialized with the system value*/
    Ifx_MAS_Modulator_F16_State_off   = 1, /**<Modulator is off, compare values are set to zero*/
    Ifx_MAS_Modulator_F16_State_on    = 2, /**<Modulator is on and is calculating the compare values based on the input
                                            * voltage reference and the measured DC link voltage*/
    Ifx_MAS_Modulator_F16_State_fault = 3, /**<Modulator is in fault state and calculates the outputs according to the
                                            * configured fault output behavior.*/
    Ifx_MAS_Modulator_F16_State_brake = 4, /**<Modulator in brake state.*/
} Ifx_MAS_Modulator_F16_State;

/**
 * Parameter structure containing configuration of the component
 */
typedef struct Ifx_MAS_Modulator_F16_Param
{
    /**
     * Time needed to measure the current, in number of ticks
     */
    int16_t measurementTime_tick;

    /**
     * Deadtime, in number of ticks
     */
    int16_t deadTime_tick;

    /**
     * Driver delay, in number of ticks
     */
    int16_t driverDelay_tick;

    /**
     * Ringing time, in number of ticks
     */
    int16_t ringingTime_tick;

    /**
     * The period of the modulator, in number of ticks
     */
    int16_t period_tick;

    /**
     * Maximum allowed amplitude input, normalized by the base voltage, represented in Q15
     */
    Ifx_Math_Fract16 maxAmplitudeQ15;

    /**
     * Hysteresis band upper limit for switching between bi-directional two phase and bi-directional three phase
     */
    Ifx_Math_Fract16 biDirectionalShiftingThresholdHighQ15;

    /**
     * Hysteresis band lower limit for switching between bi-directional two phase and bi-directional three phase
     */
    Ifx_Math_Fract16 biDirectionalShiftingThresholdLowQ15;

    /**
     * Point of triggering
     */
    Ifx_MAS_Modulator_F16_measurementPoint measurementPoint;
} Ifx_MAS_Modulator_F16_Param;

/**
 * States of the modulator substate machine
 */
typedef enum Ifx_MAS_Modulator_F16_SubState
{
    Ifx_MAS_Modulator_F16_SubState_bidirectionalTwoPhase   = 0, /**<Substate machine is in bidirectional two phase
                                                                 * shifting*/
    Ifx_MAS_Modulator_F16_SubState_bidirectionalThreePhase = 1, /**<Substate machine is in bidirectional three phase
                                                                 * shifting*/
} Ifx_MAS_Modulator_F16_SubState;

/**
 * Status of the modulator, containing the bit coded errors and the state machine state.
 */
typedef struct Ifx_MAS_Modulator_F16_Status
{
    /**
     * State of the modulator
     */
    Ifx_MAS_Modulator_F16_State state;

    /**
     * State of the substate machine
     */
    Ifx_MAS_Modulator_F16_SubState subState;

    /**
     * Maximum amplitude status flag
     */
    bool maxAmplitudeFlag;

    /**
     * Overmodulation status flag
     */
    bool overmodulationFlag;
} Ifx_MAS_Modulator_F16_Status;

/**
 * \brief Data structure that stores all data of module instance.
 */
typedef struct Ifx_MAS_Modulator_F16
{
    /**
     * Parameter structure
     */
    Ifx_MAS_Modulator_F16_Param* param;

    /**
     * Configuration for current measurement
     */
    Ifx_MAS_Modulator_F16_currentMeasurementConfig p_currentMeasurement;

    /**
     * Status of the modulator
     */
    Ifx_MAS_Modulator_F16_Status p_status;

    /**
     * Contains the module output variables
     */
    Ifx_MAS_Modulator_F16_Output p_output;

    /**
     * The period of the modulator, in number of ticks (copy of the dynamic parameter for improved execution time)
     */
    int16_t period_tick;

    /**
     * Minimum time needed in order to measure the single shunt current, in number of ticks
     */
    int16_t p_minSenseTime_tick;

    /**
     * Limits the minimum turn on time for the phases, in number of ticks / 2
     */
    int16_t p_minTurnOnTimeHalf_tick;

    /**
     * State machine fault clear setting
     */
    bool p_clearFault;

    /**
     * State machine enable setting
     */
    bool p_enable;

    /**
     * Enable setting for brake state of modulator
     */
    bool p_enableBrake;
} Ifx_MAS_Modulator_F16;

/**
 * Lookup table to store the values needed to calculate the switching times.
 */
extern const Ifx_Math_Fract16 Ifx_MAS_Modulator_F16_lutSin60Sqrt3[];

/**
 * Global variable which holds default parameter values
 */
extern Ifx_MAS_Modulator_F16_Param Ifx_MAS_Modulator_F16_g_defaultParam;

/**
 *  \brief Recalculate private variable p_currentMeasurement.p_deltaBegin_tick based on
 *  other private variables.
 *
 *  This function also limits it's result to the specified datatype.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MAS_Modulator_F16_p_recalculateDeltaBegin(Ifx_MAS_Modulator_F16* self)
{
    int32_t tempSum = ((int32_t)self->param->ringingTime_tick + (int32_t)self->param->driverDelay_tick +
                       (int32_t)self->param->deadTime_tick);

    /* limit to datatype ranges to prevent overflows */
    if (tempSum <= (int32_t)INT16_MAX)
    {
        /* limiting value < 0 is handled by the setter of the summands */
        self->p_currentMeasurement.p_deltaBegin_tick = (int16_t)tempSum;
    }
    else
    {
        self->p_currentMeasurement.p_deltaBegin_tick = INT16_MAX;
    }
}


/**
 *  \brief Recalculate private variable p_currentMeasurement.p_deltaEnd_tick based on other
 *  private variables.
 *
 *  This function also limits it's result to the specified datatype.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MAS_Modulator_F16_p_recalculateDeltaEnd(Ifx_MAS_Modulator_F16* self)
{
    int32_t tempSum = ((int32_t)self->param->measurementTime_tick - (int32_t)self->param->driverDelay_tick);

    /* limit to datatype ranges to prevent overflows */
    if (tempSum >= 0)
    {
        /* limiting values > INT16_MAX is handled by the setter of the summands */
        self->p_currentMeasurement.p_deltaEnd_tick = (int16_t)tempSum;
    }
    else
    {
        self->p_currentMeasurement.p_deltaEnd_tick = 0;
    }
}


/**
 *  \brief Recalculate private variable p_minSenseTime_tick based on other private
 *  variables.
 *
 *  This function also limits it's result to the specified datatype.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MAS_Modulator_F16_p_recalculateMinSenseTime(Ifx_MAS_Modulator_F16* self)
{
    int32_t tempSum = ((int32_t)self->param->ringingTime_tick + (int32_t)self->param->measurementTime_tick +
                       (int32_t)self->param->deadTime_tick);

    /* limit to datatype ranges to prevent overflows */
    if (tempSum <= (int32_t)INT16_MAX)
    {
        /* limiting value < 0 is handled by the setter of the summands */
        self->p_minSenseTime_tick = (int16_t)tempSum;
    }
    else
    {
        self->p_minSenseTime_tick = INT16_MAX;
    }
}


/**
 *  \brief Initializes the modulator module.
 *
 *  Initialize the state to INIT, the subState to bidirectional two phase shifting,
 *  and other private variables to zero or to their configured values. Set the
 *  compare values to zero. Initialize the first current trigger to "2" and the
 *  second trigger to the middle of the PWM period to ensure that current
 *  measurement can be triggered in every state.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [inout] param Pointer to parameter structure containing configuration of the component
 */
void Ifx_MAS_Modulator_F16_init(Ifx_MAS_Modulator_F16* self, Ifx_MAS_Modulator_F16_Param* param);

/**
 *  \brief Get the status of the modulator, containing the state machine state and the bit
 *  coded errors.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Modulator status
 */
static inline Ifx_MAS_Modulator_F16_Status Ifx_MAS_Modulator_F16_getStatus(Ifx_MAS_Modulator_F16* self)
{
    return self->p_status;
}


/**
 *  \brief Enable or disable the brake state of state machine of the modulator based on the
 *  input parameter.
 *
 *  If the input parameter is true, the brake state will be enabled in the next call
 *  to execute.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] enable
 */
static inline void Ifx_MAS_Modulator_F16_enableBrake(Ifx_MAS_Modulator_F16* self, bool enable)
{
    self->p_enableBrake = enable;
}


/**
 *  \brief Get the modulator maximum amplitude value.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Maximum allowed amplitude input, normalized by the base voltage, represented in
 * Q15
 */
static inline Ifx_Math_Fract16 Ifx_MAS_Modulator_F16_getMaxAmplitude(Ifx_MAS_Modulator_F16* self)
{
    return self->param->maxAmplitudeQ15;
}


/**
 *  \brief Get the modulator deadtime
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Modulator deadtime, in number of ticks
 */
static inline uint16_t Ifx_MAS_Modulator_F16_getDeadTime_tick(Ifx_MAS_Modulator_F16* self)
{
    return (int16_t)self->param->deadTime_tick;
}


/**
 *  \brief Get the modulator period, which is the number of available ticks in one PWM
 *  period.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Period, in number of ticks
 */
static inline uint16_t Ifx_MAS_Modulator_F16_getPeriod_tick(Ifx_MAS_Modulator_F16* self)
{
    return (uint16_t)self->param->period_tick;
}


/**
 *  \brief Calculate the switching times of the Space Vector Modulation.
 *
 *  This API implements the various states of the modulator state machine:
 *  <ul>
 *  <li>init: init internal variables</li>
 *  <li>off: set compare values to 0% duty cycle and trigger times to default</li>
 *  <li>fault: set compare values according to fault output behavior and trigger
 *  times to default</li>
 *  <li>brake: set compare values according to brake output behavior and trigger
 *  times to default</li>
 *  <li>on: calculate compare values and current triggers according to the following
 *  steps:</li>
 *  </ul>
 *  1. calculate the symmetric switching pattern
 *  2. eliminate the shortest pulse in case it is shorter than the configured
 *  minimum on time limitation and compensate other phases (only eliminate if after
 *  compensation the middle pulse is still large enough for current measurement)
 *  3. change to asymmetric switching if the current measurement window is not
 *  sufficient
 *  4. limit the compare values to prevent overflow
 *  5. calculate the current triggers
 *
 *  For the default trigger times initialize the first current trigger to "2" and
 *  the second trigger to the middle of the PWM period to ensure that current
 *  measurement can be triggered in every state.
 *
 *  Inputs for the calculation:
 *  <ul>
 *  <li>SVM Angle (α): the "angle" member of the data structure of type
 *  Ifx_Modulator_F16_Polar</li>
 *  <li>SVM Amplitude (Uc): the "amplitude" member of the data structure of type
 *  Ifx_Modulator_F16_Polar, normalized by IFX_MODULATOR_F16_BASE_VOLTAGE_V in
 *  Q15</li>
 *  <li>SVM DC-link Voltage (Udc): the "dcLinkVoltage" parameter of this API,
 *  normalized by IFX_MODULATOR_F16_BASE_VOLTAGE_V in Q15</li>
 *  </ul>
 *
 *  Outputs of the calculation:
 *  <ul>
 *  <li>SVM switching times of the three channels</li>
 *  <li>Trigger Times for current measurement</li>
 *  <li>Sector information and whether the second current trigger measures the sum
 *  of two currents</li>
 *  <li>Applied voltage vector, normalized by IFX_MODULATOR_F16_BASE_VOLTAGE_V in
 *  Q15</li>
 *  </ul>
 *  <ul>
 *  <li>Three phase currents, Ia, Ib, and Ic, normalized by
 *  IFX_MODULATOR_F16_BASE_CURRENT_A in Q15</li>
 *  </ul>
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] refVoltage Reference input voltage to the modulator
 *
 *  \param [in] dcLinkVoltageQ15 The actual DC link voltage value.
 */
void Ifx_MAS_Modulator_F16_execute(Ifx_MAS_Modulator_F16* self, Ifx_Math_PolarFract16 refVoltage, Ifx_Math_Fract16
                                   dcLinkVoltageQ15);

/**
 *  \brief Clears all faults from the module.
 *
 *  This API clears all faults from the module. The module will be set to enable or
 *  disable state in the next call to execute() depending on the enable setting.
 *  New hardware fault will lead the module to fault state again.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MAS_Modulator_F16_clearFault(Ifx_MAS_Modulator_F16* self)
{
    self->p_clearFault = true;
}


/**
 *  \brief This API enables or disables the module based on the input parameter enable.
 *
 *  If the input parameter is TRUE and no fault is detected, then the module will be
 *  enabled in the next call to execute().
 *  This API also set the HW-related members of the internal "self" data structure
 *  to the corresponding HW registers according to the user manual.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] enable Paramter of boolean type to enable or disable the module
 */
static inline void Ifx_MAS_Modulator_F16_enable(Ifx_MAS_Modulator_F16* self, bool enable)
{
    self->p_enable = enable;
}


/**
 *  \brief Returns the component ID
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 */
void Ifx_MAS_Modulator_F16_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Get the module output variables.
 *
 *  The output contains:
 *  - Information for the current reconstruction
 *  - 6 compare values in ticks, representing the pattern to be generated
 *  - 2 trigger times in ticks, to be used in the current measurement
 *  - The actual voltage requested to the modulator. This is the voltage after the
 *  limitations applied to the voltage vector: maximum amplitude and overmodulation.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [out] output Structure containing the module output
 */
static inline void Ifx_MAS_Modulator_F16_getOutput(Ifx_MAS_Modulator_F16* self, Ifx_MAS_Modulator_F16_Output* output)
{
    *output = self->p_output;
}


/**
 *  \brief Returns the component version
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 */
void Ifx_MAS_Modulator_F16_getVersion(const Ifx_ComponentVersion** componentVersion);

/**
 *  \brief Check if a clear fault request is pending.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return True if a clear fault request was made but still not processed by the module and
 * False otherwise
 */
static inline bool Ifx_MAS_Modulator_F16_clearFaultIsPending(Ifx_MAS_Modulator_F16* self)
{
    return self->p_clearFault;
}


/**
 *  \brief Set the modulator deadtime
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] deadTime_tick Deadtime to be set, in number of ticks
 */
static inline void Ifx_MAS_Modulator_F16_setDeadTime_tick(Ifx_MAS_Modulator_F16* self, uint16_t deadTime_tick)
{
    /* Set internal deadtime variable in ticks */
    self->param->deadTime_tick = (int16_t)deadTime_tick;

    /* Set internal variable for minimum on time */
    self->p_minTurnOnTimeHalf_tick = (IFX_MAS_MODULATOR_F16_CFG_MIN_ON_TIME_TICK + (int16_t)deadTime_tick) / 2;

    /* Recalculate internal variables for current measurement */
    Ifx_MAS_Modulator_F16_p_recalculateMinSenseTime(self);
    Ifx_MAS_Modulator_F16_p_recalculateDeltaBegin(self);
}


/**
 *  \brief Set the modulator driver delay
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] driverDelay_tick Driver delay to be set, in number of ticks
 */
static inline void Ifx_MAS_Modulator_F16_setDriverDelay_tick(Ifx_MAS_Modulator_F16* self, uint16_t driverDelay_tick)
{
    self->param->driverDelay_tick = (int16_t)driverDelay_tick;
    Ifx_MAS_Modulator_F16_p_recalculateDeltaBegin(self);
    Ifx_MAS_Modulator_F16_p_recalculateDeltaEnd(self);
}


/**
 *  \brief Set the modulator maximum amplitude value.
 *
 *  This amplitude will limit the maximum amplitude input for the modulator.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] maxAmplitudeQ15 Maximum allowed amplitude input, normalized by the base voltage, represented in
 * Q15
 */
static inline void Ifx_MAS_Modulator_F16_setMaxAmplitude(Ifx_MAS_Modulator_F16* self, Ifx_Math_Fract16
                                                         maxAmplitudeQ15)
{
    self->param->maxAmplitudeQ15 = maxAmplitudeQ15;
}


/**
 *  \brief Get the modulator driver delay
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Modulator driver delay, in number of ticks
 */
static inline uint16_t Ifx_MAS_Modulator_F16_getDriverDelay_tick(Ifx_MAS_Modulator_F16* self)
{
    return (uint16_t)self->param->driverDelay_tick;
}


/**
 *  \brief Set the modulator measurement time
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] measurementTime_tick Measurement time to be set, in number of ticks
 */
static inline void Ifx_MAS_Modulator_F16_setMeasurementTime_tick(Ifx_MAS_Modulator_F16* self, uint16_t
                                                                 measurementTime_tick)
{
    self->param->measurementTime_tick = (int16_t)measurementTime_tick;
    Ifx_MAS_Modulator_F16_p_recalculateMinSenseTime(self);
    Ifx_MAS_Modulator_F16_p_recalculateDeltaEnd(self);
}


/**
 *  \brief Get the modulator measurement time
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Modulator measurement time, in number of ticks
 */
static inline uint16_t Ifx_MAS_Modulator_F16_getMeasurementTime_tick(Ifx_MAS_Modulator_F16* self)
{
    return (uint16_t)self->param->measurementTime_tick;
}


/**
 *  \brief Set the modulator period, which is the number of available ticks in one PWM
 *  period.
 *
 *  The period ticks are peripheral frequency/switching frequency.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] period_tick Period to be set, in number of ticks
 */
static inline void Ifx_MAS_Modulator_F16_setPeriod_tick(Ifx_MAS_Modulator_F16* self, uint16_t period_tick)
{
    self->param->period_tick = (int16_t)period_tick;
    self->period_tick        = (int16_t)period_tick;
}


/**
 *  \brief Get the modulator ringing time
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Modulator ringingtime, in number of ticks
 */
static inline uint16_t Ifx_MAS_Modulator_F16_getRingingTime_tick(Ifx_MAS_Modulator_F16* self)
{
    return (uint16_t)self->param->ringingTime_tick;
}


/**
 *  \brief Set the modulator ringing time
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] ringingTime_tick Ringing time to be set, in number of ticks
 */
static inline void Ifx_MAS_Modulator_F16_setRingingTime_tick(Ifx_MAS_Modulator_F16* self, uint16_t ringingTime_tick)
{
    self->param->ringingTime_tick = (int16_t)ringingTime_tick;
    Ifx_MAS_Modulator_F16_p_recalculateMinSenseTime(self);
    Ifx_MAS_Modulator_F16_p_recalculateDeltaBegin(self);
}


#endif /*IFX_MAS_MODULATOR_F16_H*/
