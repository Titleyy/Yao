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
#include "Ifx_MS_FocSolution_F16.h"
#include "Ifx_MS_FocSolution_F16_Cfg.h"

/* C library */
#include "stddef.h"

/* Math library includes */
#include "Ifx_Math_Arithmetic_F16.h"
#include "Ifx_Math_CartToPolar.h"
#include "Ifx_Math_Clarke.h"
#include "Ifx_Math_Park.h"
#include "Ifx_Math_PolarToCart.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/* Macro to saturate the value with wrapping */
#define IFX_MS_FOCSOLUTION_F16_2MAX                              (-2 * IFX_MATH_FRACT16_MIN)

/* Represents -PI in signed 16-bit */
#define IFX_MS_FOCSOLUTION_F16_PINEG                             (IFX_MATH_FRACT16_MIN + 1)

/* 30 degrees in Q32 */
#define IFX_MS_FOCSOLUTION_F16_PIBY6                             (IFX_MATH_PI_INDEX / 6U)

/* 90 degrees in Q32 */
#define IFX_MS_FOCSOLUTION_F16_PIBY2                             (IFX_MATH_PI_INDEX / 2U)

/* Macro for transition mode when mode is direct transition */
#define IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_DIRECT_TRANSITION (0)

/* Macro for transition mode when mode is smooth transition */
#define IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION (1)

/* Macro to define number of decimal places parameters with variable Q formats */
#define IFX_MS_FOCSOLUTION_F16_DEC_PLACES                        10000

/* *INDENT-OFF* */
/* Macros to define the component ID */
#define IFX_MS_FOCSOLUTION_F16_COMPONENTID_SOURCEID \
    ((uint8_t) Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MS_FOCSOLUTION_F16_COMPONENTID_LIBRARYID      ((uint16_t)Ifx_ComponentID_LibraryID_mctrlSolution)
#define IFX_MS_FOCSOLUTION_F16_COMPONENTID_MODULEID       (0U)
#define IFX_MS_FOCSOLUTION_F16_COMPONENTID_COMPONENTID1   (1U)
#define IFX_MS_FOCSOLUTION_F16_COMPONENTID_COMPONENTID2   ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_MAJOR     (2U)
#define IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_MINOR     (0U)
#define IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_PATCH     (0U)
#define IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_T         (4U)
#define IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_REV       (0U)
/* *INDENT-ON* */
/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Initialization functions called by Ifx_MS_FocSolution_F16_init() */
void               Ifx_MS_FocSolution_F16_initModules(Ifx_MS_FocSolution_F16* self,
                                                      Ifx_MS_FocSolution_F16_ParamComposition* paramComposition);
static inline void Ifx_MS_FocSolution_F16_initDriveAlgo(Ifx_MS_FocSolution_F16* self,
                                                        Ifx_MS_FocSolution_F16_ParamComposition* paramComposition);
static inline void Ifx_MS_FocSolution_F16_initSpeedPi(Ifx_MS_FocSolution_F16* self);
static inline void Ifx_MS_FocSolution_F16_initSpeedAccelerationLimiters(Ifx_MS_FocSolution_F16* self);
static inline void Ifx_MS_FocSolution_F16_initStartCurrentRateLimiter(Ifx_MS_FocSolution_F16* self);

/* Functions called by Ifx_MS_FocSolution_F16_executeControlMode() */

/* Calls the DriverGroup APIs to measure and to get the reconstructed currents and DC-Link voltage, stores the current 
 * reconstruction information for the next cycle, calls the Math API to perform the Clarke transformation from UVW to 
 * alpha-beta and returns the reconstructed currents and DC-Link voltage */
static inline Ifx_MHA_DriverGroup_F16_Output Ifx_MS_FocSolution_F16_measureAndReconstruct(Ifx_MS_FocSolution_F16* self);

/* Calls the FluxEstimator APIs to perform the estimation of the speed/position, stores the voltage in alpha beta format
 * for the next cycle, assigns the estimated speed to the output and returns the estimated angle */
static inline uint32_t              Ifx_MS_FocSolution_F16_fluxEstimation(Ifx_MS_FocSolution_F16* self);

/* Checks if the Q command has changed its sign and potentially calls the private API for rotating the ref. DQ system, 
 * calls the private APIs for openLoop/closedLoop depending on the current subState and executes the FocController and 
 * returns its calculated output voltage command */
static inline Ifx_Math_PolarFract16 Ifx_MS_FocSolution_F16_regulationLoop(Ifx_MS_FocSolution_F16* self,
                                                                          uint32_t estimatedAngle, Ifx_Math_Fract16
                                                                          dcLinkVoltageQ15);

/* Implement voltage generation by calling the Modulator execute function and passing on the calculated compare values
 * and trigger times to the DriverGroup. Use the Modulator output to update the current reconstruction information and
 * and to convert the output voltage to cartesian (voltageAlphaBeta) for the flux estimation in the next cycle. */
static inline void Ifx_MS_FocSolution_F16_voltageGeneration(Ifx_MS_FocSolution_F16       * self,
                                                            Ifx_Math_PolarFract16          voltageCommandPolar,
                                                            Ifx_MHA_DriverGroup_F16_Output driverGroupOutput);
/* *INDENT-ON* */
static void Ifx_MS_FocSolution_F16_stateMachine(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 speedQ15,
                                                Ifx_Math_CmpFract16 currentsDqRef);

/* State functions called by Ifx_MS_FocSolution_F16_stateMachine function */
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateInit(Ifx_MS_FocSolution_F16* self);
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateOff(Ifx_MS_FocSolution_F16* self, bool
                                                                           faultStatus);
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateStandby(Ifx_MS_FocSolution_F16* self, bool
                                                                               faultStatus);
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateFault(Ifx_MS_FocSolution_F16* self);

/* State transition functions called by Ifx_MS_FocSolution_F16_stateStandby */

/* Implements the transition from StandBy to Off. Calls localEnable to disable all underlying modules,
 * checkModulesStateOff to check whether all underlying modules are in off state and returns the nextState
 * accordingly. */
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateStandbyToOff(Ifx_MS_FocSolution_F16* self);

/* Implements the transition from StandBy to Run. Updates the control mode using getControlMode, sets the open loop ramp
 * up/down rates of the Math AccelLimit component, initializes the substate machine to openLoop, enables the
 * FluxEstimator component and returns the nextState run. */
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateStandbyToRun(Ifx_MS_FocSolution_F16* self);

/* Functions called by Ifx_MS_FocSolution_F16_stateFault() */
/* Requests a clear fault of the underlying modules */
static inline bool Ifx_MS_FocSolution_F16_clearModuleFaults(Ifx_MS_FocSolution_F16* self);
static inline bool Ifx_MS_FocSolution_F16_clearModuleFaultsMHA(Ifx_MS_FocSolution_F16* self);
static inline void Ifx_MS_FocSolution_F16_clearModuleFaultsMAS(Ifx_MS_FocSolution_F16* self);

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1
static inline void Ifx_MS_FocSolution_F16_clearModuleFaultsMDA(Ifx_MS_FocSolution_F16* self);
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
/* Returns true if any module has a clear fault pending and false otherwise */
static inline bool Ifx_MS_FocSolution_F16_anyClearFaultIsPending(Ifx_MS_FocSolution_F16* self);

/* Check if all faults of the underlying module are successfully cleared */
static inline bool Ifx_MS_FocSolution_F16_faultSuccessfullyCleared(
    Ifx_MS_FocSolution_F16* self);
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateRun(Ifx_MS_FocSolution_F16* self,
                                                                           Ifx_Math_Fract16 speedQ15, bool
                                                                           faultStatus, Ifx_Math_CmpFract16
                                                                           currentsDqRef);
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateRampDown(Ifx_MS_FocSolution_F16* self, bool
                                                                                faultStatus, Ifx_Math_CmpFract16
                                                                                currentsDqRef);

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateStartAngleIdent(Ifx_MS_FocSolution_F16* self,
                                                                                       Ifx_Math_Fract16 speedQ15, bool
                                                                                       faultStatus,
                                                                                       Ifx_MDA_IToFController_F16_Output
                                                                                       iToFOutput);

/* Set angle previous value of iToF controller to the estimated angle minus pi/2 offset and tolerance based on the sign
 * of the reference speed */
void Ifx_MS_FocSolution_F16_setAnglePreviousValue(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 speedQ15,
                                                  Ifx_MDA_StartAngleIdent_F16_Output startAngleIdentOutput);
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
/* Execute the sub-state machine */
static inline void Ifx_MS_FocSolution_F16_subStateMachine(Ifx_MS_FocSolution_F16* self, Ifx_Math_CmpFract16
                                                          currentsDqRef);

/* State functions called by Ifx_MS_FocSolution_F16_subStateMachine function */
static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateOpenLoop(Ifx_MS_FocSolution_F16* self,
                                                                                      Ifx_MDA_IToFController_F16_Output
                                                                                      iToFOutput, Ifx_Math_CmpFract16
                                                                                      currentsDqRef);

#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)
static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateTransitUp(Ifx_MS_FocSolution_F16* self,
                                                                                       Ifx_MDA_IToFController_F16_Output
                                                                                       iToFOutput, Ifx_Math_CmpFract16
                                                                                       currentsDqRef);

/* Implements the transition from transitionUp to closedLoop. Sets the q current command by calling currentQRef, sets
 * the Acceleration Limit ramp up/down rates to closed loop and returns the next state closedLoop. */
static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateTransitUpExit(
    Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 estimatedSpeedQ15, Ifx_Math_CmpFract16 currentsDqRef);

/* *INDENT-OFF* */
static inline
Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateTransitDown(Ifx_MS_FocSolution_F16 *self,
                                                                           Ifx_MDA_IToFController_F16_Output iToFOutput,
                                                                           Ifx_Math_CmpFract16 currentsDqRef);
#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */

static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateClosedLoop(Ifx_MS_FocSolution_F16* self,
                                                                                        Ifx_Math_CmpFract16
                                                                                        currentsDqRef);

/* Calculates the d current for smooth transition down based on the absolute estimated speed, the transitionSpeedMid
 * and the target d current for transition down. The d current shall ramp up linearily from 0 to the target d current as
 * the estimated speed ramps down from transitionSpeedMid to transitionSpeedDown. */
static inline void Ifx_MS_FocSolution_F16_smoothTransitionDownCalc(Ifx_MS_FocSolution_F16* self,
                                                                   Ifx_Math_Fract16 estimatedSpeedAbsQ15);
/* *INDENT-ON* */

/* Implements the transition from closedLoop to openLoopReset and sets/resets the following:
 *  - IToF previous angle to last flux estimator angle
 *  - Acceleration Limit state to the last estimated speed
 *  - Acceleration Limit ramp up/down rates to open loop
 * Additionally, it stores the last ref. q current to calculate the slope in subStateTransitDown, and resets the
 * transition counter. It returns the next state transitionDown in case smooth transition is enabled, or openLoop if
 * disabled. */
static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateClosedLoopExit(
    Ifx_MS_FocSolution_F16* self);

/* Implement open loop by calling the IToF controller with the normalized rate limited speed to generate and set the
 * angle */
static inline void Ifx_MS_FocSolution_F16_openLoop(Ifx_MS_FocSolution_F16* self);

/* Implement closed loop by setting the angle to the input parameter which shall be set to the output angle of the flux
 * estimator */
static inline void Ifx_MS_FocSolution_F16_closedLoop(Ifx_MS_FocSolution_F16* self, uint32_t fluxEstimatorAngle);

/* Execute the fast loop operations with VToF */
static inline Ifx_Math_PolarFract16 Ifx_MS_FocSolution_F16_vToFLoop(Ifx_MS_FocSolution_F16* self);

/* Reset the previous values of this component and underlying components to their initial values. Reset the rate limited
 * input speed and the previous current reconstruction info directly and reset the Q current command to the initial
 * startup current. Reset the underlying components by calling their respective APIs, this includes:
 *  - Low Pass Filters of the flux estimator
 *  - PLL of the flux estimator
 *  - Speed PI controller
 *  - FocController
 *  - Acceleration Limiter
 *  - Rate Limiter (reset to the initial startup current) */
static void Ifx_MS_FocSolution_F16_reset(Ifx_MS_FocSolution_F16* self);

/* Fault check for all the modules */
static bool Ifx_MS_FocSolution_F16_faultStatus(Ifx_MS_FocSolution_F16* self);

/* API to enable / disable all the modules */
static void Ifx_MS_FocSolution_F16_localEnable(Ifx_MS_FocSolution_F16* self, bool enable);

/* Apply the Math Limit and AccelLimit components on the input speed and set the rate limited speed accordingly */
static inline void Ifx_MS_FocSolution_F16_limitSpeed(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 speedQ15);

/* Check if all modules are in the on state */
static inline bool Ifx_MS_FocSolution_F16_checkModulesStateOn(Ifx_MS_FocSolution_F16* self);

/* Calculate and return the speed error in Q14 format */
static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_calcSpeedErrorQ14(Ifx_Math_Fract16 rateLimitInSpeedQ15,
                                                                        Ifx_Math_Fract16 estimatedSpeedQ15);

/**
 * \brief Check and process clear fault requests
 *
 * Resets the clearFault request once it has been propagated to all underlying components and requests a clearFault
 * of FocSolution itself in case it is in fault state.
 *
 * Inputs and outputs of this API:
 *     [in] self->p_status.state
 *     [inout] self->p_clearFault
 *     [out] self->p_clearFaultIsRequested
 *
 * \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MS_FocSolution_F16_checkAndProcessClearFault(Ifx_MS_FocSolution_F16* self);

/* Determine and return the ref. q current in the speed loop. If direct interface is enabled it sets it equal to the
 * direct interface Q ref. current and resets the speed PI with the input ref. Q current. Else it calls currentQRefCalc
 * to calculate the ref. q current based on the estimated speed and the rate limited speed. */
static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_currentQRef(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                  estimatedSpeedQ15, Ifx_Math_Fract16 currentQRef);

/* Calculate the ref. q current by determining the speed error with calcSpeedErrorQ14, getting the q voltage saturation
 * status from the FocController and passing both values on to the speed PI controller. If speed precontrol is enabled,
 * the output of the PI controller is passed on to calcSumCurrentQRef, which calculates the ref q. current. Else the ref
 * q. current is directly set to the output of the PI controller and the state of SpeedPreControl is reset with the last
 * ref. speed. */
static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_currentQRefCalc(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                      estimatedSpeedQ15);

/**
 * \brief Calculate the transition speed down, middle, and half the band
 *
 * This function calculates the transition speed down and middle transition speed. It also calculates half of the
 * transition speed band.
 *
 * \param self Reference to structure that contains instance data members
 */
static inline void Ifx_MS_FocSolution_F16_p_calcInternalTransitionSpeeds(Ifx_MS_FocSolution_F16* self);

#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)

/* Implement the check for the transition between open and closed loop. As long as the conditions are not fulfilled
 * either ramp down the Q current by calling decreaseQCurrent or set the dqCommand directly in case the direct interface
 * is enabled.
 * Conditions for the transition:
 *  - the angle error returned by private function angleError is smaller than the transition angle tolerance, OR
 *  - the maximum transition time has elapsed
 * Return true if at least one of the conditions is true, and additionally set the integral previous value of the speed
 * pi controller to the last reached ref. q current to improve the transient response in closed loop. */
static inline bool Ifx_MS_FocSolution_F16_angleMergeCheck(Ifx_MS_FocSolution_F16          * self,
                                                          Ifx_MDA_IToFController_F16_Output iToFOutput,
                                                          Ifx_Math_CmpFract16               currentsDqRef);

/* Calculate and return the angle error by subtracting the output of the Flux Estimator (estimated angle) from the
 * output of the IToF Controller (reference angle) and wrapping the result within -pi and pi */
static inline Ifx_Math_Fract32 Ifx_MS_FocSolution_F16_angleError(Ifx_MS_FocSolution_F16          * self,
                                                                 Ifx_MDA_IToFController_F16_Output iToFOutput);

/* Increase the amplitude of the reference Q current when transitioning/ramping down. This API calculates the difference
 * between the last ref. Q current and the IToF ref. Q current and uses the transition counter to calculate a slope to
 * set the Q current command to. In case of a positive rate limited speed, the Q current command ramps to the positive
 * IToF ref. Q current, and in case of a negative rate limited speed, the Q current command ramps to the
 * negative IToF ref. Q current. */
static inline void Ifx_MS_FocSolution_F16_increaseQCurrent(Ifx_MS_FocSolution_F16                * self,
                                                           Ifx_MDA_IToFController_F16_Output const iToFOutput,
                                                           Ifx_Math_Fract16 const
                                                           transitionCounterQ15);

/* Decrease the amplitude of the reference Q current when transitioning up. This API increments the transition counter,
 * calculates a slope based on the transition time and then decreases the amplitude of the imaginary ref. current by
 * that slope. In case of a positive rate limited speed the ramp starts at the positive IToF ref. Q current, and in case
 * of a negative rate limited speed at the negative IToF ref. Q current. */
static inline void Ifx_MS_FocSolution_F16_decreaseQCurrent(Ifx_MS_FocSolution_F16                * self,
                                                           Ifx_MDA_IToFController_F16_Output const iToFOutput);

/* Decrease the amplitude of the reference D current when transitioning/ramping down. It linearily decreases the actual
 * D current based on the transitionCounter, the input reference D current and the transition down target D current.
 * This function expects a positive reference D current as well as a positive transition down target D current. */
static inline void Ifx_MS_FocSolution_F16_decreaseDCurrent(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 const
                                                           currentDRefQ15, Ifx_Math_Fract16 const
                                                           transitionCounterQ15);
#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */

/**
 * \brief Calculate internal variable p_transitionDeltaQ15
 *
 * This function calculates the internal variable p_transitionDeltaQ15 based on the transitionTimeLimit_cycles input,
 * as the ratio of IFX_MATH_FRACT16_MAX and transitionTimeLimit_cycles. If transitionTimeLimit_cycles is greater than
 * IFX_MATH_FRACT16_MAX, then p_transitionDeltaQ15 is set to 1.
 *
 * \param self Reference to structure that contains instance data members
 * \param transitionTimeLimit_cycles Time limit to do the transition, in number of execution cycles
 */
static inline void Ifx_MS_FocSolution_F16_p_setTransitionDelta(Ifx_MS_FocSolution_F16* self, uint16_t
                                                               transitionTimeLimit_cycles);

/**
 * \brief Set the transition down target D current
 *
 * This function calculates the internal parameter transitionDownTargetDCurrentQ15 as a multiplication of the
 * startup current and target D current scaling factor.
 *
 * \param self Reference to structure that contains instance data members
 */
static inline void Ifx_MS_FocSolution_F16_p_setTransitionDownTargetDCurrentQ15(Ifx_MS_FocSolution_F16* self);

/* API to initialize speed precontrol */
static inline void Ifx_MS_FocSolution_F16_initSpeedPreControl(Ifx_MS_FocSolution_F16* self);

/* Initialize reference current limiter */
static inline void Ifx_MS_FocSolution_F16_initRefCurrentLimiter(Ifx_MS_FocSolution_F16* self);

/* Calculate the sum of feedforward and feedback current by executing the Math speedPreControl component using the rate
 * limited speed as input, adding the result to the speedPiOut, limiting using the Math limit component and returning
 * the limited result */
static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_calcSumCurrentQRef(Ifx_MS_FocSolution_F16* self,
                                                                         Ifx_Math_Fract16        speedPiOut);

/* Check if all modules are in the off state */
static inline bool Ifx_MS_FocSolution_F16_checkModulesStateOff(Ifx_MS_FocSolution_F16* self);

/* API to initialize the iToF angle and to set the dq ref. currents, based on the ref. speed. The d current command is
 * directly set to the IToF reference d current, and the q current command is determined by calling setQCommand.
 * Additionally, update the previousQCommand variable. */
static inline void Ifx_MS_FocSolution_F16_setDqCommand(Ifx_MS_FocSolution_F16* self, Ifx_MDA_IToFController_F16_Output
                                                       iToFOutput);

/* API to set the q current command based on the rate limited speed. First executes the Math RateLimit component to
 * limit the rate of change of the q current command. Then it sets the q current command to the output of the RateLimit,
 * respectively to the negated output in case the rate limited speed is negative. Additionally, calls
 * detectQCommandZeroCross to check whether the Q command changed its sign. */
static inline void Ifx_MS_FocSolution_F16_setQCommand(Ifx_MS_FocSolution_F16* self, Ifx_MDA_IToFController_F16_Output
                                                      iToFOutput);

/* API to detect current Q command sign change */
static inline void Ifx_MS_FocSolution_F16_detectQCommandZeroCross(Ifx_MS_FocSolution_F16* self);

/* Rotates the DQ reference system by calling the IToFController API to add 180 degrees (Pi) to the I2F angle and by
 * resetting the current PIs with the negative of their previous value */
static inline void Ifx_MS_FocSolution_F16_rotateDQRefSystem(Ifx_MS_FocSolution_F16* self);

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Component ID */
static const Ifx_ComponentID      Ifx_MS_FocSolution_F16_componentID = {
    .sourceID     = IFX_MS_FOCSOLUTION_F16_COMPONENTID_SOURCEID,
    .libraryID    = IFX_MS_FOCSOLUTION_F16_COMPONENTID_LIBRARYID,
    .moduleID     = IFX_MS_FOCSOLUTION_F16_COMPONENTID_MODULEID,
    .componentID1 = IFX_MS_FOCSOLUTION_F16_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_MS_FOCSOLUTION_F16_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MS_FocSolution_F16_componentVersion = {
    .major = IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_MAJOR,
    .minor = IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_MINOR,
    .patch = IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_PATCH,
    .t     = IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_T,
    .rev   = IFX_MS_FOCSOLUTION_F16_COMPONENTVERSION_REV
};

/******************************************************************************/
/*-----------------------Exported Variables/Constants-------------------------*/
/******************************************************************************/
/* temporary solution, XML_VERSION indicates if static cfg is generated by cfgwiz */
#ifdef IFX_MS_FOCSOLUTION_F16_CFG_XML_VERSION
/* Default parameters initialized with CfgWizard defines */
Ifx_MS_FocSolution_F16_Param Ifx_MS_FocSolution_F16_g_defaultParam =
{
    .transitionAngleTolerance          = (int32_t)IFX_MS_FOCSOLUTION_F16_CFG_ANGLE_ERROR_MIN,
    .speedRampUpRateOpenLoopQ30        = IFX_MS_FOCSOLUTION_F16_CFG_OPEN_LOOP_RAMP_UP_RATE_Q30,
    .speedRampDownRateOpenLoopQ30      = IFX_MS_FOCSOLUTION_F16_CFG_OPEN_LOOP_RAMP_DOWN_RATE_Q30,
    .speedRampUpRateClosedLoopQ30      = IFX_MS_FOCSOLUTION_F16_CFG_CLOSED_LOOP_RAMP_UP_RATE_Q30,
    .speedRampDownRateClosedLoopQ30    = IFX_MS_FOCSOLUTION_F16_CFG_CLOSED_LOOP_RAMP_DOWN_RATE_Q30,
    .transitionTimeLimit_cycles        = (uint16_t)IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_TIME_CYCLES,
    .startUpCurrentQ15                 = IFX_MS_FOCSOLUTION_F16_CFG_START_UP_CURRENT_Q15,
    .initStartupCurrentQ15             = IFX_MS_FOCSOLUTION_F16_CFG_INIT_START_UP_CURRENT_Q15,
    .transitionDownDCurrentScalingQ14  = IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_D_CURRENT_SCALING_Q14,
    .startUpCurrentRampUpRateQ15       = IFX_MS_FOCSOLUTION_F16_CFG_START_CURRENT_RAMP_UP_RATE_Q15,
    .qCurrentAtTransitionQ15           = IFX_MS_FOCSOLUTION_F16_CFG_Q_CURRENT_AT_TRANSITION_Q15,
    .minimumSpeedThresholdQ15          = IFX_MS_FOCSOLUTION_F16_CFG_MIN_SPEED_THRESHOLD_Q15,
    .transitionSpeedUpQ15              = IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_SPEED_UP_Q15,
    .transitionSpeedBandQ15            = IFX_MS_FOCSOLUTION_F16_CFG_SPEED_BAND_Q15,
    .refCurrentLowerLimitQ15           = IFX_MS_FOCSOLUTION_F16_CFG_SPEED_PI_OUT_LOW_LIMIT_Q,
    .refCurrentUpperLimitQ15           = IFX_MS_FOCSOLUTION_F16_CFG_SPEED_PI_OUT_UPP_LIMIT_Q,
    .speedPiPropGain                   = IFX_MS_FOCSOLUTION_F16_CFG_SPEED_PI_PROPGAIN_N_TU,
    .speedPiIntegGainSamplingTime      = IFX_MS_FOCSOLUTION_F16_CFG_SPEED_PI_KI_TS_N_TU,
    .speedPiAntiWindupGainSamplingTime = IFX_MS_FOCSOLUTION_F16_CFG_SPEED_PI_KAW_TS_N_TU,
    .frictionConstant                  = IFX_MS_FOCSOLUTION_F16_CFG_VISCOUS_FRICTION_CONSTANT_Q15,
    .rotorInertiaOverSamplingTime      = IFX_MS_FOCSOLUTION_F16_CFG_INERTIA_BY_TS_N_TU,
    .inverseTorqueConstant             = IFX_MS_FOCSOLUTION_F16_CFG_INVERSE_TORQUE_CONSTANT_Q15,
    .refSpeedUpperLimitQ15             = IFX_MS_FOCSOLUTION_F16_CFG_MAXIMUM_SPEED_Q15,
    .refSpeedLowerLimitQ15             = IFX_MS_FOCSOLUTION_F16_CFG_MINIMUM_SPEED_Q15,
};

Ifx_MS_FocSolution_F16_ParamComposition Ifx_MS_FocSolution_F16_g_defaultParamComposition =
{
    .fluxEstimatorParam  = &Ifx_MDA_FluxEstimator_F16_g_defaultParam,
    .focControllerParam  = &Ifx_MDA_FocController_F16_g_defaultParam,
    .iToFControllerParam = &Ifx_MDA_IToFController_F16_g_defaultParam,
    .modulatorParam      = &Ifx_MAS_Modulator_F16_g_defaultParam,
    .vToFControllerParam = &Ifx_MDA_VToFController_F16_g_defaultParam,
};
#endif
/* *INDENT-ON* */
/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
/* Function to get the component ID */
void Ifx_MS_FocSolution_F16_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MS_FocSolution_F16_componentID;
}


/* Function to get the component version */
void Ifx_MS_FocSolution_F16_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MS_FocSolution_F16_componentVersion;
}


void Ifx_MS_FocSolution_F16_init(Ifx_MS_FocSolution_F16* self, Ifx_MS_FocSolution_F16_Param* param,
                                 Ifx_MS_FocSolution_F16_ParamComposition* paramComposition)
{
    self->param = param;

    /* Initialize modules from used libraries */
    Ifx_MS_FocSolution_F16_initModules(self, paramComposition);

    /* Initialize speed PI controller */
    Ifx_MS_FocSolution_F16_initSpeedPi(self);

    /* Initialize internal transition speed parameters */
    Ifx_MS_FocSolution_F16_p_calcInternalTransitionSpeeds(self);

#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)

    /* Init transition counter and limit */
    Ifx_MS_FocSolution_F16_p_setTransitionDelta(self, self->param->transitionTimeLimit_cycles);
    self->p_transitionCounter_cycles = 0u;

    /* Set the D current for smooth transition */
    Ifx_MS_FocSolution_F16_p_setTransitionDownTargetDCurrentQ15(self);

#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */
    /* Initialize speed and acceleration limiters */
    Ifx_MS_FocSolution_F16_initSpeedAccelerationLimiters(self);

    /* Initialize startup current rate limiter */
    Ifx_MS_FocSolution_F16_initStartCurrentRateLimiter(self);

    /* Initialize internal variables */
    self->p_status.state          = Ifx_MS_FocSolution_F16_State_init;
    self->p_status.subState       = Ifx_MS_FocSolution_F16_SubState_openLoop;
    self->p_enablePowerStage      = false;
    self->p_enableControl         = false;
    self->p_clearFault            = false;
    self->p_clearFaultIsRequested = false;
    self->p_qCommandZeroCrossing  = false;
}


void Ifx_MS_FocSolution_F16_executeSpeedControl(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 speedQ15,
                                                Ifx_Math_CmpFract16 currentsDqRef)
{
    Ifx_MS_FocSolution_F16_checkAndProcessClearFault(self);
    Ifx_MHA_DriverGroup_F16_executeBridgeDriver(&(self->driverGroup));
    Ifx_MS_FocSolution_F16_stateMachine(self, speedQ15, currentsDqRef);

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1

    /* Disable start angle identification */
    self->p_enableStartAngleIdent = false;
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
}


static inline void Ifx_MS_FocSolution_F16_checkAndProcessClearFault(Ifx_MS_FocSolution_F16* self)
{
    if (self->p_clearFault == true)
    {
        /* Request to clear the faults of the underlying components  no matter whether FocSolution is in fault state or
         * not to make sure that the fault bits of the underlying components can be cleared */
        bool clearFaultPropagated = Ifx_MS_FocSolution_F16_clearModuleFaults(self);

        if (self->p_status.state == Ifx_MS_FocSolution_F16_State_fault)
        {
            /* Request to clear the fault state of FocSolution itself */
            self->p_clearFaultIsRequested = true;
        }

        /* polyspace +2 MISRA-C3:14.3 [Justified:Low] "False positive, the controlling expression is not invariant,
         * depending on the target platform MHA the if condition can be true or false." */
        if (clearFaultPropagated == true)
        {
            self->p_clearFault = false;
        }
    }
}


static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_currentQRef(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                  estimatedSpeedQ15, Ifx_Math_Fract16 currentQRef)
{
    Ifx_Math_Fract16 refCurrent;

    /* Check if the direct interface is enabled */
    if (self->p_enableDirectInterface == true)
    {
        /* Set the ref. Q current to the input ref. Q current */
        refCurrent = currentQRef;

        /* Reset speed PI if direct interface is enabled with the input ref. Q current */
        Ifx_Math_Pi_F16_setIntegPreviousValue(&(self->speedPi), currentQRef);
    }
    else
    {
        refCurrent = Ifx_MS_FocSolution_F16_currentQRefCalc(self, estimatedSpeedQ15);
    }

    return refCurrent;
}


static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_currentQRefCalc(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                      estimatedSpeedQ15)
{
    Ifx_Math_Fract16               refCurrent;
    Ifx_Math_Fract16               errSpeedQ14     = Ifx_MS_FocSolution_F16_calcSpeedErrorQ14(
        self->rateLimitInSpeedQ15, estimatedSpeedQ15);
    Ifx_Math_Pi_F16_AntiWindupCtrl qAntiwindupCtrl = Ifx_MDA_FocController_F16_getQVoltageSatStatus(
        &(self->focController));

    /* Execute speed PI */
    Ifx_Math_Fract16               speedPiOut = Ifx_Math_Pi_F16_execute(&(self->speedPi), errSpeedQ14,
        qAntiwindupCtrl);

    /* Check if speedPreControl is enabled */
    if (self->p_enableSpeedPreControl == true)
    {
        /* Calculate sum of feedforward and feedback current and limit result */
        refCurrent = Ifx_MS_FocSolution_F16_calcSumCurrentQRef(self, speedPiOut);
    }
    else
    {
        /* Reset state of SpeedPreControl with the last ref. speed */
        Ifx_Math_SpeedPreControl_F16_setPreviousSpeed(&(self->speedPreControl), self->rateLimitInSpeedQ15);

        /* Set the ref. current to the output of the speed PI */
        refCurrent = speedPiOut;
    }

    return refCurrent;
}


static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_calcSpeedErrorQ14(Ifx_Math_Fract16 rateLimitInSpeedQ15,
                                                                        Ifx_Math_Fract16 estimatedSpeedQ15)
{
    Ifx_Math_Fract16 rateLimitInSpeedQ14 = Ifx_Math_ShR_F16(rateLimitInSpeedQ15, 1U);
    Ifx_Math_Fract16 estimatedSpeedQ14   = Ifx_Math_ShR_F16(estimatedSpeedQ15, 1U);
    Ifx_Math_Fract16 errSpeedQ14         = Ifx_Math_Sub_F16(rateLimitInSpeedQ14, estimatedSpeedQ14);

    return errSpeedQ14;
}


static inline Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_calcSumCurrentQRef(Ifx_MS_FocSolution_F16* self,
                                                                         Ifx_Math_Fract16        speedPiOut)
{
    /* Execute speed precontrol */
    Ifx_Math_Fract16 speedPreControlOut = Ifx_Math_SpeedPreControl_F16_execute(&(self->speedPreControl),
        self->rateLimitInSpeedQ15);

    /* Sum the two reference currents from speed control and speed precontrol */
    Ifx_Math_Fract16 refCurrentSum = Ifx_Math_AddSat_F16(speedPiOut, speedPreControlOut);

    /* Limit the sum of the two reference currents */
    Ifx_Math_Fract16 refCurrentSumLimited = Ifx_Math_Limit_F16_execute(&(self->p_refCurrentLimit), refCurrentSum);

    return refCurrentSumLimited;
}


/* API to execute the fast loop operations */
void Ifx_MS_FocSolution_F16_executeControlMode(Ifx_MS_FocSolution_F16* self)
{
    /* Local variables */
    Ifx_Math_PolarFract16          voltageCommandPolar;
    Ifx_MHA_DriverGroup_F16_Output driverGroupOutput;
    uint32_t                       estimatedAngle;

    /* Return voltage measurement and perform current measurement and reconstruction */
    driverGroupOutput = Ifx_MS_FocSolution_F16_measureAndReconstruct(self);

    /* Perform the angle and speed estimation */
    estimatedAngle = Ifx_MS_FocSolution_F16_fluxEstimation(self);

    if ((self->p_status.state == Ifx_MS_FocSolution_F16_State_run)
        || (self->p_status.state == Ifx_MS_FocSolution_F16_State_rampDown))
    {
        if (self->p_status.actualControlMode == Ifx_MS_FocSolution_F16_ControlMode_foc)
        {
            voltageCommandPolar = Ifx_MS_FocSolution_F16_regulationLoop(self, estimatedAngle,
                driverGroupOutput.dcLinkVoltageQ15);
        }

        /* p_status.actualControlMode == Ifx_MS_FocSolution_F16_ControlMode_vToF */
        else
        {
            voltageCommandPolar = Ifx_MS_FocSolution_F16_vToFLoop(self);
        }
    }
    else
    {
#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1

        /* Execute start angle identification */
        /* Ensure to call this function in all states, as otherwise clearfault will not be handled */
        Ifx_MDA_StartAngleIdent_F16_execute(&(self->startAngleIdent), driverGroupOutput.dcLinkVoltageQ15,
            driverGroupOutput.shuntCurrentsQ15);
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
        /* Modulator output is 0 while in standby, fault, off, startAngleIdent, init */
        voltageCommandPolar.amplitude = 0;
        voltageCommandPolar.angle     = 0;
    }

    /* Generate voltage according to the command */
    Ifx_MS_FocSolution_F16_voltageGeneration(self, voltageCommandPolar, driverGroupOutput);
}


static inline void Ifx_MS_FocSolution_F16_rotateDQRefSystem(Ifx_MS_FocSolution_F16* self)
{
    /* Add 180deg. to the I2f angle */
    Ifx_MDA_IToFController_F16_addOneEightyDegreeInAngle(&(self->iToF));

    /* Get current controllers previous value */
    Ifx_Math_Fract16 currentDPiPrevValue = Ifx_Math_Pi_F16_getIntegPreviousValue(&(self->focController.currentDPi));
    Ifx_Math_Fract16 currentQPiPrevValue = Ifx_Math_Pi_F16_getIntegPreviousValue(&(self->focController.currentQPi));

    /* Reset current PIs with negative of previous value */
    Ifx_Math_Pi_F16_setIntegPreviousValue(&(self->focController.currentDPi), Ifx_Math_NegSat_F16(
        currentDPiPrevValue));
    Ifx_Math_Pi_F16_setIntegPreviousValue(&(self->focController.currentQPi), Ifx_Math_NegSat_F16(
        currentQPiPrevValue));
}


static inline bool Ifx_MS_FocSolution_F16_clearModuleFaults(Ifx_MS_FocSolution_F16* self)
{
    /* Flag whether the clear fault request has been propagated to all underlying modules */
    bool clearFaultPropagated;

    /* Clear faults of MHA modules */
    clearFaultPropagated = Ifx_MS_FocSolution_F16_clearModuleFaultsMHA(self);

    /* Clear faults of MAS modules */
    Ifx_MS_FocSolution_F16_clearModuleFaultsMAS(self);

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1

    /* Clear faults of MDA modules */
    Ifx_MS_FocSolution_F16_clearModuleFaultsMDA(self);
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */

    return clearFaultPropagated;
}


static inline bool Ifx_MS_FocSolution_F16_clearModuleFaultsMHA(Ifx_MS_FocSolution_F16* self)
{
    /* Flag whether the clear fault request has been propagated to all underlying modules */
    bool clearFaultPropagated = false;

    /* Clear faults of modules */
    clearFaultPropagated = Ifx_MHA_DriverGroup_F16_clearFaults(&self->driverGroup);

    return clearFaultPropagated;
}


static inline void Ifx_MS_FocSolution_F16_clearModuleFaultsMAS(Ifx_MS_FocSolution_F16* self)
{
    /* Variable declaration */
    Ifx_MAS_Modulator_F16_Status modulatorStatus;

    /* Get the status of modules */
    modulatorStatus = Ifx_MAS_Modulator_F16_getStatus(&self->modulator);

    /* Clear faults of modules */
    if ((modulatorStatus.maxAmplitudeFlag != false)
        || (modulatorStatus.overmodulationFlag != false))
    {
        Ifx_MAS_Modulator_F16_clearFault(&self->modulator);
    }
}


#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1
static inline void Ifx_MS_FocSolution_F16_clearModuleFaultsMDA(Ifx_MS_FocSolution_F16* self)
{
    /* Variable declaration */
    Ifx_MDA_StartAngleIdent_F16_Status startAngleIdentStatus;

    /* Get the status of modules */
    startAngleIdentStatus = Ifx_MDA_StartAngleIdent_F16_getStatus(&self->startAngleIdent);

    /* Clear faults of modules */
    if (startAngleIdentStatus.voltageFluctuation != false)
    {
        Ifx_MDA_StartAngleIdent_F16_clearFault(&(self->startAngleIdent));
    }
}


#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
static inline Ifx_MHA_DriverGroup_F16_Output Ifx_MS_FocSolution_F16_measureAndReconstruct(
    Ifx_MS_FocSolution_F16* self)
{
    /* Get measured shunt currents from previous cycle */
    Ifx_MHA_DriverGroup_F16_Output driverGroupOutput;
    Ifx_MHA_DriverGroup_F16_measureAndReconstruct(&(self->driverGroup), self->previousCurrentReconstructionInfo);
    Ifx_MHA_DriverGroup_F16_getOutput(&(self->driverGroup), &driverGroupOutput);

    /* Update current reconstruction information for the next state */
    self->previousCurrentReconstructionInfo = self->p_currentReconstructionInfo;

    /* Current Clark transformation (UVW to alpha-beta) */
    self->currentsAlphaBeta = Ifx_Math_Clarke_F16(driverGroupOutput.currentsUVW);

    return driverGroupOutput;
}


static inline uint32_t Ifx_MS_FocSolution_F16_fluxEstimation(Ifx_MS_FocSolution_F16* self)
{
    /* Flux estimator */
    Ifx_MDA_FluxEstimator_F16_execute(&(self->fluxEstimator), self->previousVoltageAlphaBeta,
        self->currentsAlphaBeta);

    /* Store voltage for next cycle */
    self->previousVoltageAlphaBeta = self->voltageAlphaBeta;

    /* Assign estimated speed output */
    Ifx_MDA_FluxEstimator_F16_Output fluxEstimatorOutput;
    Ifx_MDA_FluxEstimator_F16_getOutput(&(self->fluxEstimator), &fluxEstimatorOutput);
    self->p_output.estimatedSpeedQ15 = fluxEstimatorOutput.speedQ15;

    /* Return estimated angle */
    return fluxEstimatorOutput.anglePLL;
}


/* Execute the fast loop operations with IToF and FOC */
static inline Ifx_Math_PolarFract16 Ifx_MS_FocSolution_F16_regulationLoop(Ifx_MS_FocSolution_F16* self, uint32_t
                                                                          estimatedAngle, Ifx_Math_Fract16
                                                                          dcLinkVoltageQ15)
{
    /* Check if the Q command changes sign */
    if (self->p_qCommandZeroCrossing == true)
    {
        /* Rotate ref. DQ system if Q command changes sign */
        Ifx_MS_FocSolution_F16_rotateDQRefSystem(self);

        /* Set the current zero cross flag to false */
        self->p_qCommandZeroCrossing = false;
    }

    if (self->p_status.subState == Ifx_MS_FocSolution_F16_SubState_closedLoop)
    {
        /* Set the angle for Park transformation from FE */
        Ifx_MS_FocSolution_F16_closedLoop(self, estimatedAngle);
    }
    else
    {
        /* Execute iToF and set the angle for Park transformation from iToF */
        Ifx_MS_FocSolution_F16_openLoop(self);
    }

    /* Field Oriented Controller */
    Ifx_MDA_FocController_F16_execute(&(self->focController), self->currentsAlphaBeta, self->dqCommand, self->angle,
        self->rateLimitInSpeedQ15, dcLinkVoltageQ15);
    Ifx_MDA_FocController_F16_Output focControllerOutput;
    Ifx_MDA_FocController_F16_getOutput(&(self->focController), &focControllerOutput);

    return focControllerOutput.voltageCommandPolar;
}


/* Open loop implementation */
static inline void Ifx_MS_FocSolution_F16_openLoop(Ifx_MS_FocSolution_F16* self)
{
    /* Execute IToF block to generate the angle */
    Ifx_MDA_IToFController_F16_execute(&(self->iToF), self->rateLimitInSpeedQ15);
    Ifx_MDA_IToFController_F16_Output iToFOutput;
    Ifx_MDA_IToFController_F16_getOutput(&(self->iToF), &iToFOutput);

    /* Angle is set by IToF */
    self->angle = iToFOutput.currentVecAngle_rad;
}


/* Closed loop implementation */
static inline void Ifx_MS_FocSolution_F16_closedLoop(Ifx_MS_FocSolution_F16* self, uint32_t fluxEstimatorAngle)
{
    /* Get the angle from the flux estimator */
    self->angle = fluxEstimatorAngle;
}


/* Execute the fast loop operations with VToF */
static inline Ifx_Math_PolarFract16 Ifx_MS_FocSolution_F16_vToFLoop(Ifx_MS_FocSolution_F16* self)
{
    Ifx_MDA_VToFController_F16_Output vToFControllerOutput;
    Ifx_MDA_VToFController_F16_execute(&(self->vToF), self->rateLimitInSpeedQ15);
    Ifx_MDA_VToFController_F16_getOutput(&(self->vToF), &vToFControllerOutput);

    return vToFControllerOutput.voltageVector;
}


static inline void Ifx_MS_FocSolution_F16_voltageGeneration(Ifx_MS_FocSolution_F16* self, Ifx_Math_PolarFract16
                                                            voltageCommandPolar, Ifx_MHA_DriverGroup_F16_Output
                                                            driverGroupOutput)
{
    /* Private variable to store the modulator output */
    Ifx_MAS_Modulator_F16_Output modulatorOutput;

    /* Private variables to store compare values and trigger times */
    uint16_t                   * compareValues_tick;
    uint16_t                   * triggerTime_tick;

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1

    /* Private variable to store the start angle identification output */
    Ifx_MDA_StartAngleIdent_F16_Output startAngleIdentOutput;

    /* Call pattern generator with compare values depending on the state */
    if (self->p_status.state != Ifx_MS_FocSolution_F16_State_startAngleIdent)
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
    {
        /* Call modulator */
        /* Ensure to call this function in all states, as otherwise clearfault will not be handled */
        Ifx_MAS_Modulator_F16_execute(&(self->modulator), voltageCommandPolar, driverGroupOutput.dcLinkVoltageQ15);
        Ifx_MAS_Modulator_F16_getOutput(&(self->modulator), &modulatorOutput);

        /* Store current reconstruction information */
        self->p_currentReconstructionInfo = modulatorOutput.currentReconstructionInfo;

        /* Convert actual voltage from modulator to cartesian, to be used by the flux estimator */
        self->voltageAlphaBeta = Ifx_Math_PolarToCart_F16(modulatorOutput.actualVoltage);

        /* Assign compare values and triggers */
        compareValues_tick = modulatorOutput.compareValues_tick;
        triggerTime_tick   = modulatorOutput.triggerTime_tick;
    }

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1
    else
    {
        /* Get start angle identification output */
        Ifx_MDA_StartAngleIdent_F16_getOutput(&(self->startAngleIdent), &startAngleIdentOutput);

        /* Assign compare values and triggers */
        compareValues_tick = startAngleIdentOutput.compareValues_tick;
        triggerTime_tick   = startAngleIdentOutput.triggerTime_tick;
    }
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
    /* Call pattern generator */
    /* Ensure to call this function in all states, as otherwise clearfault will not be handled */
    Ifx_MHA_DriverGroup_F16_setCompVal(&(self->driverGroup), compareValues_tick, triggerTime_tick);
}


/* Init all modules */
void Ifx_MS_FocSolution_F16_initModules(Ifx_MS_FocSolution_F16                 * self,
                                        Ifx_MS_FocSolution_F16_ParamComposition* paramComposition)
{
    /* Initialize hardware abstraction modules */
    Ifx_MHA_DriverGroup_F16_initModules(&(self->driverGroup));

    /* Initialize building blocks and drive algorithm modules */
    Ifx_MAS_Modulator_F16_init(&(self->modulator), paramComposition->modulatorParam);
    Ifx_MS_FocSolution_F16_initDriveAlgo(self, paramComposition);

    /* Init speed precontrol */
    Ifx_MS_FocSolution_F16_initSpeedPreControl(self);

    /* Init reference current limiter */
    Ifx_MS_FocSolution_F16_initRefCurrentLimiter(self);
}


static inline void Ifx_MS_FocSolution_F16_initSpeedPreControl(Ifx_MS_FocSolution_F16* self)
{
    /*Static configuration */
    Ifx_Math_SpeedPreControl_F16_StaticConfig speedPreControlStaticConfig;

    /* Friction constant */
    speedPreControlStaticConfig.fricitionConstant = self->param->frictionConstant;

    /* Get the gain J/Ts in Q format */
    speedPreControlStaticConfig.rotorInertiaOverSamplingTime = Ifx_Math_ConvToQForm_F16(
        self->param->rotorInertiaOverSamplingTime, IFX_MS_FOCSOLUTION_F16_DEC_PLACES);

    /* Call init */
    Ifx_Math_SpeedPreControl_F16_init(&(self->speedPreControl), speedPreControlStaticConfig);

    /* set Inverse torque ct. */
    Ifx_Math_SpeedPreControl_F16_setInverseTorqueConstant(&(self->speedPreControl),
        self->param->inverseTorqueConstant);
}


static inline void Ifx_MS_FocSolution_F16_initRefCurrentLimiter(Ifx_MS_FocSolution_F16* self)
{
    /* Set upper limit of reference current limiter */
    Ifx_Math_Limit_F16_setUpperLimit(&(self->p_refCurrentLimit), self->param->refCurrentUpperLimitQ15);

    /* Set lower limit of reference current limiter */
    Ifx_Math_Limit_F16_setLowerLimit(&(self->p_refCurrentLimit), self->param->refCurrentLowerLimitQ15);
}


void Ifx_MS_FocSolution_F16_setRefSpeedLowerLimit(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                  refSpeedLowerLimitQ15)
{
    /* Set internal parameter */
    self->param->refSpeedLowerLimitQ15 = refSpeedLowerLimitQ15;

    /* Set lower limit of reference speed limiter */
    Ifx_Math_Limit_F16_setLowerLimit(&(self->speedLimit), self->param->refSpeedLowerLimitQ15);
}


void Ifx_MS_FocSolution_F16_setRefSpeedUpperLimit(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                  refSpeedUpperLimitQ15)
{
    /* Set internal parameter */
    self->param->refSpeedUpperLimitQ15 = refSpeedUpperLimitQ15;

    /* Set upper limit of reference speed limiter */
    Ifx_Math_Limit_F16_setUpperLimit(&(self->speedLimit), self->param->refSpeedUpperLimitQ15);
}


Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_getRefSpeedUpperLimit(Ifx_MS_FocSolution_F16* self)
{
    return self->param->refSpeedUpperLimitQ15;
}


Ifx_Math_Fract16 Ifx_MS_FocSolution_F16_getRefSpeedLowerLimit(Ifx_MS_FocSolution_F16* self)
{
    return self->param->refSpeedLowerLimitQ15;
}


static inline void Ifx_MS_FocSolution_F16_initDriveAlgo(Ifx_MS_FocSolution_F16                 * self,
                                                        Ifx_MS_FocSolution_F16_ParamComposition* paramComposition)
{
    /* Init FluxEstimator */
    Ifx_MDA_FluxEstimator_F16_init(&(self->fluxEstimator), paramComposition->fluxEstimatorParam);

    /* Init IToFController */
    Ifx_MDA_IToFController_F16_init(&(self->iToF), paramComposition->iToFControllerParam);

    /* Set IToFController parameters */
    Ifx_MDA_IToFController_F16_setReferenceCurrentImag(&(self->iToF), self->param->startUpCurrentQ15);

    /* Init FocController */
    Ifx_MDA_FocController_F16_init(&(self->focController), paramComposition->focControllerParam);

    /* Init VToFController */
    Ifx_MDA_VToFController_F16_init(&(self->vToF), paramComposition->vToFControllerParam);

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1
    Ifx_MDA_StartAngleIdent_F16_init(&(self->startAngleIdent));
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
}


static inline void Ifx_MS_FocSolution_F16_initSpeedAccelerationLimiters(Ifx_MS_FocSolution_F16* self)
{
    /* Speed limiter */
    Ifx_Math_Limit_F16_setLowerLimit(&(self->speedLimit), self->param->refSpeedLowerLimitQ15);
    Ifx_Math_Limit_F16_setUpperLimit(&(self->speedLimit), self->param->refSpeedUpperLimitQ15);

    /* Acceleration limiter */
    Ifx_Math_AccelLimit_F16_init(&(self->accelerationLimit));
    Ifx_Math_AccelLimit_F16_setSpeedStepUpLimit(&(self->accelerationLimit), self->param->speedRampUpRateOpenLoopQ30);
    Ifx_Math_AccelLimit_F16_setSpeedStepDownLimit(&(self->accelerationLimit),
        self->param->speedRampDownRateOpenLoopQ30);
}


static inline void Ifx_MS_FocSolution_F16_initStartCurrentRateLimiter(Ifx_MS_FocSolution_F16* self)
{
    /* Initialize rate limiter */
    Ifx_Math_RateLimit_F16_init(&(self->p_startCurrentRateLimit));

    /* Set rate limiter up rate */
    Ifx_Math_RateLimit_F16_setUpRate(&(self->p_startCurrentRateLimit), self->param->startUpCurrentRampUpRateQ15);

    /* Set rate limiter down rate */
    Ifx_Math_RateLimit_F16_setDownRate(&(self->p_startCurrentRateLimit), self->param->startUpCurrentRampUpRateQ15);
}


static inline void Ifx_MS_FocSolution_F16_initSpeedPi(Ifx_MS_FocSolution_F16* self)
{
    /* Get the gain in Q format */
    Ifx_Math_Fract16Q        propGain                          = Ifx_Math_ConvToQForm_F16(
        self->param->speedPiPropGain, IFX_MS_FOCSOLUTION_F16_DEC_PLACES);
    Ifx_Math_Fract16Q        integGainSamplingTime             = Ifx_Math_ConvToQForm_F16(
        self->param->speedPiIntegGainSamplingTime, IFX_MS_FOCSOLUTION_F16_DEC_PLACES);
    Ifx_Math_Fract16Q        speedPiAntiWindupGainSamplingTime = Ifx_Math_ConvToQForm_F16(
        self->param->speedPiAntiWindupGainSamplingTime, IFX_MS_FOCSOLUTION_F16_DEC_PLACES);

    /* Set Q formats */
    Ifx_Math_Pi_F16_Qformats piCtrlSpeedQform = {
        .qFormatError = Ifx_Math_FractQFormat_q14, .qFormatOutput = Ifx_Math_FractQFormat_q15
    };

    /* Call init */
    Ifx_Math_Pi_F16_init(&(self->speedPi), piCtrlSpeedQform);

    /* Call setters */
    Ifx_Math_Pi_F16_setPropGain(&(self->speedPi), propGain);
    Ifx_Math_Pi_F16_setIntegGainSamplingTime(&(self->speedPi), integGainSamplingTime);
    Ifx_Math_Pi_F16_setAntiWindupGainSamplingTime(&(self->speedPi), speedPiAntiWindupGainSamplingTime);
    Ifx_Math_Pi_F16_setUpperLimit(&(self->speedPi), self->param->refCurrentUpperLimitQ15);
    Ifx_Math_Pi_F16_setLowerLimit(&(self->speedPi), self->param->refCurrentLowerLimitQ15);
}


static void Ifx_MS_FocSolution_F16_localEnable(Ifx_MS_FocSolution_F16* self, bool enable)
{
    /* Enable/disable driverGroup */
    Ifx_MHA_DriverGroup_F16_enableModules(&(self->driverGroup), enable);

    /* Enable/disable modulator */
    Ifx_MAS_Modulator_F16_enable(&(self->modulator), enable);
}


static inline bool Ifx_MS_FocSolution_F16_checkModulesStateOn(Ifx_MS_FocSolution_F16* self)
{
    /* Variable declaration */
    Ifx_MAS_Modulator_F16_Status modulatorStatus;
    bool                         returnValue;

    /* Get the status of modulator */
    modulatorStatus = Ifx_MAS_Modulator_F16_getStatus(&self->modulator);

    /* Check if all modules are in the on state */
    if (Ifx_MHA_DriverGroup_F16_checkStateOn(&(self->driverGroup))
        && (modulatorStatus.state == Ifx_MAS_Modulator_F16_State_on))
    {
        returnValue = true;
    }
    else
    {
        returnValue = false;
    }

    return returnValue;
}


static inline bool Ifx_MS_FocSolution_F16_checkModulesStateOff(Ifx_MS_FocSolution_F16* self)
{
    /* Variable declaration */
    Ifx_MAS_Modulator_F16_Status modulatorStatus;
    bool                         returnValue;

    /* Get the status of modulator */
    modulatorStatus = Ifx_MAS_Modulator_F16_getStatus(&self->modulator);

    /* Check if all modules are in the off state */
    if (Ifx_MHA_DriverGroup_F16_checkStateOff(&(self->driverGroup))
        && (modulatorStatus.state == Ifx_MAS_Modulator_F16_State_off))
    {
        returnValue = true;
    }
    else
    {
        returnValue = false;
    }

    return returnValue;
}


/* API to set the fault status of the all used modules */
/* fault check for all the modules */
static bool Ifx_MS_FocSolution_F16_faultStatus(Ifx_MS_FocSolution_F16* self)
{
    /* local variable to check fault status */
    bool faultStatusRet = false;

    /* DriverGroup faults */
    faultStatusRet = Ifx_MHA_DriverGroup_F16_checkFaults(&(self->driverGroup));

    /* Modulator fault */
    if (Ifx_MAS_Modulator_F16_getStatus(&(self->modulator)).state == Ifx_MAS_Modulator_F16_State_fault)
    {
        faultStatusRet = true;
    }

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1

    /* Start angle identification fault */
    else if (Ifx_MDA_StartAngleIdent_F16_getStatus(&(self->startAngleIdent)).state ==
             Ifx_MDA_StartAngleIdent_F16_State_fault)
    {
        faultStatusRet = true;
    }
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
    else
    {
        /* No operation */
    }

    /* Return the fault status */
    return faultStatusRet;
}


/* FOC state machine */
static inline void Ifx_MS_FocSolution_F16_stateMachine(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 speedQ15,
                                                       Ifx_Math_CmpFract16 currentsDqRef)
{
    /* Variable to store the state */
    Ifx_MS_FocSolution_F16_State previousState = self->p_status.state;
    Ifx_MS_FocSolution_F16_State nextState;
    bool                         faultStatus   = Ifx_MS_FocSolution_F16_faultStatus(self);

    switch (previousState)
    {
        /* Initialize the module */
        case Ifx_MS_FocSolution_F16_State_init:
            nextState = Ifx_MS_FocSolution_F16_stateInit(self);
            break;

        /* All the modules are enabled */
        case Ifx_MS_FocSolution_F16_State_standBy:
            nextState = Ifx_MS_FocSolution_F16_stateStandby(self, faultStatus);
            break;

        /* Run state */
        case Ifx_MS_FocSolution_F16_State_run:
            nextState = Ifx_MS_FocSolution_F16_stateRun(self, speedQ15, faultStatus, currentsDqRef);
            break;

        /* FOC not running */
        case Ifx_MS_FocSolution_F16_State_off:
            nextState = Ifx_MS_FocSolution_F16_stateOff(self, faultStatus);
            break;

        /* FOC is in fault */
        case Ifx_MS_FocSolution_F16_State_fault:
            nextState = Ifx_MS_FocSolution_F16_stateFault(self);
            break;

        /* FOC is in ramp down state */
        case Ifx_MS_FocSolution_F16_State_rampDown:
            nextState = Ifx_MS_FocSolution_F16_stateRampDown(self, faultStatus, currentsDqRef);
            break;

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1

        /*  FOC is in start angle identification state */
        case Ifx_MS_FocSolution_F16_State_startAngleIdent:
            nextState = Ifx_MS_FocSolution_F16_stateStartAngleIdent(self, speedQ15, faultStatus);
            break;

#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
        /* do default transition to INIT */
        default:
            nextState = Ifx_MS_FocSolution_F16_State_init;
            break;
    }

    self->p_status.state = nextState;
}


static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateInit(Ifx_MS_FocSolution_F16* self)
{
    Ifx_MS_FocSolution_F16_State nextState;

    /* Check if all the modules are in OFF state */
    if (Ifx_MS_FocSolution_F16_checkModulesStateOff(self) == true)
    {
        /* Set the state to OFF */
        nextState = Ifx_MS_FocSolution_F16_State_off;
    }
    else
    {
        nextState = Ifx_MS_FocSolution_F16_State_init;
    }

    return nextState;
}


static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateOff(Ifx_MS_FocSolution_F16* self, bool
                                                                           faultStatus)
{
    Ifx_MS_FocSolution_F16_State nextState;

    /* Reset previous values */
    Ifx_MS_FocSolution_F16_reset(self);

    if (faultStatus == true)
    {
        nextState = Ifx_MS_FocSolution_F16_State_fault;
    }

    /* Check if power stage is enabled */
    else if (self->p_enablePowerStage == true)
    {
        /* Enable underlying modules */
        Ifx_MS_FocSolution_F16_localEnable(self, true);

        /* Check if underlying modules are in the on state */
        if (Ifx_MS_FocSolution_F16_checkModulesStateOn(self) == true)
        {
            nextState = Ifx_MS_FocSolution_F16_State_standBy;
        }
        else
        {
            nextState = Ifx_MS_FocSolution_F16_State_off;
        }
    }
    else
    {
        nextState = Ifx_MS_FocSolution_F16_State_off;
    }

    return nextState;
}


static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateStandby(Ifx_MS_FocSolution_F16* self, bool
                                                                               faultStatus)
{
    Ifx_MS_FocSolution_F16_State nextState;

    if (faultStatus == true)
    {
        /* Go into fault state */
        nextState = Ifx_MS_FocSolution_F16_State_fault;
    }
    else if (self->p_enablePowerStage == false)
    {
        nextState = Ifx_MS_FocSolution_F16_stateStandbyToOff(self);
    }

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1
    else if (self->p_enableStartAngleIdent == true)
    {
        /* Enable start angle identification module */
        Ifx_MDA_StartAngleIdent_F16_enable(&(self->startAngleIdent), true);

        /* Go to start angle identification state */
        nextState = Ifx_MS_FocSolution_F16_State_startAngleIdent;
    }
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
    else if (self->p_enableControl == true)
    {
        nextState = Ifx_MS_FocSolution_F16_stateStandbyToRun(self);
    }
    else
    {
        nextState = Ifx_MS_FocSolution_F16_State_standBy;
    }

    /* Always reset FocCtrl module in standby */
    Ifx_MS_FocSolution_F16_reset(self);

    return nextState;
}


static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateStandbyToOff(Ifx_MS_FocSolution_F16* self)
{
    Ifx_MS_FocSolution_F16_State nextState;

    /* Disable all underlying modules */
    Ifx_MS_FocSolution_F16_localEnable(self, false);

    /* Check if underlying modules are in the off state */
    if (Ifx_MS_FocSolution_F16_checkModulesStateOff(self) == true)
    {
        nextState = Ifx_MS_FocSolution_F16_State_off;
    }
    else
    {
        nextState = Ifx_MS_FocSolution_F16_State_standBy;
    }

    return nextState;
}


static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateStandbyToRun(Ifx_MS_FocSolution_F16* self)
{
    Ifx_MS_FocSolution_F16_State nextState;

    /* Update control mode */
    self->p_status.actualControlMode = Ifx_MS_FocSolution_F16_getControlMode(self);

    /* Set open loop ramp up/down rates */
    Ifx_Math_AccelLimit_F16_setSpeedStepUpLimit(&(self->accelerationLimit), self->param->speedRampUpRateOpenLoopQ30);
    Ifx_Math_AccelLimit_F16_setSpeedStepDownLimit(&(self->accelerationLimit),
        self->param->speedRampDownRateOpenLoopQ30);

    /* Init. substate machine */
    self->p_status.subState = Ifx_MS_FocSolution_F16_SubState_openLoop;

    /* Enable flux estimator */
    Ifx_MDA_FluxEstimator_F16_configMode(&(self->fluxEstimator), Ifx_MDA_FluxEstimator_F16_Mode_enable);

    /* Go to run state */
    nextState = Ifx_MS_FocSolution_F16_State_run;

    return nextState;
}


static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateRun(Ifx_MS_FocSolution_F16* self,
                                                                           Ifx_Math_Fract16 speedQ15, bool
                                                                           faultStatus, Ifx_Math_CmpFract16
                                                                           currentsDqRef)
{
    Ifx_MS_FocSolution_F16_State nextState;

    /* Perform speed and acceleration limit */
    Ifx_MS_FocSolution_F16_limitSpeed(self, speedQ15);

    if (faultStatus == true)
    {
        /* Disable flux estimator */
        Ifx_MDA_FluxEstimator_F16_configMode(&(self->fluxEstimator), Ifx_MDA_FluxEstimator_F16_Mode_disable);

        /* Set the module to fault */
        nextState = Ifx_MS_FocSolution_F16_State_fault;
    }
    else if ((self->p_enableControl == false)
             || (self->p_enablePowerStage == false))
    {
#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)

        /* Reset transition counter */
        self->p_transitionCounter_cycles = 0U;
#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */
        /* Set state to ramp down */
        nextState = Ifx_MS_FocSolution_F16_State_rampDown;
    }
    else
    {
        /* Execute sub-state machine */
        Ifx_MS_FocSolution_F16_subStateMachine(self, currentsDqRef);

        /* Stay in run */
        nextState = Ifx_MS_FocSolution_F16_State_run;
    }

    return nextState;
}


static inline void Ifx_MS_FocSolution_F16_limitSpeed(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 speedQ15)
{
    /* Value limited speed */
    Ifx_Math_Fract16 limSpeed;

    /* Limit input speed */
    limSpeed = Ifx_Math_Limit_F16_execute(&(self->speedLimit), speedQ15);

    /* Limit acceleration */
    self->rateLimitInSpeedQ15 = Ifx_Math_AccelLimit_F16_execute(&(self->accelerationLimit), limSpeed);
}


static inline void Ifx_MS_FocSolution_F16_subStateMachine(Ifx_MS_FocSolution_F16* self, Ifx_Math_CmpFract16
                                                          currentsDqRef)
{
    /* Variable to store the state */
    Ifx_MS_FocSolution_F16_SubState   previousSubState = self->p_status.subState;
    Ifx_MS_FocSolution_F16_SubState   nextSubState;
    Ifx_MDA_IToFController_F16_Output iToFOutput;
    Ifx_MDA_IToFController_F16_getOutput(&(self->iToF), &iToFOutput);

    switch (previousSubState)
    {
        /* Sub state machine run is in open loop */
        case Ifx_MS_FocSolution_F16_SubState_openLoop:
            nextSubState = Ifx_MS_FocSolution_F16_subStateOpenLoop(self, iToFOutput, currentsDqRef);
            break;

#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)

        /* Sub state machine run is in transition up */
        case Ifx_MS_FocSolution_F16_SubState_transitionUp:
            nextSubState = Ifx_MS_FocSolution_F16_subStateTransitUp(self, iToFOutput, currentsDqRef);
            break;

#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */
        /* Sub state machine run is in closed loop */
        case Ifx_MS_FocSolution_F16_SubState_closedLoop:
            nextSubState = Ifx_MS_FocSolution_F16_subStateClosedLoop(self, currentsDqRef);
            break;

#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)

        /* Sub state machine run is in transition down */
        case Ifx_MS_FocSolution_F16_SubState_transitionDown:
            nextSubState = Ifx_MS_FocSolution_F16_subStateTransitDown(self, iToFOutput, currentsDqRef);
            break;

#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */
        /* do default transition to open loop */
        default:
            nextSubState = Ifx_MS_FocSolution_F16_SubState_openLoop;
            break;
    }

    self->p_status.subState = nextSubState;
}


static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateOpenLoop(Ifx_MS_FocSolution_F16* self,
                                                                                      Ifx_MDA_IToFController_F16_Output
                                                                                      iToFOutput, Ifx_Math_CmpFract16
                                                                                      currentsDqRef)
{
    Ifx_MS_FocSolution_F16_SubState nextSubState;

    /* Check if the direct current interface is enabled or disabled */
    if (self->p_enableDirectInterface == true)
    {
        self->dqCommand.real = currentsDqRef.real;
        self->dqCommand.imag = currentsDqRef.imag;
    }
    else
    {
        /* Set dq ref current and iToF angle */
        Ifx_MS_FocSolution_F16_setDqCommand(self, iToFOutput);
    }

    /* Check the ref. speed to go to transition up or stay in open loop */
    if ((Ifx_Math_Abs_F16(self->rateLimitInSpeedQ15) > self->param->transitionSpeedUpQ15)
        && (self->p_status.actualControlMode == Ifx_MS_FocSolution_F16_ControlMode_foc))
    {
#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)

        /* Init. transition counter */
        self->p_transitionCounter_cycles = 0U;

        /* Go to transition up */
        nextSubState = Ifx_MS_FocSolution_F16_SubState_transitionUp;
#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */

#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_DIRECT_TRANSITION)

        /* Set the integral previous value to the configured transition q current */
        if (self->rateLimitInSpeedQ15 > 0)
        {
            Ifx_Math_Pi_F16_setIntegPreviousValue(&(self->speedPi), self->param->qCurrentAtTransitionQ15);
        }
        else
        {
            Ifx_Math_Pi_F16_setIntegPreviousValue(&(self->speedPi), Ifx_Math_Neg_F16(
                self->param->qCurrentAtTransitionQ15));
        }

        /* Set closed loop ramp up/down rates */
        Ifx_Math_AccelLimit_F16_setSpeedStepUpLimit(&(self->accelerationLimit),
            self->param->speedRampUpRateClosedLoopQ30);
        Ifx_Math_AccelLimit_F16_setSpeedStepDownLimit(&(self->accelerationLimit),
            self->param->speedRampDownRateClosedLoopQ30);

        /* Go to closed loop */
        nextSubState = Ifx_MS_FocSolution_F16_SubState_closedLoop;
#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_DIRECT_TRANSITION) */
    }
    else
    {
        /* Stay in open loop */
        nextSubState = Ifx_MS_FocSolution_F16_SubState_openLoop;
    }

    return nextSubState;
}


static inline void Ifx_MS_FocSolution_F16_setDqCommand(Ifx_MS_FocSolution_F16* self, Ifx_MDA_IToFController_F16_Output
                                                       iToFOutput)
{
    /* Set ref. d current to I2f */
    self->dqCommand.real = iToFOutput.refCurrent.real;

    /* Set ref. q current to I2f depending on current and previous reference speed */
    Ifx_MS_FocSolution_F16_setQCommand(self, iToFOutput);

    /* Update previous ref. current */
    self->p_previousQCommand = self->dqCommand.imag;
}


static inline void Ifx_MS_FocSolution_F16_setQCommand(Ifx_MS_FocSolution_F16* self, Ifx_MDA_IToFController_F16_Output
                                                      iToFOutput)
{
    /* Effective Q command current */
    Ifx_Math_Fract16 effectiveRefQCurrent;
    effectiveRefQCurrent = Ifx_Math_RateLimit_F16_execute(&(self->p_startCurrentRateLimit),
        iToFOutput.refCurrent.imag);

    /* Set ref. q current to I2f depending on current and previous reference speed */
    if (self->rateLimitInSpeedQ15 >= 0)
    {
        self->dqCommand.imag = effectiveRefQCurrent;
    }
    else
    {
        self->dqCommand.imag = Ifx_Math_Neg_F16(effectiveRefQCurrent);
    }

    /* Detect if the current Q command changed sign */
    Ifx_MS_FocSolution_F16_detectQCommandZeroCross(self);
}


static inline void Ifx_MS_FocSolution_F16_detectQCommandZeroCross(Ifx_MS_FocSolution_F16* self)
{
    if (self->p_previousQCommand < 0)
    {
        if (self->dqCommand.imag > 0)
        {
            self->p_qCommandZeroCrossing = true;
        }
    }

    /* self->p_previousQCommand >= 0 */
    else
    {
        if (self->dqCommand.imag < 0)
        {
            self->p_qCommandZeroCrossing = true;
        }
    }
}


#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)
static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateTransitUp(Ifx_MS_FocSolution_F16* self,
                                                                                       Ifx_MDA_IToFController_F16_Output
                                                                                       iToFOutput, Ifx_Math_CmpFract16
                                                                                       currentsDqRef)
{
    Ifx_MS_FocSolution_F16_SubState nextSubState;

    /* Run smooth transition */
    bool                            mergeAngle;
    mergeAngle = Ifx_MS_FocSolution_F16_angleMergeCheck(self, iToFOutput, currentsDqRef);

    if (mergeAngle == true)
    {
        nextSubState = Ifx_MS_FocSolution_F16_subStateTransitUpExit(self, self->p_output.estimatedSpeedQ15,
            currentsDqRef);
    }
    else
    {
        nextSubState = Ifx_MS_FocSolution_F16_SubState_transitionUp;
    }

    return nextSubState;
}


static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateTransitUpExit(
    Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 estimatedSpeedQ15, Ifx_Math_CmpFract16 currentsDqRef)
{
    /* Calculate ref. Q current */
    self->dqCommand.imag = Ifx_MS_FocSolution_F16_currentQRef(self, estimatedSpeedQ15, currentsDqRef.imag);

    /* Set closed loop ramp up/down rates  */
    Ifx_Math_AccelLimit_F16_setSpeedStepUpLimit(&(self->accelerationLimit),
        self->param->speedRampUpRateClosedLoopQ30);
    Ifx_Math_AccelLimit_F16_setSpeedStepDownLimit(&(self->accelerationLimit),
        self->param->speedRampDownRateClosedLoopQ30);

    /* Go to closed loop */
    return Ifx_MS_FocSolution_F16_SubState_closedLoop;
}


static inline bool Ifx_MS_FocSolution_F16_angleMergeCheck(Ifx_MS_FocSolution_F16          * self,
                                                          Ifx_MDA_IToFController_F16_Output iToFOutput,
                                                          Ifx_Math_CmpFract16               currentsDqRef)
{
    /* Angle merge indication */
    bool    mergeAngle = false;

    /* Angle error between iToF and flux estimator */
    int32_t angleError = Ifx_MS_FocSolution_F16_angleError(self, iToFOutput);

    /* Check angle error minimum value and transition time */
    if ((Ifx_Math_Abs_F32(angleError) < self->param->transitionAngleTolerance)
        || (self->p_transitionCounter_cycles >= self->param->transitionTimeLimit_cycles - 1))
    {
        /* Set the integral previous value to the last reached ref. q current */
        Ifx_Math_Pi_F16_setIntegPreviousValue(&(self->speedPi), self->dqCommand.imag);

        /* Set angle merge flag to true */
        mergeAngle = true;
    }

    /* Decrease ref. q current amplitude */
    else
    {
        /* Check if the direct current interface is enabled or disabled */
        if (self->p_enableDirectInterface == true)
        {
            self->dqCommand.real = currentsDqRef.real;
            self->dqCommand.imag = currentsDqRef.imag;
        }
        else
        {
            Ifx_MS_FocSolution_F16_decreaseQCurrent(self, iToFOutput);
        }
    }

    return mergeAngle;
}


static inline int32_t Ifx_MS_FocSolution_F16_angleError(Ifx_MS_FocSolution_F16          * self,
                                                        Ifx_MDA_IToFController_F16_Output iToFOutput)
{
    /* Get flux estimator output */
    Ifx_MDA_FluxEstimator_F16_Output fluxEstimatorOutput;
    Ifx_MDA_FluxEstimator_F16_getOutput(&(self->fluxEstimator), &fluxEstimatorOutput);

    /* I2f angle in uint16_t */
    uint16_t                         refAngle = (uint16_t)(iToFOutput.currentVecAngle_rad >> 16);

    /* FE angle in uint16_t */
    uint16_t                         estAngle = (uint16_t)(fluxEstimatorOutput.anglePLL >> 16);

    /* Angle error in int32_t */
    int32_t                          angleErr = (int32_t)refAngle - (int32_t)estAngle;

    /* Angle overflowed */
    int32_t                          outputAngle;

    /* Check if error > pi */
    if (angleErr > IFX_MATH_FRACT16_MAX)
    {
        outputAngle = Ifx_Math_Sub_F32(angleErr, IFX_MS_FOCSOLUTION_F16_2MAX);
    }

    /* Check if error < -pi */
    else if (angleErr < IFX_MS_FOCSOLUTION_F16_PINEG)
    {
        /* -1 is due to the asymmetry of the range on the negative (-32768) and positive (32767) limits */
        outputAngle = (Ifx_Math_Add_F32(angleErr, IFX_MS_FOCSOLUTION_F16_2MAX) - 1);
    }
    else
    {
        /* No wrapping needed, return the input */
        outputAngle = angleErr;
    }

    return outputAngle;
}


#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */
static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateClosedLoop(Ifx_MS_FocSolution_F16* self,
                                                                                        Ifx_Math_CmpFract16
                                                                                        currentsDqRef)
{
    Ifx_MS_FocSolution_F16_SubState nextSubState = Ifx_MS_FocSolution_F16_SubState_closedLoop;

    /* Store estimated speed and absolute estimated speed in local variables for further processing */
    Ifx_Math_Fract16                estimatedSpeedQ15    = self->p_output.estimatedSpeedQ15;
    Ifx_Math_Fract16                estimatedSpeedAbsQ15 = Ifx_Math_AbsSat_F16(estimatedSpeedQ15);

    /* Calculate ref. Q current */
    self->dqCommand.imag = Ifx_MS_FocSolution_F16_currentQRef(self, estimatedSpeedQ15, currentsDqRef.imag);

    if (estimatedSpeedAbsQ15 < self->transitionSpeedDownQ15)
    {
        /* Perform all required actions for exit from closedLoop and determine next subState */
        nextSubState = Ifx_MS_FocSolution_F16_subStateClosedLoopExit(self);
    }

#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)

    else if (estimatedSpeedAbsQ15 < self->transitionSpeedMidQ15)
    {
        Ifx_MS_FocSolution_F16_smoothTransitionDownCalc(self, estimatedSpeedAbsQ15);
    }

#endif /*(IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */
    else
    {
        /* Set the direct real reference current (ready for field weakening) */
        self->dqCommand.real = currentsDqRef.real;
    }

    /* Return the substate for the next cycle */
    return nextSubState;
}


static inline Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateClosedLoopExit(
    Ifx_MS_FocSolution_F16* self)
{
    Ifx_MS_FocSolution_F16_SubState  nextSubState;
    Ifx_Math_Fract16                 estimatedSpeedQ15 = self->p_output.estimatedSpeedQ15;

    /* Get flux estimator output */
    Ifx_MDA_FluxEstimator_F16_Output fluxEstimator;
    Ifx_MDA_FluxEstimator_F16_getOutput(&(self->fluxEstimator), &fluxEstimator);

    /* Reset the iToF previous angle to last flux estimator angle */
    Ifx_MDA_IToFController_F16_setAnglePreviousValue(&(self->iToF), fluxEstimator.anglePLL);

    /* Reset the acceleration limit state to the last estimated speed */
    Ifx_Math_AccelLimit_F16_setSpeedStepPreviousValue(&(self->accelerationLimit), estimatedSpeedQ15);

    /* Set open loop ramp up/down rates */
    Ifx_Math_AccelLimit_F16_setSpeedStepUpLimit(&(self->accelerationLimit), self->param->speedRampUpRateOpenLoopQ30);
    Ifx_Math_AccelLimit_F16_setSpeedStepDownLimit(&(self->accelerationLimit),
        self->param->speedRampDownRateOpenLoopQ30);

    /* Store last ref. q current to calculate the slope in subStateTransitDown */
    self->p_previousQCommand = self->dqCommand.imag;

#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)

    /* Reset transition counter */
    self->p_transitionCounter_cycles = 0U;
    nextSubState                     = Ifx_MS_FocSolution_F16_SubState_transitionDown;
#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */

#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_DIRECT_TRANSITION)

    nextSubState = Ifx_MS_FocSolution_F16_SubState_openLoop;
#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_DIRECT_TRANSITION) */

    return nextSubState;
}


static inline void Ifx_MS_FocSolution_F16_smoothTransitionDownCalc(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                   estimatedSpeedAbsQ15)
{
    /* Calculate the rate of the target d current based on the absolute estimated speed. The rate increases from 0.0 to
     * 1.0 as the speed decreases from the mid point between transitionSpeedUp and transitionSpeedDown
     * (transitionSpeedMid) to transitionSpeedDown. */
    Ifx_Math_Fract16 speedDifferenceQ15 = self->transitionSpeedMidQ15 - estimatedSpeedAbsQ15;
    Ifx_Math_Fract16 rate               = Ifx_Math_DivSat_F16(speedDifferenceQ15, self->p_transitionSpeedBandHalfQ15);

    /* Calculate Id */
    self->dqCommand.real = Ifx_Math_Mul_F16(rate, self->p_transitionDownTargetDCurrentQ15);
}


#if (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION)

/* *INDENT-OFF* */
static inline
Ifx_MS_FocSolution_F16_SubState Ifx_MS_FocSolution_F16_subStateTransitDown(Ifx_MS_FocSolution_F16* self,
                                                                           Ifx_MDA_IToFController_F16_Output iToFOutput,
                                                                           Ifx_Math_CmpFract16 currentsDqRef)
{
    Ifx_MS_FocSolution_F16_SubState nextSubState;

    /* Calculate the slope of the transition down based on the elapsed transition cycles */
    Ifx_Math_Fract16 transitionCounterQ15 = Ifx_Math_MulShR_F16((Ifx_Math_Fract16)self->p_transitionCounter_cycles,
        self->p_transitionDeltaQ15, 0U);

    if (self->p_transitionCounter_cycles < (self->param->transitionTimeLimit_cycles - 1U))
    {
        /* Increase amplitude of ref. Q current */
        Ifx_MS_FocSolution_F16_increaseQCurrent(self, iToFOutput, transitionCounterQ15);

        /* Decrease amplitude of ref. D current */
        Ifx_MS_FocSolution_F16_decreaseDCurrent(self, currentsDqRef.real, transitionCounterQ15);

        /* Increment transition counter */
        self->p_transitionCounter_cycles = self->p_transitionCounter_cycles + 1U;

        /* Stay in transition down */
        nextSubState = Ifx_MS_FocSolution_F16_SubState_transitionDown;
    }
    else
    {
        /* Store last ref. q current to check for addition of PI */
        self->p_previousQCommand = self->dqCommand.imag;

        /* Go to open loop */
        nextSubState = Ifx_MS_FocSolution_F16_SubState_openLoop;
    }

    return nextSubState;
}
/* *INDENT-ON* */
static inline void Ifx_MS_FocSolution_F16_increaseQCurrent(Ifx_MS_FocSolution_F16                * self,
                                                           Ifx_MDA_IToFController_F16_Output const iToFOutput,
                                                           Ifx_Math_Fract16 const
                                                           transitionCounterQ15)
{
    /* Increase amplitude of positive imaginary ref. current at slope defined */
    if (self->rateLimitInSpeedQ15 >= 0)
    {
        /* Store delta between i2f q current and last ref. q current from PI based on sign ref. speed */
        Ifx_Math_Fract16 currDeltaQ15 = Ifx_Math_SubSat_F16(iToFOutput.refCurrent.imag, self->p_previousQCommand);

        /* Increase amplitude of ref. q current */
        self->dqCommand.imag = Ifx_Math_AddSat_F16(self->p_previousQCommand, Ifx_Math_Mul_F16(transitionCounterQ15,
            currDeltaQ15));
    }

    /* Increase amplitude of negative imaginary ref. current at slope defined */
    else
    {
        /* Store delta between i2f q current and last ref. q current from PI based on sign ref. speed */
        Ifx_Math_Fract16 currDeltaQ15 = Ifx_Math_AddSat_F16(iToFOutput.refCurrent.imag, self->p_previousQCommand);

        /* Increase amplitude of ref. q current */
        self->dqCommand.imag = Ifx_Math_SubSat_F16(self->p_previousQCommand, Ifx_Math_Mul_F16(transitionCounterQ15,
            currDeltaQ15));
    }
}


static inline void Ifx_MS_FocSolution_F16_decreaseQCurrent(Ifx_MS_FocSolution_F16                * self,
                                                           Ifx_MDA_IToFController_F16_Output const iToFOutput)
{
    /* Increment transition counter */
    self->p_transitionCounter_cycles = self->p_transitionCounter_cycles + 1u;

    /* Slope to increase or decrease the currents */
    Ifx_Math_Fract16 slope = Ifx_Math_DivShLSat_F16((Ifx_Math_Fract16)self->p_transitionCounter_cycles,
        (Ifx_Math_Fract16)self->param->transitionTimeLimit_cycles, 15U);

    /* Decrease amplitude of positive imaginary ref. current at slope defined */
    if (self->rateLimitInSpeedQ15 >= 0)
    {
        self->dqCommand.imag = Ifx_Math_SubSat_F16(iToFOutput.refCurrent.imag, Ifx_Math_Mul_F16(slope,
            iToFOutput.refCurrent.imag));
    }

    /* Decrease amplitude of negative imaginary ref. current at slope defined */
    else
    {
        self->dqCommand.imag = Ifx_Math_SubSat_F16(Ifx_Math_Mul_F16(slope, iToFOutput.refCurrent.imag),
            iToFOutput.refCurrent.imag);
    }
}


static inline void Ifx_MS_FocSolution_F16_decreaseDCurrent(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 const
                                                           currentDRefQ15, Ifx_Math_Fract16 const
                                                           transitionCounterQ15)
{
    /* Calculate difference between target D current during transition down and reference D current */
    Ifx_Math_Fract16 currentDDiffQ15 = Ifx_Math_Sub_F16(self->p_transitionDownTargetDCurrentQ15, currentDRefQ15);

    /* Decrease Id from the target Id current for transition down to the input d command */
    self->dqCommand.real = Ifx_Math_Sub_F16(self->p_transitionDownTargetDCurrentQ15, Ifx_Math_Mul_F16(
        transitionCounterQ15, currentDDiffQ15));
}


#endif /* (IFX_MS_FOCSOLUTION_F16_CFG_TRANSITION_MODE == IFX_MS_FOCSOLUTION_F16_TRANSITION_MODE_SMOOTH_TRANSITION) */
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateFault(Ifx_MS_FocSolution_F16* self)
{
    Ifx_MS_FocSolution_F16_State nextState;
    bool                         faultCleared;

    /* Check whether clear fault state is requested */
    if (self->p_clearFaultIsRequested == true)
    {
        /* Check whether all faults in the underlying modules have been successfully cleared */
        faultCleared = Ifx_MS_FocSolution_F16_faultSuccessfullyCleared(self);

        if (faultCleared == true)
        {
            /* Disable underlying modules and go to off state */
            Ifx_MS_FocSolution_F16_localEnable(self, false);
            nextState = Ifx_MS_FocSolution_F16_State_off;
        }
        else
        {
            /* Fault of an underlying module has not been successfully cleared, stay in fault state */
            nextState = Ifx_MS_FocSolution_F16_State_fault;
        }
    }
    else
    {
        /* No clear fault requested, stay in fault state */
        nextState = Ifx_MS_FocSolution_F16_State_fault;
    }

    return nextState;
}


static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateRampDown(Ifx_MS_FocSolution_F16* self, bool
                                                                                faultStatus, Ifx_Math_CmpFract16
                                                                                currentsDqRef)
{
    Ifx_MS_FocSolution_F16_State nextState;

    /* Perform speed and acceleration limit */
    Ifx_MS_FocSolution_F16_limitSpeed(self, 0);

    if (faultStatus == true)
    {
        /* Disable flux estimator */
        Ifx_MDA_FluxEstimator_F16_configMode(&(self->fluxEstimator), Ifx_MDA_FluxEstimator_F16_Mode_disable);

        /* Go into fault state */
        nextState = Ifx_MS_FocSolution_F16_State_fault;
    }
    else if (Ifx_Math_Abs_F16(self->rateLimitInSpeedQ15) <= self->param->minimumSpeedThresholdQ15)
    {
        /* Disable flux estimator */
        Ifx_MDA_FluxEstimator_F16_configMode(&(self->fluxEstimator), Ifx_MDA_FluxEstimator_F16_Mode_disable);
        nextState = Ifx_MS_FocSolution_F16_State_standBy;
    }
    else
    {
        /* Execute sub-state machine */
        Ifx_MS_FocSolution_F16_subStateMachine(self, currentsDqRef);
        nextState = Ifx_MS_FocSolution_F16_State_rampDown;
    }

    return nextState;
}


static inline bool Ifx_MS_FocSolution_F16_anyClearFaultIsPending(Ifx_MS_FocSolution_F16* self)
{
    /* Holds the overall result of the underlying modules clearFaultIsPending */
    bool clearFaultIsPending = false;

    /* MHA modules fault is pending */
    clearFaultIsPending = Ifx_MHA_DriverGroup_F16_anyClearFaultIsPending(&(self->driverGroup));

    /* Modulator fault is pending */
    if (Ifx_MAS_Modulator_F16_clearFaultIsPending(&(self->modulator)) == true)
    {
        clearFaultIsPending = true;
    }

#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1

    /* StartAngleIdent fault is pending */
    else if (Ifx_MDA_StartAngleIdent_F16_clearFaultIsPending(&(self->startAngleIdent)) == true)
    {
        clearFaultIsPending = true;
    }
#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
    else
    {
        /* No operation */
    }

    /* Return whether clear fault of any underlying module is still pending */
    return clearFaultIsPending;
}


static inline bool Ifx_MS_FocSolution_F16_faultSuccessfullyCleared(Ifx_MS_FocSolution_F16* self)
{
    bool faultCleared;

    /* Check that none of the clear fault requests in the underlying modules are pending */
    if (Ifx_MS_FocSolution_F16_anyClearFaultIsPending(self) == false)
    {
        /* Check for faults in underlying modules */
        if (Ifx_MS_FocSolution_F16_faultStatus(self) == true)
        {
            faultCleared = false;
        }

        /* No faults detected any more, fault(s) successfully cleared */
        else
        {
            faultCleared = true;
        }

        /* Reset the clear fault requested flag */
        self->p_clearFaultIsRequested = false;
    }
    else
    {
        /* Clear fault has not finished execution in every module yet */
        faultCleared = false;
    }

    return faultCleared;
}


#if IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1
static inline Ifx_MS_FocSolution_F16_State Ifx_MS_FocSolution_F16_stateStartAngleIdent(Ifx_MS_FocSolution_F16* self,
                                                                                       Ifx_Math_Fract16 speedQ15, bool
                                                                                       faultStatus,
                                                                                       Ifx_MDA_IToFController_F16_Output
                                                                                       iToFOutput)
{
    Ifx_MS_FocSolution_F16_State       nextState;

    /* Get the status of the start angle estimation */
    Ifx_MDA_StartAngleIdent_F16_Status startAngleEstStatus = Ifx_MDA_StartAngleIdent_F16_getStatus(
        &(self->startAngleIdent));

    if (faultStatus == true)
    {
        /* Go to fault */
        nextState = Ifx_MS_FocSolution_F16_State_fault;
    }
    else if (startAngleEstStatus.state == Ifx_MDA_StartAngleIdent_F16_State_estimationReady)
    {
        /* Disable start angle ident. module */
        Ifx_MDA_StartAngleIdent_F16_enable(&(self->startAngleIdent), false);

        /* Stay in start angle ident. state */
        nextState = Ifx_MS_FocSolution_F16_State_startAngleIdent;
    }
    else if (startAngleEstStatus.state == Ifx_MDA_StartAngleIdent_F16_State_off)
    {
        /* Variable to store start angle identification output */
        Ifx_MDA_StartAngleIdent_F16_Output startAngleIdentOutput;

        /* Get start angle identification output */
        Ifx_MDA_StartAngleIdent_F16_getOutput(&(self->startAngleIdent), &startAngleIdentOutput);

        /* Init i2f angle prev value */
        Ifx_MS_FocSolution_F16_setAnglePreviousValue(self, speedQ15, startAngleIdentOutput);

        if (self->p_enableControl == true)
        {
            /* Update control mode */
            self->p_status.actualControlMode = Ifx_MS_FocSolution_F16_getControlMode(self);

            /* Set Q command current */
            self->dqCommand.imag = iToFOutput.refCurrent.imag;

            /* Set open loop ramp up/down rates */
            Ifx_Math_AccelLimit_F16_setSpeedStepUpLimit(&(self->accelerationLimit),
                self->param->speedRampUpRateOpenLoopQ30);
            Ifx_Math_AccelLimit_F16_setSpeedStepDownLimit(&(self->accelerationLimit),
                self->param->speedRampDownRateOpenLoopQ30);

            /* Init. substate machine */
            self->p_status.subState = Ifx_MS_FocSolution_F16_SubState_openLoop;

            /* Enable flux estimator */
            Ifx_MDA_FluxEstimator_F16_configMode(&(self->fluxEstimator), Ifx_MDA_FluxEstimator_F16_Mode_enable);

            /* If estimation is done and speed control is enabled, store estimated angle and go to run */
            nextState = Ifx_MS_FocSolution_F16_State_run;
        }
        else
        {
            nextState = Ifx_MS_FocSolution_F16_State_standBy;
        }
    }
    else
    {
        /* Stay in start angle ident. state */
        nextState = Ifx_MS_FocSolution_F16_State_startAngleIdent;
    }

    return nextState;
}


void Ifx_MS_FocSolution_F16_setAnglePreviousValue(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 speedQ15,
                                                  Ifx_MDA_StartAngleIdent_F16_Output startAngleIdentOutput)
{
    /* Init i2f angle prev value */
    if (speedQ15 > 0)
    {
        Ifx_MDA_IToFController_F16_setAnglePreviousValue(&(self->iToF), startAngleIdentOutput.estimatedAngle -
            IFX_MS_FOCSOLUTION_F16_PIBY2 + IFX_MS_FOCSOLUTION_F16_PIBY6);
    }
    else if (speedQ15 < 0)
    {
        Ifx_MDA_IToFController_F16_setAnglePreviousValue(&(self->iToF), startAngleIdentOutput.estimatedAngle -
            IFX_MS_FOCSOLUTION_F16_PIBY2 - IFX_MS_FOCSOLUTION_F16_PIBY6);
    }
    else
    {
        Ifx_MDA_IToFController_F16_setAnglePreviousValue(&(self->iToF), startAngleIdentOutput.estimatedAngle -
            IFX_MS_FOCSOLUTION_F16_PIBY2);
    }
}


#endif /* IFX_MS_FOCSOLUTION_F16_CFG_INCLUDE_STARTANGLE_IDENT == 1 */
static void Ifx_MS_FocSolution_F16_reset(Ifx_MS_FocSolution_F16* self)
{
    /* Flux estimator */
    Ifx_Math_LowPass1st_F16_setPreviousValue(&(self->fluxEstimator.p_alphaFilter), 0);
    Ifx_Math_LowPass1st_F16_setPreviousValue(&(self->fluxEstimator.p_betaFilter), 0);
    Ifx_Math_LowPass1st_F16_setPreviousValue(&(self->fluxEstimator.p_speedFilter), 0);
    Ifx_Math_PLL_F16_resetBuffer(&self->fluxEstimator.p_pllFilter);
    Ifx_Math_PLL_F16_setPreviousValue(&self->fluxEstimator.p_pllFilter, 0);

    /* PI controller */
    Ifx_Math_Pi_F16_setIntegPreviousValue(&(self->speedPi), 0);

    /* Field oriented controller */
    Ifx_MDA_FocController_F16_reset(&(self->focController));

    /* Acceleration limiter and I2f */
    Ifx_Math_AccelLimit_F16_setSpeedStepPreviousValue(&(self->accelerationLimit), 0);
    Ifx_Math_RateLimit_F16_setPreviousValue(&(self->p_startCurrentRateLimit), self->param->initStartupCurrentQ15);

    /* Set the initial startup current to the statically configured value */
    self->dqCommand.imag = self->param->initStartupCurrentQ15;

    /* Speed and sector number */
    self->rateLimitInSpeedQ15                      = 0;
    self->previousCurrentReconstructionInfo.sector = 0;
}


void Ifx_MS_FocSolution_F16_setTransitionTimeLimit(Ifx_MS_FocSolution_F16* self, uint16_t transitionTimeLimit_cycles)
{
    self->param->transitionTimeLimit_cycles = transitionTimeLimit_cycles;
    Ifx_MS_FocSolution_F16_p_setTransitionDelta(self, transitionTimeLimit_cycles);
}


static inline void Ifx_MS_FocSolution_F16_p_setTransitionDelta(Ifx_MS_FocSolution_F16* self, uint16_t
                                                               transitionTimeLimit_cycles)
{
    if (transitionTimeLimit_cycles > (uint16_t)IFX_MATH_FRACT16_MAX)
    {
        self->p_transitionDeltaQ15 = 1;
    }
    else
    {
        self->p_transitionDeltaQ15 = Ifx_Math_DivShL_F16(IFX_MATH_FRACT16_MAX,
            (Ifx_Math_Fract16)transitionTimeLimit_cycles, 0U);
    }
}


void Ifx_MS_FocSolution_F16_setTransitionSpeedUp(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 transitionSpeedUpQ15)
{
    /* Set the internal parameter */
    self->param->transitionSpeedUpQ15 = transitionSpeedUpQ15;

    /* Calculate dependent transition speed parameters */
    Ifx_MS_FocSolution_F16_p_calcInternalTransitionSpeeds(self);
}


void Ifx_MS_FocSolution_F16_setTransitionSpeedBand(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                   transitionSpeedBandQ15)
{
    /* Set the internal parameter */
    self->param->transitionSpeedBandQ15 = transitionSpeedBandQ15;

    /* Calculate dependent transition speed parameters */
    Ifx_MS_FocSolution_F16_p_calcInternalTransitionSpeeds(self);
}


static inline void Ifx_MS_FocSolution_F16_p_calcInternalTransitionSpeeds(Ifx_MS_FocSolution_F16* self)
{
    /* Set the transition speed down */
    self->transitionSpeedDownQ15 = self->param->transitionSpeedUpQ15 - self->param->transitionSpeedBandQ15;

    /* Set the middle transition speed */
    self->transitionSpeedMidQ15 = self->transitionSpeedDownQ15 + Ifx_Math_ShR_F16(self->param->transitionSpeedBandQ15,
        1);

    /* Set half of the speed band */
    self->p_transitionSpeedBandHalfQ15 = Ifx_Math_ShR_F16(self->param->transitionSpeedBandQ15, 1);
}


void Ifx_MS_FocSolution_F16_setStartUpCurrent(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16 startUpCurrentQ15)
{
    /* Set the startup current in the parameter struct */
    self->param->startUpCurrentQ15 = startUpCurrentQ15;

    /* Set the startup current */
    Ifx_MDA_IToFController_F16_setReferenceCurrentImag(&(self->iToF), startUpCurrentQ15);

    /* Set the D current for smooth transition */
    Ifx_MS_FocSolution_F16_p_setTransitionDownTargetDCurrentQ15(self);
}


void Ifx_MS_FocSolution_F16_setTransitionDownDCurrentScalingQ14(Ifx_MS_FocSolution_F16* self, Ifx_Math_Fract16
                                                                transitionDownDCurrentScalingQ14)
{
    if (transitionDownDCurrentScalingQ14 < 0)
    {
        self->param->transitionDownDCurrentScalingQ14 = 0;
    }
    else
    {
        self->param->transitionDownDCurrentScalingQ14 = transitionDownDCurrentScalingQ14;
    }

    /* Set the D current for smooth transition */
    Ifx_MS_FocSolution_F16_p_setTransitionDownTargetDCurrentQ15(self);
}


static inline void Ifx_MS_FocSolution_F16_p_setTransitionDownTargetDCurrentQ15(Ifx_MS_FocSolution_F16* self)
{
    /* Set the D current for smooth transition */
    self->p_transitionDownTargetDCurrentQ15 = Ifx_Math_MulShRSat_F16(self->param->startUpCurrentQ15,
        self->param->transitionDownDCurrentScalingQ14, 14U);
}


void Ifx_MS_FocSolution_F16_setSpeedPiPropGain(Ifx_MS_FocSolution_F16* self, uint32_t propGain)
{
    /* Set internal parameter */
    self->param->speedPiPropGain = propGain;

    /* Get the gain in Q format */
    Ifx_Math_Fract16Q propGainQ = Ifx_Math_ConvToQForm_F16(propGain, IFX_MS_FOCSOLUTION_F16_DEC_PLACES);

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setPropGain(&(self->speedPi), propGainQ);
}


uint32_t Ifx_MS_FocSolution_F16_getSpeedPiPropGain(Ifx_MS_FocSolution_F16* self)
{
    return self->param->speedPiPropGain;
}


void Ifx_MS_FocSolution_F16_setSpeedPiIntegGainSamplingTime(Ifx_MS_FocSolution_F16* self, uint32_t
                                                            integGainSamplingTime)
{
    /* Set internal parameter */
    self->param->speedPiIntegGainSamplingTime = integGainSamplingTime;

    /* Get the gain in Q format */
    Ifx_Math_Fract16Q integGainSamplingTimeQ = Ifx_Math_ConvToQForm_F16(integGainSamplingTime,
        IFX_MS_FOCSOLUTION_F16_DEC_PLACES);

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setIntegGainSamplingTime(&(self->speedPi), integGainSamplingTimeQ);
}


uint32_t Ifx_MS_FocSolution_F16_getSpeedPiIntegGainSamplingTime(Ifx_MS_FocSolution_F16* self)
{
    return self->param->speedPiIntegGainSamplingTime;
}
