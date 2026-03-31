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
#include "Ifx_MHA_PatternGen_F16_TLE987.h"
#include "Ifx_MHA_PatternGen_F16_TLE987_Cfg.h"

/* SDK */
#include "int.h"
#include "port.h"
#include "timer3.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/* Macros to define the component ID */
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_SOURCEID \
    ((uint8_t)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_LIBRARYID \
    ((uint16_t)Ifx_ComponentID_LibraryID_mctrlHardwareAbstraction)
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_MODULEID        (2U)
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_COMPONENTID1    (1U)
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_COMPONENTID2    ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_MAJOR      (2U)
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_MINOR      (0U)
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_PATCH      (0U)
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_T          (4U)
#define IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_REV        (0U)

/* Macro for enabling CCU6 outputs */
#define IFX_MHA_PATTERNGEN_F16_TLE987_PWM_PATTERN_SHADOW_OUT      (0x3FUL)

/* Macro for triggering T13 in T12 zero match */
#define IFX_MHA_PATTERNGEN_F16_TLE987_TRIGGER_T13_ON_ZERO_MATCH   (0x7AU)

/* Macro for triggering T13 in T12 period match */
#define IFX_MHA_PATTERNGEN_F16_TLE987_TRIGGER_T13_ON_PERIOD_MATCH (0x76U)

/* Total number of compare values */
#define IFX_MHA_PATTERNGEN_F16_TLE987_N_COMPARE_VALUES            (6U)

/* Minimum trigger time */
#define IFX_MHA_PATTERNGEN_F16_TLE987_MIN_TRIGGER_TIME            (2U)

/* Maximum supported pwm frequency for MCL on tle987x in kHz*/
#define IFX_MHA_PATTERNGEN_F16_TLE987_FREQUENCY_MAX               40U

/******************************************************************************/
/*-----------------------Static Function Prototypes---------------------------*/
/******************************************************************************/

/**
 * \brief Set CCU Trap Control register according to param fault reaction
 * Except for fault reaction is DISBALE the trap functionality block is enabled.
 * But only for fault reaction REPORT_REACT trap pin level has an impact on the CCU6 channels and trap must be cleared
 * by SW.
 *
 * \param self unused
 * \param faultReaction A value of IFX_MHA_PATTERNGEN_F16_TLE987_FAULT_REACTION_*
 */
static void Ifx_MHA_PatternGen_F16_TLE987_setFaultReactionOnTrap(Ifx_MHA_PatternGen_F16_TLE987* self, uint8_t
                                                                 faultReaction);

/**
 * \brief Check trap pin and if it is high clear the trap flag.
 * If the trap fault is disable, the trap flag does not need to be cleared.
 */
static void Ifx_MHA_PatternGen_F16_TLE987_clearTrapFlag(void);

/* Checks if any fault occurred and acts accordingly */
static inline bool Ifx_MHA_PatternGen_F16_TLE987_checkFaultStatus(Ifx_MHA_PatternGen_F16_TLE987* self, bool
                                                                  clearFault);

/* Pattern generator state machine implementation */
static inline void Ifx_MHA_PatternGen_F16_TLE987_stateMachine(Ifx_MHA_PatternGen_F16_TLE987* self, bool faultStatus,
                                                              bool clearFault);

/* Pattern generator in on state */
static inline Ifx_MHA_PatternGen_F16_TLE987_State Ifx_MHA_PatternGen_F16_TLE987_stateOn(
    Ifx_MHA_PatternGen_F16_TLE987* self, bool faultStatus);

/* Pattern generator in off state */
static inline Ifx_MHA_PatternGen_F16_TLE987_State Ifx_MHA_PatternGen_F16_TLE987_stateOff(
    Ifx_MHA_PatternGen_F16_TLE987* self, bool faultStatus);

/* Pattern generator in fault state */
static inline Ifx_MHA_PatternGen_F16_TLE987_State Ifx_MHA_PatternGen_F16_TLE987_stateFault(
    Ifx_MHA_PatternGen_F16_TLE987* self, bool faultStatus, bool clearFault);

/* Load shadow registers and request shadow transfer */
static inline void Ifx_MHA_PatternGen_F16_TLE987_transferDownCounting(Ifx_MHA_PatternGen_F16_TLE987* self);
static inline void Ifx_MHA_PatternGen_F16_TLE987_transferUpCounting(Ifx_MHA_PatternGen_F16_TLE987* self);

/* Disable and enable the pattern generator */
static inline void Ifx_MHA_PatternGen_F16_TLE987_actionDisable(void);
static inline void Ifx_MHA_PatternGen_F16_TLE987_actionEnable(void);

#if (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_CURRENT_LOOP_FACTOR > 1)

/* Update values from the shadow variables to the used ones */
static inline void Ifx_MHA_PatternGen_F16_TLE987_updateCompareAndTriggers(Ifx_MHA_PatternGen_F16_TLE987* self);
#endif

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Component ID */
static const Ifx_ComponentID      Ifx_MHA_PatternGen_F16_TLE987_componentID = {
    .sourceID     = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_SOURCEID,
    .libraryID    = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_LIBRARYID,
    .moduleID     = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_MODULEID,
    .componentID1 = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MHA_PatternGen_F16_TLE987_componentVersion = {
    .major = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_MAJOR,
    .minor = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_MINOR,
    .patch = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_PATCH,
    .t     = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_T,
    .rev   = IFX_MHA_PATTERNGEN_F16_TLE987_COMPONENTVERSION_REV
};
/* *INDENT-ON* */
/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
/* Function to get the component ID */
void Ifx_MHA_PatternGen_F16_TLE987_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MHA_PatternGen_F16_TLE987_componentID;
}


/* Function to get the component version */
void Ifx_MHA_PatternGen_F16_TLE987_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MHA_PatternGen_F16_TLE987_componentVersion;
}


void Ifx_MHA_PatternGen_F16_TLE987_init(Ifx_MHA_PatternGen_F16_TLE987* self)
{
    /* Initialize internal parameters to 0 */
    self->p_enable       = false;
    self->p_clearFault   = false;
    self->p_status.state = Ifx_MHA_PatternGen_F16_TLE987_State_init;
    self->p_status.trap  = false;

    /* Ensure that CCU6 configuration is in sync with static configuration. This is necessary in case the PatternGen
     * configuration is generated from the python parameter model respectively without Configwizard. */
    Ifx_MHA_PatternGen_F16_TLE987_setDeadTime_ns(self, IFX_MHA_PATTERNGEN_F16_TLE987_CFG_DEADTIME);
    Ifx_MHA_PatternGen_F16_TLE987_setFrequency_kHz(self, IFX_MHA_PATTERNGEN_F16_TLE987_CFG_FREQUENCY_KHZ);
    Ifx_MHA_PatternGen_F16_TLE987_setFaultReactionOnTrap(self, IFX_MHA_PATTERNGEN_F16_TLE987_CFG_FAULT_REACTION_TRAP);

    /* Reset the trigger and the compare values to be used in the interrupts */
    self->p_triggerTime_tick[0] = self->p_triggerTimeShadow_tick[0] = IFX_MHA_PATTERNGEN_F16_TLE987_MIN_TRIGGER_TIME;
    self->p_triggerTime_tick[1] = self->p_triggerTimeShadow_tick[1] = CCU6_T12_Period_Value_Get() + 1U;

    for (uint8_t i = 0U; i < IFX_MHA_PATTERNGEN_F16_TLE987_N_COMPARE_VALUES; i++)
    {
        self->p_compareValues_tick[i]       = 0U;
        self->p_compareValuesShadow_tick[i] = 0U;
    }

    /* Reset executed flag and cycle counter */
    self->p_executed     = false;
    self->p_cycleCounter = 0U;

    /* Start T13 on the period match to generate trigger for second current measurement */
    CCU6_SetT13Trigger(IFX_MHA_PATTERNGEN_F16_TLE987_TRIGGER_T13_ON_PERIOD_MATCH);
    Ifx_MHA_PatternGen_F16_TLE987_clearTrapFlag();
}


static void Ifx_MHA_PatternGen_F16_TLE987_setFaultReactionOnTrap(Ifx_MHA_PatternGen_F16_TLE987* self, uint8_t
                                                                 faultReaction)
{
    (void)self;

    switch (faultReaction)
    {
        case IFX_MHA_PATTERNGEN_F16_TLE987_FAULT_REACTION_DISABLE:
            CCU6_Trap_Channel_En(0U);
            CCU6_Trap_HW_Clr_En();
            CCU6_Trap_Pin_Dis();
            break;

        case IFX_MHA_PATTERNGEN_F16_TLE987_FAULT_REACTION_ENABLE:
            CCU6_Trap_Channel_En(0U);
            CCU6_Trap_HW_Clr_En();
            CCU6_Trap_Pin_En();
            break;

        case IFX_MHA_PATTERNGEN_F16_TLE987_FAULT_REACTION_REPORT_ONLY:
            CCU6_Trap_Channel_En(0U);
            CCU6_Trap_HW_Clr_En();
            CCU6_Trap_Pin_En();
            break;

        /* Default fault reaction is IFX_MHA_PATTERNGEN_F16_TLE987_FAULT_REACTION_REPORT_REACT */
        default:
            CCU6_Trap_Channel_En(0x3FU);
            CCU6_Trap_SW_Clr_En();
            CCU6_Trap_Pin_En();
            break;
    }
}


static void Ifx_MHA_PatternGen_F16_TLE987_clearTrapFlag(void)
{
#if IFX_MHA_PATTERNGEN_F16_TLE987_CFG_FAULT_REACTION_TRAP != 0

    /* Clear the trap interrupt flag if it is high */
    uint8_t trapPortStatus;

    /* Check which port is used for trap and get the port status */
    if (CCU6->PISEL0.bit.ISTRP == 0)
    {
        trapPortStatus = PORT_P24_Get();
    }
    else
    {
        trapPortStatus = PORT_P23_Get();
    }

    /* If the port is high, clear the trap flag */
    if (trapPortStatus != 0)
    {
        CCU6_TRAP_Int_Clr();
    }

#endif
}


void Ifx_MHA_PatternGen_F16_TLE987_execute(Ifx_MHA_PatternGen_F16_TLE987* self, uint16_t compareValues[6], uint16_t
                                           triggerTime_tick[2])
{
    /* Store clearFault variable at the beginning of the module */
    bool clearFault = self->p_clearFault;

/* If the current loop factor is 1, then don't use the shadow variables */
#if (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_CURRENT_LOOP_FACTOR == 1)

    /* Assign the trigger and the compare values to be used in the interrupts */
    self->p_triggerTime_tick[0] = triggerTime_tick[0];
    self->p_triggerTime_tick[1] = triggerTime_tick[1];

    for (uint8_t i = 0; i < IFX_MHA_PATTERNGEN_F16_TLE987_N_COMPARE_VALUES; i++)
    {
        self->p_compareValues_tick[i] = compareValues[i];
    }

#else

    /* Assign the trigger and the compare values to the shadow variables */
    self->p_triggerTimeShadow_tick[0] = triggerTime_tick[0];
    self->p_triggerTimeShadow_tick[1] = triggerTime_tick[1];

    for (uint8_t i = 0U; i < IFX_MHA_PATTERNGEN_F16_TLE987_N_COMPARE_VALUES; i++)
    {
        self->p_compareValuesShadow_tick[i] = compareValues[i];
    }

#endif

    /* Check if any fault occurred and execute state machine */
    bool faultStatusLocal = Ifx_MHA_PatternGen_F16_TLE987_checkFaultStatus(self, clearFault);
    Ifx_MHA_PatternGen_F16_TLE987_stateMachine(self, faultStatusLocal, clearFault);
}


/* Period match interrupt */
void Ifx_MHA_PatternGen_F16_TLE987_onPeriodMatch(Ifx_MHA_PatternGen_F16_TLE987* self)
{
    /* Last cycle and execute was already called, update compare values and transfer it to the CCU6 */
    if ((self->p_cycleCounter == (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_CURRENT_LOOP_FACTOR - 1))
        && (self->p_executed == true))
    {
/* Only update the values if the current loop factor is bigger than 1 */
#if (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_CURRENT_LOOP_FACTOR > 1)

        /* Update compare and trigger values */
        Ifx_MHA_PatternGen_F16_TLE987_updateCompareAndTriggers(self);
#endif

        /* Transfer compare values and reset counter and flag */
        Ifx_MHA_PatternGen_F16_TLE987_transferUpCounting(self);
    }

    /* Last cycle and PatternGen execute was not called, compare values will be updated by execute */
    else if (self->p_cycleCounter == (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_CURRENT_LOOP_FACTOR - 1))
    {
        self->p_cycleCounter++;
    }

    /* Not the last cycle, transfer current shadow values to the CCU6 */
    else
    {
        /* Transfer compare values and increment cycle counter */
        Ifx_MHA_PatternGen_F16_TLE987_transferUpCounting(self);
        self->p_cycleCounter++;
    }
}


/* One match interrupt */
void Ifx_MHA_PatternGen_F16_TLE987_onOneMatch(Ifx_MHA_PatternGen_F16_TLE987* self)
{
    /* Transfer compare values */
    Ifx_MHA_PatternGen_F16_TLE987_transferDownCounting(self);
}


static inline void Ifx_MHA_PatternGen_F16_TLE987_transferUpCounting(Ifx_MHA_PatternGen_F16_TLE987* self)
{
    /* Load shadow registers with the up counting compare values */

#if (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_INVERT_ROTATION == 1)

    /* Enable inverse rotation by swapping the compare value of phase u up-counting and phase v up-counting */
    CCU6_LoadShadowRegister_CC60(self->p_compareValues_tick[1]);
    CCU6_LoadShadowRegister_CC61(self->p_compareValues_tick[0]);
#else
    CCU6_LoadShadowRegister_CC60(self->p_compareValues_tick[0]);
    CCU6_LoadShadowRegister_CC61(self->p_compareValues_tick[1]);
#endif /* (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_INVERT_ROTATION == 1) */
    CCU6_LoadShadowRegister_CC62(self->p_compareValues_tick[2]);

    /* Load shadow register for first trigger */
    TIMER3_Set_Cmp_Value(self->p_triggerTime_tick[0] >> 1U);

    /* Enable shadow transfer */
    CCU6_T12_Str_En();
}


static inline void Ifx_MHA_PatternGen_F16_TLE987_transferDownCounting(Ifx_MHA_PatternGen_F16_TLE987* self)
{
    /* Store T12 period value */
    uint16 periodValue = CCU6_T12_Period_Value_Get() + 1;
    uint16 timerPeriod = 2U * periodValue;

    /* Load shadow registers with the down counting compare values */
#if (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_INVERT_ROTATION == 1)

    /* Enable inverse rotation by swapping the compare value of phase u down-counting and v down-counting */
    CCU6_LoadShadowRegister_CC60(timerPeriod - self->p_compareValues_tick[4]);
    CCU6_LoadShadowRegister_CC61(timerPeriod - self->p_compareValues_tick[3]);
#else
    CCU6_LoadShadowRegister_CC60(timerPeriod - self->p_compareValues_tick[3]);
    CCU6_LoadShadowRegister_CC61(timerPeriod - self->p_compareValues_tick[4]);
#endif /* (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_INVERT_ROTATION == 1) */
    CCU6_LoadShadowRegister_CC62(timerPeriod - self->p_compareValues_tick[5]);

    /* Load shadow register for second trigger */
    CCU6_LoadShadowRegister_CC63(self->p_triggerTime_tick[1] - periodValue);

    /* Enable shadow transfer */
    CCU6_T12_Str_En();
    CCU6_T13_Str_En();
}


#if (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_CURRENT_LOOP_FACTOR > 1)
static inline void Ifx_MHA_PatternGen_F16_TLE987_updateCompareAndTriggers(Ifx_MHA_PatternGen_F16_TLE987* self)
{
    /* Update compare values */
    self->p_triggerTime_tick[0] = self->p_triggerTimeShadow_tick[0];
    self->p_triggerTime_tick[1] = self->p_triggerTimeShadow_tick[1];

    for (uint8_t i = 0U; i < IFX_MHA_PATTERNGEN_F16_TLE987_N_COMPARE_VALUES; i++)
    {
        self->p_compareValues_tick[i] = self->p_compareValuesShadow_tick[i];
    }
}


#endif

static inline Ifx_MHA_PatternGen_F16_TLE987_State Ifx_MHA_PatternGen_F16_TLE987_stateOn(
    Ifx_MHA_PatternGen_F16_TLE987* self, bool faultStatus)
{
    /* Variable to store the next state */
    Ifx_MHA_PatternGen_F16_TLE987_State nextState;

    if (faultStatus == true)
    {
        /* Set the state to fault */
        nextState = Ifx_MHA_PatternGen_F16_TLE987_State_fault;
    }

    /* Check if module disabled */
    else if (self->p_enable == false)
    {
        /* Set the state to OFF */
        nextState = Ifx_MHA_PatternGen_F16_TLE987_State_off;

        /* Disable the pattern generator */
        Ifx_MHA_PatternGen_F16_TLE987_actionDisable();
    }
    else
    {
        /* Disable CCU6 SR1 node */
        NVIC_Node5_Dis();

        /* Transfer values if the last period match was already executed */
        if (self->p_cycleCounter == IFX_MHA_PATTERNGEN_F16_TLE987_CFG_CURRENT_LOOP_FACTOR)
        {
#if (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_CURRENT_LOOP_FACTOR > 1)

            /* Update compare and trigger values and transfer them to CCU6 */
            Ifx_MHA_PatternGen_F16_TLE987_updateCompareAndTriggers(self);
#endif
            Ifx_MHA_PatternGen_F16_TLE987_transferUpCounting(self);
        }
        else
        {
            /* Signal to the period match function that this function was executed */
            self->p_executed = true;
        }

        nextState = Ifx_MHA_PatternGen_F16_TLE987_State_on;

        /* Enable CCU6 SR1 node */
        NVIC_Node5_En();
    }

    return nextState;
}


static inline Ifx_MHA_PatternGen_F16_TLE987_State Ifx_MHA_PatternGen_F16_TLE987_stateOff(
    Ifx_MHA_PatternGen_F16_TLE987* self, bool faultStatus)
{
    /* Variable to store the next state */
    Ifx_MHA_PatternGen_F16_TLE987_State nextState;

    /* Check if any fault occurred */
    if (faultStatus == true)
    {
        /* Set the state to fault */
        nextState = Ifx_MHA_PatternGen_F16_TLE987_State_fault;
    }

    /* Check if module enabled */
    else if (self->p_enable == true)
    {
        /* Transition to ON if module enabled on the next execution cycle */
        nextState = Ifx_MHA_PatternGen_F16_TLE987_State_on;

        /* Enable the pattern generator */
        Ifx_MHA_PatternGen_F16_TLE987_actionEnable();
    }
    else
    {
        nextState = Ifx_MHA_PatternGen_F16_TLE987_State_off;
    }

    return nextState;
}


static inline Ifx_MHA_PatternGen_F16_TLE987_State Ifx_MHA_PatternGen_F16_TLE987_stateFault(
    Ifx_MHA_PatternGen_F16_TLE987* self, bool faultStatus, bool clearFault)
{
    (void)self;

    /* Variable to store the next state */
    Ifx_MHA_PatternGen_F16_TLE987_State nextState;

    if ((faultStatus == false)
        && (clearFault == true))
    {
        /* Transition to off if module is in fault in the next execution cycle */
        nextState = Ifx_MHA_PatternGen_F16_TLE987_State_off;

        /* Disable the pattern generator*/
        Ifx_MHA_PatternGen_F16_TLE987_actionDisable();
    }
    else
    {
        nextState = Ifx_MHA_PatternGen_F16_TLE987_State_fault;
    }

    return nextState;
}


static inline void Ifx_MHA_PatternGen_F16_TLE987_stateMachine(Ifx_MHA_PatternGen_F16_TLE987* self, bool faultStatus,
                                                              bool clearFault)
{
    /* Variable to store the state */
    Ifx_MHA_PatternGen_F16_TLE987_State previousState = self->p_status.state;
    Ifx_MHA_PatternGen_F16_TLE987_State nextState     = previousState;

    switch (previousState)
    {
        /* Initialize the module */
        case Ifx_MHA_PatternGen_F16_TLE987_State_init:

            /* Set the state to OFF */
            nextState = Ifx_MHA_PatternGen_F16_TLE987_State_off;
            break;

        /* Pattern generator is running */
        case Ifx_MHA_PatternGen_F16_TLE987_State_on:
            nextState = Ifx_MHA_PatternGen_F16_TLE987_stateOn(self, faultStatus);
            break;

        /* Pattern generator not running */
        case Ifx_MHA_PatternGen_F16_TLE987_State_off:
            nextState = Ifx_MHA_PatternGen_F16_TLE987_stateOff(self, faultStatus);
            break;

        /* Pattern generator is in fault */
        case Ifx_MHA_PatternGen_F16_TLE987_State_fault:
            nextState = Ifx_MHA_PatternGen_F16_TLE987_stateFault(self, faultStatus, clearFault);
            break;

        default:

            /* do default transition to INIT */
            nextState = Ifx_MHA_PatternGen_F16_TLE987_State_init;
            break;
    }

    self->p_status.state = nextState;

    /* Clear the internal variable */
    if (clearFault == true)
    {
        self->p_clearFault = false;
    }
}


static inline bool Ifx_MHA_PatternGen_F16_TLE987_checkFaultStatus(Ifx_MHA_PatternGen_F16_TLE987* self, bool
                                                                  clearFault)
{
    /* boolean output fault status initialized to false */
    bool faultStatusRet = false;

    /* local copy of the previously detected faults */
    bool faultTrap = self->p_status.trap;

    /* first check if a fault clear request was placed */
    if (clearFault == true)
    {
        /* Clear the trap fault in case a fault clear request was done */
        faultTrap = false;

        /* Clear the trap flag */
        CCU6_TRAP_Int_Clr();
    }

    /* Check if the trap fault is enabled */
#if IFX_MHA_PATTERNGEN_F16_TLE987_CFG_FAULT_REACTION_TRAP >= IFX_MHA_PATTERNGEN_F16_TLE987_FAULT_REACTION_ENABLE

    /* Read the status bits for trap fault from pattern generator Interrupt Status register */
    if (CCU6_Trap_Flag_Int_Sts() != 0)
    {
        /* set the trap fault information status bit */
        faultTrap = true;

        /* Check if this fault is configured for REPORTING */
#if IFX_MHA_PATTERNGEN_F16_TLE987_CFG_FAULT_REACTION_TRAP >= IFX_MHA_PATTERNGEN_F16_TLE987_FAULT_REACTION_REPORT_ONLY

#if (IFX_MHA_PATTERNGEN_F16_TLE987_CFG_ENABLE_FAULT_OUT == 1)

        /* report fault source ONLY the first time it occurs */
        if (self->p_status.trap == false)
        {
            /* report the fault through the user interface */
            IFX_MHA_PATTERNGEN_F16_TLE987_CFG_FAULT_OUT();
        }

#endif

        /* Check if this fault is configured for REACTION */
#if IFX_MHA_PATTERNGEN_F16_TLE987_CFG_FAULT_REACTION_TRAP >= IFX_MHA_PATTERNGEN_F16_TLE987_FAULT_REACTION_REPORT_REACT

        /* set the fault status for reacting */
        faultStatusRet = true;
#endif
#endif
    }

#endif

    self->p_status.trap = faultTrap;

    return faultStatusRet;
}


static inline void Ifx_MHA_PatternGen_F16_TLE987_actionDisable(void)
{
    /* Sets Multi-Channel PWM Pattern shadow register */
    CCU6_Multi_Ch_PWM_Shadow_Reg_Load((uint16_t)0x0000U);

    /* Enables shadow transfer request by software */
    CCU6_MCM_PWM_Str_SW_En();
}


static inline void Ifx_MHA_PatternGen_F16_TLE987_actionEnable(void)
{
    /* Sets Multi-Channel PWM Pattern shadow register */
    CCU6_Multi_Ch_PWM_Shadow_Reg_Load((uint16_t)IFX_MHA_PATTERNGEN_F16_TLE987_PWM_PATTERN_SHADOW_OUT);

    /* Enables shadow transfer request by software*/
    CCU6_MCM_PWM_Str_SW_En();
}


void Ifx_MHA_PatternGen_F16_TLE987_setDeadTime_ns(Ifx_MHA_PatternGen_F16_TLE987* self, uint32_t deadTime_ns)
{
    (void)self;

    /* Local variables for dead time ticks and dead time */
    uint32_t deadTimeSet_tick;
    uint32_t newDeadtime = deadTime_ns;

#if IFX_MHA_PATTERNGEN_F16_TLE987_CFG_MIN_DEADTIME > 0

    /* Check if input dead time is less than the minimum dead time */
    if (deadTime_ns < IFX_MHA_PATTERNGEN_F16_TLE987_CFG_MIN_DEADTIME)
    {
        newDeadtime = IFX_MHA_PATTERNGEN_F16_TLE987_CFG_MIN_DEADTIME;
    }

#endif

    /* Conversion of dead time into dead time ticks for center aligned mode */
    deadTimeSet_tick = ((newDeadtime) * CCU6_T12_CLK) / (IFX_MHA_PATTERNGEN_F16_TLE987_NANO_TO_MICRO);

    /* Setting up dead time in ticks using CCU6 function */
    CCU6_Deadtime_Set((uint16)deadTimeSet_tick);
}


uint32_t Ifx_MHA_PatternGen_F16_TLE987_getDeadTime_ns(Ifx_MHA_PatternGen_F16_TLE987* self)
{
    (void)self;

    /* Local variable for dead time*/
    uint32_t deadTimeGet;

    /* Conversion of dead time ticks into dead time for center align mode */
    deadTimeGet = ((uint32_t)CCU6_Deadtime_Get() * IFX_MHA_PATTERNGEN_F16_TLE987_NANO_TO_MICRO);
    deadTimeGet = deadTimeGet / (CCU6_T12_CLK);

    return deadTimeGet;
}


void Ifx_MHA_PatternGen_F16_TLE987_setFrequency_kHz(Ifx_MHA_PatternGen_F16_TLE987* self, uint32_t frequency_kHz)
{
    uint32_t period_tick;

    if ((self->p_status.state == Ifx_MHA_PatternGen_F16_TLE987_State_off)
        || (self->p_status.state == Ifx_MHA_PatternGen_F16_TLE987_State_init))
    {
        /* Check if frequency is greater than MAX  */
        if (frequency_kHz > IFX_MHA_PATTERNGEN_F16_TLE987_FREQUENCY_MAX)
        {
            frequency_kHz = IFX_MHA_PATTERNGEN_F16_TLE987_FREQUENCY_MAX;
        }

        /* Conversion of frequency into period ticks for center align mode */
        period_tick = ((CCU6_T12_CLK * IFX_MHA_PATTERNGEN_F16_TLE987_MHZ_TO_KHZ) / (frequency_kHz));
        period_tick = (period_tick / (2U)) - 1U;

        /* Setting up period ticks using CCU6 function */
        CCU6_T12_Period_Value_Set((uint16)period_tick);

        /* The period of T13 should be period of T12 -1. The -1 is to allow the timer to start again
         * There is a period match on T13 1 cycle before a period match in T12, and T13 start is triggered again by T12
         * period match.
         */
        CCU6_LoadPeriodRegister_T13_Tick((uint16)period_tick - 1U);
    }
    else
    {
        /* ignore set value */
    }
}


uint32_t Ifx_MHA_PatternGen_F16_TLE987_getFrequency_kHz(Ifx_MHA_PatternGen_F16_TLE987* self)
{
    (void)self;

    /* Local variable for frequency*/
    uint32_t frequencyTemp;

    /* Conversion of period ticks into frequency for center align mode */
    frequencyTemp = ((CCU6_T12_CLK * IFX_MHA_PATTERNGEN_F16_TLE987_MHZ_TO_KHZ) /
                     ((uint32_t)CCU6_T12_Period_Value_Get() + 1U));
    frequencyTemp = frequencyTemp / (2U);

    return frequencyTemp;
}
