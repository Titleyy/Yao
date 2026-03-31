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
 * \file Ifx_MS_FocSolution_F16.h
 * \brief This module takes input from user, e.g. reference speed, and drives a motor according to the control mode
 * configuration, such as foc.
 */

#ifndef IFX_MS_FOCSOLUTION_F16_H
#define IFX_MS_FOCSOLUTION_F16_H
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "Ifx_MAS_Modulator_F16.h"
#include "Ifx_MDA_FluxEstimator_F16.h"
#include "Ifx_MDA_FocController_F16.h"
#include "Ifx_MDA_IToFController_F16.h"
#include "Ifx_MDA_StartAngleIdent_F16.h"
#include "Ifx_MDA_VToFController_F16.h"
#include "Ifx_MHA_DriverGroup_F16.h"
#include "Ifx_MS_FocSolution_F16_Cfg.h"
#include "Ifx_Math.h"
#include "Ifx_Math_AccelLimit_F16.h"
#include "Ifx_Math_Limit_F16.h"
#include "Ifx_Math_MulShRSat.h"
#include "Ifx_Math_Pi_F16.h"
#include "Ifx_Math_RateLimit_F16.h"
#include "Ifx_Math_SpeedPreControl_F16.h"

/**
 * Contains the module output variables.
 */
typedef struct Ifx_MS_FocSolution_F16_Output
{
    /**
     * Estimated speed by the flux estimator, represented in Q15 and normalized by the base speed
     */
    Ifx_Math_Fract16 estimatedSpeedQ15;
} Ifx_MS_FocSolution_F16_Output;

/**
 * Parameter structure containing configuration of the component
 */
typedef struct Ifx_MS_FocSolution_F16_Param
{
    /**
     * Angle error value to do the transition.
     */
    Ifx_Math_Fract32 transitionAngleTolerance;

    /**
     * Acceleration limit of speed ramp in open-loop represented in Q30
     */
    Ifx_Math_Fract32 speedRampUpRateOpenLoopQ30;

    /**
     * Deceleration limit of speed ramp in open-loop represented in Q30
     */
    Ifx_Math_Fract32 speedRampDownRateOpenLoopQ30;

    /**
     * Acceleration limit of speed ramp in closed-loop represented in Q30
     */
    Ifx_Math_Fract32 speedRampUpRateClosedLoopQ30;

    /**
     * Deceleration limit of speed ramp in closed-loop represented in Q30
     */
    Ifx_Math_Fract32 speedRampDownRateClosedLoopQ30;

    /**
     * Speed PI proportional gain
     */
    uint32_t speedPiPropGain;

    /**
     * Speed PI integral gain * sampling time
     */
    uint32_t speedPiIntegGainSamplingTime;

    /**
     * Speed PI antiwindup gain * sampling time
     */
    uint32_t speedPiAntiWindupGainSamplingTime;

    /**
     * Speed PreControl J/Ts static configuration
     */
    uint32_t rotorInertiaOverSamplingTime;

    /**
     * Time required to do the smooth transition, in number of execution cycles.
     */
    uint16_t transitionTimeLimit_cycles;

    /**
     * Reference imaginary current in open loop
     */
    Ifx_Math_Fract16 startUpCurrentQ15;

    /**
     * Initial reference startup current
     */
    Ifx_Math_Fract16 initStartupCurrentQ15;

    /**
     * Ratio between the target D current for smooth transition and the startup current
     */
    Ifx_Math_Fract16 transitionDownDCurrentScalingQ14;

    /**
     * Startup current ramp up rate
     */
    Ifx_Math_Fract16 startUpCurrentRampUpRateQ15;

    /**
     * Quadrature current that will be applied when transitioning from open to closed loop if Direct Transition is
     * selected
     */
    Ifx_Math_Fract16 qCurrentAtTransitionQ15;

    /**
     * Minimum speed below which state machine goes into standby (freewheeling)
     */
    Ifx_Math_Fract16 minimumSpeedThresholdQ15;

    /**
     * Transition speed for going from open to close loop
     */
    Ifx_Math_Fract16 transitionSpeedUpQ15;

    /**
     * Transition speed band
     */
    Ifx_Math_Fract16 transitionSpeedBandQ15;

    /**
     * Reference current upper limit
     */
    Ifx_Math_Fract16 refCurrentUpperLimitQ15;

    /**
     * Reference current lower limit
     */
    Ifx_Math_Fract16 refCurrentLowerLimitQ15;

    /**
     * Reference speed upper limit
     */
    Ifx_Math_Fract16 refSpeedUpperLimitQ15;

    /**
     * Reference speed lower limit
     */
    Ifx_Math_Fract16 refSpeedLowerLimitQ15;

    /**
     * Friction constant for speed precontrol
     */
    Ifx_Math_Fract16 frictionConstant;

    /**
     * Inverse torque constant for speed precontrol
     */
    Ifx_Math_Fract16 inverseTorqueConstant;
} Ifx_MS_FocSolution_F16_Param;

/**
 * Composition of pointers to the parameter structures of the underlying components
 */
typedef struct Ifx_MS_FocSolution_F16_ParamComposition
{
    /**
     * Pointer to the parameter struct of the Modulator
     */
    Ifx_MAS_Modulator_F16_Param* modulatorParam;

    /**
     * Pointer to the parameter struct of the FluxEstimator
     */
    Ifx_MDA_FluxEstimator_F16_Param* fluxEstimatorParam;

    /**
     * Pointer to the parameter struct of the FocController
     */
    Ifx_MDA_FocController_F16_Param* focControllerParam;

    /**
     * Pointer to the parameter struct of the IToFController
     */
    Ifx_MDA_IToFController_F16_Param* iToFControllerParam;

    /**
     * Pointer to the parameter struct of the VToFController
     */
    Ifx_MDA_VToFController_F16_Param* vToFControllerParam;
} Ifx_MS_FocSolution_F16_ParamComposition;

/**
 * Control modes of the module
 */
typedef enum Ifx_MS_FocSolution_F16_ControlMode
{
    Ifx_MS_FocSolution_F16_ControlMode_vToF = 0, /**<Executes voltage-to-frequency control*/
    Ifx_MS_FocSolution_F16_ControlMode_foc  = 1, /**<Executes field oriented control*/
} Ifx_MS_FocSolution_F16_ControlMode;

/**
 * States of the FOC state machine
 */
typedef enum Ifx_MS_FocSolution_F16_State
{
    Ifx_MS_FocSolution_F16_State_init            = 0, /**<FOC is in init state*/
    Ifx_MS_FocSolution_F16_State_off             = 1, /**<FOC is in off state*/
    Ifx_MS_FocSolution_F16_State_standBy         = 2, /**<FOC is in stand by state*/
    Ifx_MS_FocSolution_F16_State_fault           = 3, /**<FOC is in fault state*/
    Ifx_MS_FocSolution_F16_State_run             = 4, /**<FOC is in run state*/
    Ifx_MS_FocSolution_F16_State_rampDown        = 5, /**<FOC is in ramp down state*/
    Ifx_MS_FocSolution_F16_State_startAngleIdent = 6, /**<FOC is in start angle identification state*/
} Ifx_MS_FocSolution_F16_State;

/**
 * States of the FOC substate machine
 */
typedef enum Ifx_MS_FocSolution_F16_SubState
{
    Ifx_MS_FocSolution_F16_SubState_openLoop       = 0, /**<Substate machine is open loop*/
    Ifx_MS_FocSolution_F16_SubState_transitionUp   = 1, /**<Substate machine is in transition up*/
    Ifx_MS_FocSolution_F16_SubState_closedLoop     = 2, /**<Substate machine is in closed loop*/
    Ifx_MS_FocSolution_F16_SubState_transitionDown = 3, /**<Substate machine is in transition down*/
} Ifx_MS_FocSolution_F16_SubState;

/**
 * Status of the FOC solution, containing the information about the control mode and state machine state
 */
typedef struct Ifx_MS_FocSolution_F16_Status
{
    /**
     * State of the FOC controller
     */
    Ifx_MS_FocSolution_F16_State state;

    /**
     * State of the substate machine
     */
    Ifx_MS_FocSolution_F16_SubState subState;

    /**
     * Status of the control mode
     */
    Ifx_MS_FocSolution_F16_ControlMode actualControlMode;
} Ifx_MS_FocSolution_F16_Status;

/**
 * \brief Data structure that stores all data of module instance.
 */
typedef struct Ifx_MS_FocSolution_F16
{
    /**
     * Parameter structure
     */
    Ifx_MS_FocSolution_F16_Param* param;

    /**
     * Instance of field oriented controller
     */
    Ifx_MDA_FocController_F16 focController;

    /**
     * Instance of modulator
     */
    Ifx_MAS_Modulator_F16 modulator;

    /**
     * Instance of driverGroup
     */
    Ifx_MHA_DriverGroup_F16 driverGroup;

    /**
     * Instance of flux estimator
     */
    Ifx_MDA_FluxEstimator_F16 fluxEstimator;

    /**
     * Instance of i2f
     */
    Ifx_MDA_IToFController_F16 iToF;

    /**
     * Instance of the speed precontrol module
     */
    Ifx_Math_SpeedPreControl_F16 speedPreControl;

    /**
     * Instance of a limiter that limits the sum of the speed controller and the speed precontrol used as current
     * reference for the foc controller.
     */
    Ifx_Math_Limit_F16 p_refCurrentLimit;

    /**
     * Status variable  for  FOC state
     */
    volatile Ifx_MS_FocSolution_F16_Status p_status;

    /**
     * Instance of the limiter module
     */
    Ifx_Math_Limit_F16 speedLimit;

    /**
     * Instance of the acceleration limiter module
     */
    Ifx_Math_AccelLimit_F16 accelerationLimit;

    /**
     * Instance of Pi for speed control
     */
    Ifx_Math_Pi_F16 speedPi;

    /**
     * Instance of the start angle identification module
     */
    Ifx_MDA_StartAngleIdent_F16 startAngleIdent;

    /**
     * Instance of VToF
     */
    Ifx_MDA_VToFController_F16 vToF;

    /**
     * Instance of a rate limiter that limits the startup current rate for the foc controller in iToF.
     */
    Ifx_Math_RateLimit_F16 p_startCurrentRateLimit;

    /**
     * Current in alpha-beta form
     */
    Ifx_Math_CmpFract16 currentsAlphaBeta;

    /**
     * Voltage in alpha-beta form
     */
    Ifx_Math_CmpFract16 voltageAlphaBeta;

    /**
     * Previous voltage in alpha-beta form
     */
    Ifx_Math_CmpFract16 previousVoltageAlphaBeta;

    /**
     * Command for the d-q current
     */
    Ifx_Math_CmpFract16 dqCommand;

    /**
     * FOC angle, coming from I2f in open loop and from the flux estimator in closed loop
     */
    uint32_t angle;

    /**
     * Transition counter, in number of cycles.
     */
    uint16_t p_transitionCounter_cycles;

    /**
     * Contains the module output variables
     */
    Ifx_MS_FocSolution_F16_Output p_output;

    /**
     * Normalized rate limited speed
     */
    volatile Ifx_Math_Fract16 rateLimitInSpeedQ15;

    /**
     * Mid point between transition speed up and transition speed down, which are relevant for going from closed loop to
     * open loop and vice versa
     */
    Ifx_Math_Fract16 transitionSpeedMidQ15;

    /**
     * Half the speed band
     */
    Ifx_Math_Fract16 p_transitionSpeedBandHalfQ15;

    /**
     * Transition speed for going close to open loop
     */
    Ifx_Math_Fract16 transitionSpeedDownQ15;

    /**
     * Previous reference Q current
     */
    Ifx_Math_Fract16 p_previousQCommand;

    /**
     * Target  to ramp up D current to for smooth transition down
     */
    Ifx_Math_Fract16 p_transitionDownTargetDCurrentQ15;

    /**
     * Internal variable which holds the slope value needed to ramp up the Q current and ramp down the D current in
     * smooth transition down
     */
    Ifx_Math_Fract16 p_transitionDeltaQ15;

    /**
     * sets the control mode either FOC control or VToF control
     */
    Ifx_MS_FocSolution_F16_ControlMode p_controlMode;

    /**
     * Current reconstruction variables
     */
    Ifx_Math_CurrentReconstruction_info p_currentReconstructionInfo;

    /**
     * Current reconstruction variables from the previous cycle
     */
    Ifx_Math_CurrentReconstruction_info previousCurrentReconstructionInfo;

    /**
     * Dynamic parameter of boolean type to enable or disable speed precontrol
     */
    bool p_enableSpeedPreControl;

    /**
     * Enables or disables the power stage
     */
    bool p_enablePowerStage;

    /**
     * Enables or disables the control
     */
    bool p_enableControl;

    /**
     * State machine fault clear setting
     */
    bool p_clearFault;

    /**
     * Enables or disables the direct interface
     */
    bool p_enableDirectInterface;

    /**
     * Dynamic parameter of boolean type to enable or disable the start angle identification
     */
    bool p_enableStartAngleIdent;

    /**
     * True if a clear fault was requested to the underlying modules
     */
    bool p_clearFaultIsRequested;

    /**
     * Flag to signal command Q current sign change
     */
    bool p_qCommandZeroCrossing;
} Ifx_MS_FocSolution_F16;

/**
 * Global variable which holds default parameter values
 */
extern Ifx_MS_FocSolution_F16_Param Ifx_MS_FocSolution_F16_g_defaultParam;

/**
 * Global variable which holds the default parameter structures composition
 */
extern Ifx_MS_FocSolution_F16_ParamComposition Ifx_MS_FocSolution_F16_g_defaultParamComposition;

/**
 *  \brief The initialization API of the field oriented control (FOC) module.
 *
 *  The normalized parameters are initialized according to the data entered in
 *  ConfigWizard.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [inout] param Pointer to the parameter structure containing the configuration of the component
 *
 *  \param [inout] paramComposition Pointer to the composition of pointers to the parameter structures of the
 * underlying components
 */
void Ifx_MS_FocSolution_F16_init(Ifx_MS_FocSolution_F16* self, Ifx_MS_FocSolution_F16_Param* param,
                                 Ifx_MS_FocSolution_F16_ParamComposition* paramComposition);

/**
 *  \brief Clears all faults from the module.
 *
 *  This API clears all faults from the module. The module will be set to off state
 *  in the next call to executeSpeedControl(). A new fault will lead the module to
 *  fault state again.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MS_FocSolution_F16_clearFault(Ifx_MS_FocSolution_F16* self)
{
    self->p_clearFault = true;
}


/**
 *  \brief Enables or disables the power stage based on the input parameter enable.
 *
 *  This API enables or disables the power stage based on the input parameter
 *  enable. If the input parameter is TRUE and no fault is detected, then the module
 *  will be set to standby in the next call to executeSpeedControl().
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] enable Parameter of boolean type to enable or disable the power stage
 */
static inline void Ifx_MS_FocSolution_F16_enablePowerStage(Ifx_MS_FocSolution_F16* self, bool enable)
{
    self->p_enablePowerStage = enable;
}


/**
 *  \brief Enables or disables the speed control based on the input parameter enable.
 *
 *  This API enables or disables the speed control based on the input parameter
 *  enable. If the input parameter is TRUE and no fault is detected, then the module
 *  will start the speed control in the next call to executeSpeedControl().
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] enable Parameter of boolean type to enable or disable the power stage
 */
static inline void Ifx_MS_FocSolution_F16_enableControl(Ifx_MS_FocSolution_F16* self, bool enable)
{
    self->p_enableControl = enable;
}


/**
 *  \brief Executes the control loop depending on the control mode.
 *
 *  This API executes the current control loop by performing these steps for FOC
 *  control mode:
 *  <ol>
 *  <li>Read the measured shunt currents and reconstruct the phase currents.</li>
 *  <li>Perform a Clarke transformation.</li>
 *  <li>Execute the flux estimator to estimate the rotor flux speed and angle.</li>
 *  <li>Switch between open and closed loop based on the state of the state
 *  machine.</li>
 *  <li>Execute the Field Oriented Control (FOC) algorithm to calculate the
 *  reference voltage vector.</li>
 *  <li>Execute the modulator to calculate the compare values and trigger times for
 *  shunt current measurement.</li>
 *  <li>Calls the Pattern Generator which controls the PWM hardware module.</li>
 *  <li>Converts the reference voltage vector from polar to cartesian
 *  coordinates.</li>
 *  </ol>
 *
 *  This API executes the control loop by performing these steps for VtoF control
 *  mode:
 *  1. Calculates the voltage vector amplitude
 *  1. Calculates the voltage vector angle
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
void Ifx_MS_FocSolution_F16_executeControlMode(Ifx_MS_FocSolution_F16* self);

/**
 *  \brief Executes the speed control loop.
 *
 *  This API executes the speed control loop by performing these steps:
 *  <ol>
 *  <li>Check for faults.</li>
 *  <li>Calculate the error between the input and measured speed.</li>
 *  <li>Execute the speed PI controller.</li>
 *  <li>Advance the state machine of the module according to the state machine
 *  diagram</li>
 *  </ol>
 *
 *  Inputs of this API:
 *  <ul>
 *  <li>Reference speed, normalized in Q15 format</li>
 *  <li>Reference D and Q currents as inputs, normalized in Q15 format (only valid
 *  if the direct current interface is enabled)</li>
 *  </ul>
 *
 *  Outputs of this API:
 *  <ul>
 *  <li>Reference d-q currents, normalized in Q15 format</li>
 *  <li>State of the state machine</li>
 *  </ul>
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] speedQ15 Input speed
 *
 *  \param [in] currentsDqRef Input dq reference currents
 */
void Ifx_MS_FocSolution_F16_executeSpeedControl(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 speedQ15,
                                                Ifx_Math_CmpFract16 currentsDqRef);

/**
 *  \brief Enables or disables the direct currents(DQ) command based on the input parameter
 *  enable.
 *
 *  This API enables or disables the direct currents(DQ) based on the input
 *  parameter enable. If the input parameter is TRUE currents will be set directly
 *  as per the user input.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] enable Parameter of boolean type to enable or disable the direct interface
 */
static inline void Ifx_MS_FocSolution_F16_enableDirectInterface(Ifx_MS_FocSolution_F16* self, bool enable)
{
    self->p_enableDirectInterface = enable;
}


/**
 *  \brief Get the module output variables, containing the estimated speed.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [out] output Structure containing the module ouput
 */
static inline void Ifx_MS_FocSolution_F16_getOutput(Ifx_MS_FocSolution_F16       * self,
                                                    Ifx_MS_FocSolution_F16_Output* output)
{
    *output = self->p_output;
}


/**
 *  \brief Get the status of the module, containing the state machine state.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Module status
 */
static inline Ifx_MS_FocSolution_F16_Status Ifx_MS_FocSolution_F16_getStatus(Ifx_MS_FocSolution_F16* self)
{
    return self->p_status;
}


/**
 *  \brief Returns the component ID
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 */
void Ifx_MS_FocSolution_F16_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Set the angle error (between I2f angle and flux estimator angle) required to do
 *  the transition from transition up to close loop state.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] transitionAngleTolerance Angle error to perform the transition, greater or equal to 0, normalized by
 * 2*pi, in signed 32-bit format
 */
static inline void Ifx_MS_FocSolution_F16_setTransitionAngleTolerance(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract32
                                                                      transitionAngleTolerance)
{
    self->param->transitionAngleTolerance = transitionAngleTolerance;
}


/**
 *  \brief Get the angle error value
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return angle error
 */
static inline Ifx_Math_Fract32 Ifx_MS_FocSolution_F16_getTransitionAngleTolerance(Ifx_MS_FocSolution_F16* self)
{
    return self->param->transitionAngleTolerance;
}


/**
 *  \brief Returns the component version
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 */
void Ifx_MS_FocSolution_F16_getVersion(const Ifx_ComponentVersion** componentVersion);

/**
 *  \brief Set the time limit to do the transition from transition up to close loop or from
 *  transition down to open loop state.
 *
 *  This API sets the time limit to do the transition from transition up to close
 *  loop or from transition down to open loop state. In addition, it sets the slope
 *  needed for ramping the currents during smooth transition down.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] transitionTimeLimit_cycles Time limit to do the transition, in number of execution cycles
 */
void Ifx_MS_FocSolution_F16_setTransitionTimeLimit(Ifx_MS_FocSolution_F16* self, uint16_t transitionTimeLimit_cycles);

/**
 *  \brief Get the transition time limit
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return transition time limit
 */
static inline uint16_t Ifx_MS_FocSolution_F16_getTransitionTimeLimit(Ifx_MS_FocSolution_F16* self)
{
    return self->param->transitionTimeLimit_cycles;
}


/**
 *  Enable start angle identification.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MS_FocSolution_F16_enableStartAngleIdent(Ifx_MS_FocSolution_F16* self)
{
    self->p_enableStartAngleIdent = true;
}


/**
 *  \brief Set the absolute value of the quadrature current that will be applied when
 *  transitioning from open to closed loop
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] qCurrentQ15 Quadrature current at transition, greater or equal to 0, normalized by the base
 * current, in Q15 format
 */
static inline void Ifx_MS_FocSolution_F16_setQCurrentAtTransition(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                  qCurrentQ15)
{
    self->param->qCurrentAtTransitionQ15 = qCurrentQ15;
}


/**
 *  \brief Get the absolute value of quadrature current that is applied when transitioning
 *  from open to closed loop.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Quadrature current value
 */
static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_getQCurrentAtTransition(Ifx_MS_FocSolution_F16* self)
{
    return self->param->qCurrentAtTransitionQ15;
}


/**
 *  \brief Enables or disables the speed precontrol based on the input parameter enable.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] enable Parameter of boolean type to enable or disable speed precontrol
 */
static inline void Ifx_MS_FocSolution_F16_enableSpeedPreControl(Ifx_MS_FocSolution_F16* self, bool enable)
{
    self->p_enableSpeedPreControl = enable;
}


/**
 *  \brief Set the control mode in which system will work
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] controlMode Parameter of enumeration type to set the control mode
 */
static inline void Ifx_MS_FocSolution_F16_setControlMode(Ifx_MS_FocSolution_F16           * self,
                                                         Ifx_MS_FocSolution_F16_ControlMode controlMode)
{
    self->p_controlMode = controlMode;
}


/**
 *  \brief Get the control mode
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline Ifx_MS_FocSolution_F16_ControlMode Ifx_MS_FocSolution_F16_getControlMode(Ifx_MS_FocSolution_F16* self)
{
    return self->p_controlMode;
}


/**
 *  \brief Check if a clear fault request is pending.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return True if a clear fault request was made but still not processed by the module and
 * False otherwise
 */
static inline bool Ifx_MS_FocSolution_F16_clearFaultIsPending(Ifx_MS_FocSolution_F16* self)
{
    return self->p_clearFault;
}


/**
 *  \brief Set the startup current and the target D current for smooth transition
 *
 *  This API sets the startup current and the transition down target D current as
 *  the product of the transition down D current scaling factor and the startup
 *  current. The target D current is the value to which the D current is ramped up
 *  to before entering the transition down state from the closed loop state.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] startUpCurrentQ15 startup current, greater or equal to 0, normalized by the base current, in Q15
 * format
 */
void Ifx_MS_FocSolution_F16_setStartUpCurrent(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 startUpCurrentQ15);

/**
 *  \brief Get the startup current.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return startup current
 */
static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_getStartUpCurrent(Ifx_MS_FocSolution_F16* self)
{
    return self->param->startUpCurrentQ15;
}


/**
 *  \brief Set the acceleration rate of the speed ramp in open-loop, represented in Q30.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] speedRampUpRateOpenLoopQ30 acceleration rate open-loop, greater or equal to zero, represented in Q30
 */
static inline void Ifx_MS_FocSolution_F16_setSpeedRampUpRateOpenLoop(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract32
                                                                     speedRampUpRateOpenLoopQ30)
{
    self->param->speedRampUpRateOpenLoopQ30 = speedRampUpRateOpenLoopQ30;

    if ((self->p_status.subState == Ifx_MS_FocSolution_F16_SubState_openLoop)
        || (self->p_status.subState == Ifx_MS_FocSolution_F16_SubState_transitionUp))
    {
        Ifx_Math_AccelLimit_F16_setSpeedStepUpLimit(&self->accelerationLimit, speedRampUpRateOpenLoopQ30);
    }
}


/**
 *  \brief Get the acceleration rate of the speed ramp in open-loop, represented in Q30.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return acceleration rate open-loop
 */
static inline Ifx_Math_Fract32 Ifx_MS_FocSolution_F16_getSpeedRampUpRateOpenLoop(Ifx_MS_FocSolution_F16* self)
{
    return self->param->speedRampUpRateOpenLoopQ30;
}


/**
 *  \brief Set the deceleration rate of the speed ramp in open-loop, represented in Q30.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] speedRampDownRateOpenLoopQ30 deceleration rate open-loop, greater or equal to 0, represented in Q30
 */
static inline void Ifx_MS_FocSolution_F16_setSpeedRampDownRateOpenLoop(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract32
                                                                       speedRampDownRateOpenLoopQ30)
{
    self->param->speedRampDownRateOpenLoopQ30 = speedRampDownRateOpenLoopQ30;

    if ((self->p_status.subState == Ifx_MS_FocSolution_F16_SubState_openLoop)
        || (self->p_status.subState == Ifx_MS_FocSolution_F16_SubState_transitionDown))
    {
        Ifx_Math_AccelLimit_F16_setSpeedStepDownLimit(&self->accelerationLimit, speedRampDownRateOpenLoopQ30);
    }
}


/**
 *  \brief Get the deceleration rate of the speed ramp in open-loop, represented in Q30.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return deceleration rate open-loop
 */
static inline Ifx_Math_Fract32 Ifx_MS_FocSolution_F16_getSpeedRampDownRateOpenLoop(Ifx_MS_FocSolution_F16* self)
{
    return self->param->speedRampDownRateOpenLoopQ30;
}


/**
 *  \brief Set the acceleration rate of the speed ramp in closed-loop, represented in Q30.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] speedRampUpRateClosedLoopQ30 acceleration rate closed-loop, greater or equal to 0, represented in Q30
 */
static inline void Ifx_MS_FocSolution_F16_setSpeedRampUpRateClosedLoop(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract32
                                                                       speedRampUpRateClosedLoopQ30)
{
    self->param->speedRampUpRateClosedLoopQ30 = speedRampUpRateClosedLoopQ30;

    if (self->p_status.subState == Ifx_MS_FocSolution_F16_SubState_closedLoop)
    {
        Ifx_Math_AccelLimit_F16_setSpeedStepUpLimit(&self->accelerationLimit, speedRampUpRateClosedLoopQ30);
    }
}


/**
 *  \brief Get the acceleration rate of the speed ramp in closed-loop, represented in Q30.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return acceleration rate closed-loop
 */
static inline Ifx_Math_Fract32 Ifx_MS_FocSolution_F16_getSpeedRampUpRateClosedLoop(Ifx_MS_FocSolution_F16* self)
{
    return self->param->speedRampUpRateClosedLoopQ30;
}


/**
 *  \brief Set the deceleration rate of the speed ramp in closed-loop, represented in Q30.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] speedRampDownRateClosedLoopQ30 deceleration rate closed-loop, greater or equal to 0, represented in Q30
 */
static inline void Ifx_MS_FocSolution_F16_setSpeedRampDownRateClosedLoop(Ifx_MS_FocSolution_F16* self,
                                                                         Ifx_Math_Fract32
                                                                         speedRampDownRateClosedLoopQ30)
{
    self->param->speedRampDownRateClosedLoopQ30 = speedRampDownRateClosedLoopQ30;

    if (self->p_status.subState == Ifx_MS_FocSolution_F16_SubState_closedLoop)
    {
        Ifx_Math_AccelLimit_F16_setSpeedStepDownLimit(&self->accelerationLimit, speedRampDownRateClosedLoopQ30);
    }
}


/**
 *  \brief Get the deceleration rate of the speed ramp in closed-loop, represented in Q30.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return deceleration rate closed-loop
 */
static inline Ifx_Math_Fract32 Ifx_MS_FocSolution_F16_getSpeedRampDownRateClosedLoop(Ifx_MS_FocSolution_F16* self)
{
    return self->param->speedRampDownRateClosedLoopQ30;
}


/**
 *  \brief Set the previous value of speed ramp.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] speedRampPreviousValue Previous value of the speed ramp
 */
static inline void Ifx_MS_FocSolution_F16_setSpeedRampPreviousValue(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                    speedRampPreviousValue)
{
    Ifx_Math_AccelLimit_F16_setSpeedStepPreviousValue(&self->accelerationLimit, speedRampPreviousValue);
}


/**
 *  \brief Get the previous value of the speed ramp.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return previous value of the speed ramp
 */
static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_getSpeedRampPreviousValue(Ifx_MS_FocSolution_F16* self)
{
    return Ifx_Math_AccelLimit_F16_getSpeedStepPreviousValue(&self->accelerationLimit);
}


/**
 *  \brief Set the transition down D current scaling factor as well as the target D current
 *
 *  This API sets the internal variable which stores the transition down D current
 *  scaling factor which is the ratio between the target D current for smooth
 *  transition and the startup current. The target D current is the value to which
 *  the D current is ramped up to before entering the transition down state from the
 *  closed loop state. In addition, it also sets the transition down target D
 *  current as the product of the transition down D current scaling factor and the
 *  startup current. The transition down D current scaling factor shall be between 0
 *  and 2-2^(-14), in Q14. If the input parameter is less than zero, then the
 *  internal parameter will be set to zero.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] transitionDownDCurrentScalingQ14 Scaling factor by which the startup current is multiplied to
 * determine the target D current during smooth transition down,
 * between 0 and 2-2^(-14), in Q14
 */
void Ifx_MS_FocSolution_F16_setTransitionDownDCurrentScalingQ14(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                transitionDownDCurrentScalingQ14);

/**
 *  \brief Get the transition down D current scaling factor
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return internal variable transitionDownDCurrentScalingQ14
 */
static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_getTransitionDownDCurrentScalingQ14(
    Ifx_MS_FocSolution_F16* self)
{
    return self->param->transitionDownDCurrentScalingQ14;
}


/**
 *  \brief Set the startup current ramp up rate
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] startUpCurrentRampUpRateQ15 Startup current ramp up rate, greater or equal to 0, in Q15
 */
static inline void Ifx_MS_FocSolution_F16_setStartUpCurrentRampUpRate(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                      startUpCurrentRampUpRateQ15)
{
    /* Initialize startup current up rate configuration */
    self->param->startUpCurrentRampUpRateQ15 = startUpCurrentRampUpRateQ15;

    /* Set rate limiter up rate */
    Ifx_Math_RateLimit_F16_setUpRate(&(self->p_startCurrentRateLimit), self->param->startUpCurrentRampUpRateQ15);
}


/**
 *  \brief Set the transition speed up
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] transitionSpeedUpQ15 Speed threshold to transition from open to closed loop, greater than 0,
 * normalized by the base speed, in Q15
 */
void Ifx_MS_FocSolution_F16_setTransitionSpeedUp(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 transitionSpeedUpQ15);

/**
 *  \brief Set the transition speed band
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] transitionSpeedBandQ15 Difference between up and down transition speeds, greater than 0, normalized by
 * the base speed, in Q15
 */
void Ifx_MS_FocSolution_F16_setTransitionSpeedBand(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                   transitionSpeedBandQ15);

/**
 *  \brief Set the upper limit of the reference current
 *
 *  This API sets the reference current upper limit configuration. Additionally it
 *  sets the upper limit of the output of the speed PI as well as the current
 *  limiter to the same limit.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] refCurrentUpperLimitQ15 Reference current upper limit, normalized by the base current, in Q15
 */
static inline void Ifx_MS_FocSolution_F16_setRefCurrentUpperLimit(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                  refCurrentUpperLimitQ15)
{
    /* Set internal parameter*/
    self->param->refCurrentUpperLimitQ15 = refCurrentUpperLimitQ15;

    /* Set the upper limit of the speed PI */
    Ifx_Math_Pi_F16_setUpperLimit(&(self->speedPi), refCurrentUpperLimitQ15);

    /* Set upper limit of reference current limiter */
    Ifx_Math_Limit_F16_setUpperLimit(&(self->p_refCurrentLimit), refCurrentUpperLimitQ15);
}


/**
 *  \brief Set the lower limit of the reference current
 *
 *  This API sets the reference current lower limit configuration. Additionally it
 *  sets the lower limit of the output of the speed PI as well as the current
 *  limiter to the same limit.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] refCurrentLowerLimitQ15 Reference current lower limit, normalized by the base current, in Q15
 */
static inline void Ifx_MS_FocSolution_F16_setRefCurrentLowerLimit(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                  refCurrentLowerLimitQ15)
{
    /* Set internal parameter*/
    self->param->refCurrentLowerLimitQ15 = refCurrentLowerLimitQ15;

    /* Set the lower limit of the speed PI */
    Ifx_Math_Pi_F16_setLowerLimit(&(self->speedPi), refCurrentLowerLimitQ15);

    /* Set lower limit of reference current limiter */
    Ifx_Math_Limit_F16_setLowerLimit(&(self->p_refCurrentLimit), refCurrentLowerLimitQ15);
}


/**
 *  \brief Set the proportional gain of the speed PI controller
 *
 *  This function calculates the proportional gain in Q format and value from the
 *  input propGain and sets the proportional gain of the speed PI controller.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] propGain Proportional gain
 */
void Ifx_MS_FocSolution_F16_setSpeedPiPropGain(Ifx_MS_FocSolution_F16* self, uint32_t propGain);

/**
 *  \brief Get the speed PI proportional gain
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Speed PI proportional gain in unsigned 32-bit format
 */
uint32_t Ifx_MS_FocSolution_F16_getSpeedPiPropGain(Ifx_MS_FocSolution_F16* self);

/**
 *  \brief Set the product of integral gain and sample time of the speed PI controller
 *
 *  This function calculates the product of integral gain and sample time in Q
 *  format and value from the input integGainSamplingTime and sets the gain of the
 *  speed PI controller.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] integGainSamplingTime Product of integral gain and sample time
 */
void Ifx_MS_FocSolution_F16_setSpeedPiIntegGainSamplingTime(Ifx_MS_FocSolution_F16* self, uint32_t
                                                            integGainSamplingTime);

/**
 *  \brief Get the speed PI product of integral gain and sampling time.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Speed PI product of integral gain and sampling time in unsigned 32-bit format
 */
uint32_t Ifx_MS_FocSolution_F16_getSpeedPiIntegGainSamplingTime(Ifx_MS_FocSolution_F16* self);

/**
 *  \brief Set the upper limit of the reference speed
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] refSpeedUpperLimitQ15 Upper speed limit
 */
void Ifx_MS_FocSolution_F16_setRefSpeedUpperLimit(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                  refSpeedUpperLimitQ15);

/**
 *  \brief Set the lower limit of the reference speed
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] refSpeedLowerLimitQ15
 */
void Ifx_MS_FocSolution_F16_setRefSpeedLowerLimit(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                  refSpeedLowerLimitQ15);

/**
 *  \brief Get the lower limit of the reference speed
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return refSpeedLowerLimitQ15
 */
Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_getRefSpeedLowerLimit(Ifx_MS_FocSolution_F16* self);

/**
 *  \brief Get the upper limit of the reference speed
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return refSpeedUpperLimitQ15
 */
Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_getRefSpeedUpperLimit(Ifx_MS_FocSolution_F16* self);

#endif /*IFX_MS_FOCSOLUTION_F16_H*/
