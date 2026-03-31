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
 *
 * \file Ifx_MHA_BEMFComparators_F16_TLE987.c
 * \brief A specialized Back EMF control module for the TLE987x devices
 *
 */

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_MHA_BEMFComparators_F16_TLE987.h"

/* SDK */
#include "bdrv.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Macros to define the component ID */
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_SOURCEID \
    ((uint8_t)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_LIBRARYID \
    ((uint16_t)Ifx_ComponentID_LibraryID_mctrlHardwareAbstraction)
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_MODULEID     (4U)
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_COMPONENTID1 (0U)
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_COMPONENTID2 ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_MAJOR   (1U)
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_MINOR   (0U)
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_PATCH   (0U)
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_T       (4U)
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_REV     (0U)
/* *INDENT-ON* */
/* Map back emf sector WVU to real sector */
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_0           (2) /* 010 */
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_1           (6) /* 110 */
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_2           (4) /* 100 */
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_3           (5) /* 101 */
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_4           (1) /* 001 */
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_5           (3) /* 011 */

#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_ZC_MASK            7U
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_ZC_SHIFT           16U

/* Counter thresholds */
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_DEBOUNCE_CTR_MAX   (10)
#define IFX_MHA_BEMFCOMPARATORS_F16_TLE987_STANDSTILL_CTR_MAX (1000)

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Component ID */
static const Ifx_ComponentID      Ifx_MHA_BEMFComparators_F16_TLE987_componentID = {
    .sourceID     = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_SOURCEID,
    .libraryID    = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_LIBRARYID,
    .moduleID     = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_MODULEID,
    .componentID1 = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MHA_BEMFComparators_F16_TLE987_componentVersion = {
    .major = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_MAJOR,
    .minor = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_MINOR,
    .patch = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_PATCH,
    .t     = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_T,
    .rev   = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_COMPONENTVERSION_REV
};
/* *INDENT-ON* */
/******************************************************************************/
/*-----------------------Static Function Prototypes---------------------------*/
/******************************************************************************/

/**
 * \brief Update the sector information
 *
 * This function maps the back emf comparator status to a sector value between 0 and 5
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MHA_BEMFComparators_F16_TLE987_updateSector(Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Function called in the init state
 *
 * This function disables the back emf comparator and sets the next state to off
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_stateInit(
    Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Function called in the off state
 *
 * This function sets the next state to the debounce state if the module is enabled and
 * enables the hardware comparators.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_stateOff(
    Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Function called in debounce state
 *
 * \param [inout] self Reference to structure that contains instance data members
 */
static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_stateDebounce(
    Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Check state transition in debounce state
 *
 * This function handles the transition of the debounce state. If the module is disabled the next state is set to off
 * and the hardware comparators are disabled. If the debounce counter reaches the positive maximum then the next state
 * is set to onPos and the internal flag isSignalValid is set to true. If the debounce counter reaches the negative
 * maximum the next state is set to onNeg and the internal flag isSignalValid is set to true.
 *
 * \param [inout] self Reference to structure that contains instance data members
 */
static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_transitionStateDebounce(
    Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Increment/decrement debounce counter
 *
 * This function updates the debounce counter on a sector change. If the sector change is in the positive direction then
 * the counter is incremented, otherwise it is decremented.
 *
 * \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MHA_BEMFComparators_F16_TLE987_doStateDebounce(Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Function called in the onPos and onNeg State
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_stateOn(
    Ifx_MHA_BEMFComparators_F16_TLE987* self, Ifx_MHA_BEMFComparators_F16_TLE987_State actualState);

/**
 * \brief Check state transition in onPos and onNeg state
 *
 * This function sets the next state to the off state if the module is disabled, disables the hardware comparators, and
 * sets the output to 0 and flag isSignalValid to false. If the standstill counter or debouce counter have run out, the
 * counters are reset to their default values, the output is set to 0, the internal flag isSignalValid is set to false
 * and the next state is set to debounce.
 *
 * \param [inout] self Reference to structure that contains instance data members
 */
static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_transitionStateOn(
    Ifx_MHA_BEMFComparators_F16_TLE987* self, Ifx_MHA_BEMFComparators_F16_TLE987_State actualState);

/**
 * \brief Update Comparator state and debounce counter in onPos state.
 *
 * This function checks the new sector and on a positive sector change reads the comparator status, increments and
 * saturates the debounce counter. It decrements the debounce counter on a negative sector change. In addition, it
 * resets the standstill counter on any sector change, and decrements it when no sector change occurs.
 *
 * \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MHA_BEMFComparators_F16_TLE987_doStateOnPos(Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Update Comparator state and debounce counter in onNeg state.
 *
 * This function checks the new sector and on a negative sector change reads the comparator status, decrements and
 * saturates the debounce counter. It increments the debounce counter on a positive sector change. In addition, it
 * resets the standstill counter on any sector change, and decrements it when no sector change occurs.
 *
 * \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MHA_BEMFComparators_F16_TLE987_doStateOnNeg(Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Read status of 3 bemf comparators
 *
 *  \param [out] bemfComparatorState state of comparators u,v,w
 */
static inline void Ifx_MHA_BEMFComparators_F16_TLE987_readComparatorStatus(
    Ifx_Math_3PhaseFract16* bemfComparatorState);

/**
 * \brief Set signal invalid
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline void Ifx_MHA_BEMFComparators_F16_TLE987_setSignalInvalid(Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Enable/disable the hardware comparators
 *
 * \param [in] enable Parameter of boolean type to enable or disable the hardware comparators
 */
static inline void Ifx_MHA_BEMFComparators_F16_TLE987_enableComparator(bool enable);

/**
 * \brief Increment the sector with overflow
 *
 * Increments the old sector value and returns the result. If the result of the increment is 6, then
 * 0 is returned.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline uint8_t Ifx_MHA_BEMFComparators_F16_TLE987_incSector(Ifx_MHA_BEMFComparators_F16_TLE987* self);

/**
 * \brief Decrement the sector with overflow
 *
 * Decrements the old sector value and returns the result. If the result of the increment is -1, then
 * 5 is returned.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
static inline uint8_t Ifx_MHA_BEMFComparators_F16_TLE987_decSector(Ifx_MHA_BEMFComparators_F16_TLE987* self);

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
/* Function to get the component ID */
void Ifx_MHA_BEMFComparators_F16_TLE987_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MHA_BEMFComparators_F16_TLE987_componentID;
}


/* Function to get the component version */
void Ifx_MHA_BEMFComparators_F16_TLE987_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MHA_BEMFComparators_F16_TLE987_componentVersion;
}


static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_stateInit(
    Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    (void)self;

    /* Disable HW comparators */
    Ifx_MHA_BEMFComparators_F16_TLE987_enableComparator(false);

    /* Variable to store the next state */
    Ifx_MHA_BEMFComparators_F16_TLE987_State nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_off;

    /* Return next state */
    return nextState;
}


static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_stateOff(
    Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* Variable to store the next state */
    Ifx_MHA_BEMFComparators_F16_TLE987_State nextState;

    /* Check enable */
    if (self->enable == true)
    {
        /* Enable HW comparators */
        Ifx_MHA_BEMFComparators_F16_TLE987_enableComparator(true);

        /* Go to debouncing */
        nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_debounce;
    }
    else
    {
        /* Stay in off state */
        nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_off;
    }

    /* Return next state */
    return nextState;
}


static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_stateDebounce(
    Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    Ifx_MHA_BEMFComparators_F16_TLE987_State nextState;
    nextState = Ifx_MHA_BEMFComparators_F16_TLE987_transitionStateDebounce(self);

    if (nextState == Ifx_MHA_BEMFComparators_F16_TLE987_State_debounce)
    {
        Ifx_MHA_BEMFComparators_F16_TLE987_doStateDebounce(self);
    }

    /* Return next state */
    return nextState;
}


static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_transitionStateDebounce(
    Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* Variable to store the next state */
    Ifx_MHA_BEMFComparators_F16_TLE987_State nextState;

    /* Check enable */
    if (self->enable == false)
    {
        /* Disable HW comparators */
        Ifx_MHA_BEMFComparators_F16_TLE987_enableComparator(false);

        /* Go to state off */
        nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_off;
    }
    else if (self->debounceCounter == IFX_MHA_BEMFCOMPARATORS_F16_TLE987_DEBOUNCE_CTR_MAX)
    {
        /* Set signal valid to true */
        self->status.isSignalValid = true;

        /* Go to positive On state */
        nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_onPos;
    }
    else if (self->debounceCounter == -IFX_MHA_BEMFCOMPARATORS_F16_TLE987_DEBOUNCE_CTR_MAX)
    {
        /* Set signal valid to true */
        self->status.isSignalValid = true;

        /* Go to negative On state */
        nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_onNeg;
    }
    else
    {
        /* Stay in debounce state */
        nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_debounce;
    }

    /* Return next state */
    return nextState;
}


static inline void Ifx_MHA_BEMFComparators_F16_TLE987_doStateDebounce(Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* Increment/decrement debounce counter on sector change */
    if (self->actualSector != self->oldSector)
    {
        if (self->actualSector == Ifx_MHA_BEMFComparators_F16_TLE987_incSector(self))
        {
            /* Increment debounce counter */
            self->debounceCounter = self->debounceCounter + 1;
        }
        else
        {
            /* Decrement debounce counter */
            self->debounceCounter = self->debounceCounter - 1;
        }
    }
}


static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_stateOn(
    Ifx_MHA_BEMFComparators_F16_TLE987* self, Ifx_MHA_BEMFComparators_F16_TLE987_State actualState)
{
    Ifx_MHA_BEMFComparators_F16_TLE987_State nextState;
    nextState = Ifx_MHA_BEMFComparators_F16_TLE987_transitionStateOn(self, actualState);

    if (nextState == actualState)
    {
        if (nextState == Ifx_MHA_BEMFComparators_F16_TLE987_State_onPos)
        {
            Ifx_MHA_BEMFComparators_F16_TLE987_doStateOnPos(self);
        }
        else
        {
            Ifx_MHA_BEMFComparators_F16_TLE987_doStateOnNeg(self);
        }
    }

    /* Return next state */
    return nextState;
}


static inline Ifx_MHA_BEMFComparators_F16_TLE987_State Ifx_MHA_BEMFComparators_F16_TLE987_transitionStateOn(
    Ifx_MHA_BEMFComparators_F16_TLE987* self, Ifx_MHA_BEMFComparators_F16_TLE987_State actualState)
{
    Ifx_MHA_BEMFComparators_F16_TLE987_State nextState;

    /* Check enable */
    if (self->enable == false)
    {
        Ifx_MHA_BEMFComparators_F16_TLE987_setSignalInvalid(self);

        /* Disable HW comparators */
        Ifx_MHA_BEMFComparators_F16_TLE987_enableComparator(false);

        /* Go to off state */
        nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_off;
    }
    else if ((self->debounceCounter == 0)
             || (self->standstillCounter == 0))
    {
        Ifx_MHA_BEMFComparators_F16_TLE987_setSignalInvalid(self);

        /* Reset debounce counter in case transition due to standstill */
        self->debounceCounter = 0;

        /* Reset standstill counter */
        self->standstillCounter = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_STANDSTILL_CTR_MAX;

        /* Go to debounce state */
        nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_debounce;
    }
    else
    {
        nextState = actualState;
    }

    return nextState;
}


static inline void Ifx_MHA_BEMFComparators_F16_TLE987_doStateOnPos(Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* On sector change */
    if (self->actualSector != self->oldSector)
    {
        if (self->actualSector == Ifx_MHA_BEMFComparators_F16_TLE987_incSector(self))
        {
            Ifx_MHA_BEMFComparators_F16_TLE987_readComparatorStatus(&(self->bemfComparatorState));

            /* We rotate in positive direction. Increment debounce counter */
            self->debounceCounter = self->debounceCounter + 1;

            /* Saturate debounce counter */
            if (self->debounceCounter > IFX_MHA_BEMFCOMPARATORS_F16_TLE987_DEBOUNCE_CTR_MAX)
            {
                self->debounceCounter = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_DEBOUNCE_CTR_MAX;
            }
        }
        else
        {
            /* False sector: */
            /* We rotate in negative direction. Decrement debounce counter */
            self->debounceCounter = self->debounceCounter - 1;
        }

        /* Reset standstill counter */
        self->standstillCounter = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_STANDSTILL_CTR_MAX;
    }
    else
    {
        /* Decrement standstill counter */
        self->standstillCounter = self->standstillCounter - 1;
    }
}


static inline void Ifx_MHA_BEMFComparators_F16_TLE987_doStateOnNeg(Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* On sector change */
    if (self->actualSector != self->oldSector)
    {
        if (self->actualSector == Ifx_MHA_BEMFComparators_F16_TLE987_decSector(self))
        {
            Ifx_MHA_BEMFComparators_F16_TLE987_readComparatorStatus(&(self->bemfComparatorState));

            /* Decrement debounce counter */
            self->debounceCounter = self->debounceCounter - 1;

            /* Saturate debounce counter */
            if (self->debounceCounter < -IFX_MHA_BEMFCOMPARATORS_F16_TLE987_DEBOUNCE_CTR_MAX)
            {
                self->debounceCounter = -IFX_MHA_BEMFCOMPARATORS_F16_TLE987_DEBOUNCE_CTR_MAX;
            }
        }
        else
        {
            /* False sector: */
            /* Increment debounce counter */
            self->debounceCounter = self->debounceCounter + 1;
        }

        /* Reset standstill counter */
        self->standstillCounter = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_STANDSTILL_CTR_MAX;
    }
    else
    {
        /* Decrement standstill counter */
        self->standstillCounter = self->standstillCounter - 1;
    }
}


static inline uint8_t Ifx_MHA_BEMFComparators_F16_TLE987_decSector(Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* Temp. variable to hold next sector */
    int16_t sector = (int16_t)self->oldSector - 1;

    /* Sector is only 0 to 5 */
    if (sector == -1)
    {
        sector = 5;
    }

    return (uint8_t)sector;
}


static inline void Ifx_MHA_BEMFComparators_F16_TLE987_readComparatorStatus(
    Ifx_Math_3PhaseFract16* bemfComparatorState)
{
    bemfComparatorState->u = (Ifx_Math_Fract16)(MF->BEMFC_CTRL_STS.bit.PHU_ZC_STS);
    bemfComparatorState->v = (Ifx_Math_Fract16)(MF->BEMFC_CTRL_STS.bit.PHV_ZC_STS);
    bemfComparatorState->w = (Ifx_Math_Fract16)(MF->BEMFC_CTRL_STS.bit.PHW_ZC_STS);
}


static inline void Ifx_MHA_BEMFComparators_F16_TLE987_setSignalInvalid(Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    self->status.isSignalValid  = false;
    self->bemfComparatorState.u = 0;
    self->bemfComparatorState.v = 0;
    self->bemfComparatorState.w = 0;
}


static inline void Ifx_MHA_BEMFComparators_F16_TLE987_updateSector(Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* Variable which holds comparators status as integer */
    uint32_t zeroCrossingStatus;

    /* Store actual sector */
    self->oldSector = self->actualSector;

    /* MF->BEMFC_CTRL_STS.reg 32 bit register
     * bits          description
     * 31:19         always read as 0
     * 18            phase W Comparator zero crossing status
     * 17            phase V Comparator zero crossing status
     * 16            phase U Comparator zero crossing status
     * 15:0          other fields */
    zeroCrossingStatus = ((MF->BEMFC_CTRL_STS.reg >> IFX_MHA_BEMFCOMPARATORS_F16_TLE987_ZC_SHIFT) &
                          (IFX_MHA_BEMFCOMPARATORS_F16_TLE987_ZC_MASK));

    /* Map comparator state to real sector */
    switch (zeroCrossingStatus)
    {
        case IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_0:
            self->actualSector = 0;
            break;

        case IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_1:
            self->actualSector = 1;
            break;

        case IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_2:
            self->actualSector = 2;
            break;

        case IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_3:
            self->actualSector = 3;
            break;

        case IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_4:
            self->actualSector = 4;
            break;

        case IFX_MHA_BEMFCOMPARATORS_F16_TLE987_SECTOR_5:
            self->actualSector = 5;
            break;

        default:
            self->actualSector = self->oldSector;
            break;
    }
}


/* polyspace-begin CODE-METRIC:VOCF [Justified:Low] "Function is setting 6 single bits there is no reduction in
 * complexity possible" */
static inline void Ifx_MHA_BEMFComparators_F16_TLE987_enableComparator(bool enable)
{
    /* Enable Comparators */
    MF->BEMFC_CTRL_STS.bit.PHUCOMP_EN = (uint8_t)enable;
    MF->BEMFC_CTRL_STS.bit.PHVCOMP_EN = (uint8_t)enable;
    MF->BEMFC_CTRL_STS.bit.PHWCOMP_EN = (uint8_t)enable;

    /* Switch Comparators On */
    MF->BEMFC_CTRL_STS.bit.PHUCOMP_ON = (uint8_t)enable;
    MF->BEMFC_CTRL_STS.bit.PHVCOMP_ON = (uint8_t)enable;
    MF->BEMFC_CTRL_STS.bit.PHWCOMP_ON = (uint8_t)enable;
}


/* polyspace-end CODE-METRIC:VOCF [Justified:Low] "Function is setting 6 single bits there is no reduction in complexity
 * possible" */
static inline uint8_t Ifx_MHA_BEMFComparators_F16_TLE987_incSector(Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* Temp. variable to hold next sector */
    uint8_t sector = self->oldSector + 1;

    /* Sector is only 0 to 5 */
    if (sector == 6)
    {
        sector = 0;
    }

    return sector;
}


void Ifx_MHA_BEMFComparators_F16_TLE987_init(Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* Init internal variables */
    self->enable            = false;
    self->status.state      = Ifx_MHA_BEMFComparators_F16_TLE987_State_init;
    self->oldSector         = 0;
    self->actualSector      = 0;
    self->debounceCounter   = 0;
    self->standstillCounter = IFX_MHA_BEMFCOMPARATORS_F16_TLE987_STANDSTILL_CTR_MAX;
    Ifx_MHA_BEMFComparators_F16_TLE987_setSignalInvalid(self);
}


void Ifx_MHA_BEMFComparators_F16_TLE987_execute(Ifx_MHA_BEMFComparators_F16_TLE987* self)
{
    /* Variable to store the previous state */
    Ifx_MHA_BEMFComparators_F16_TLE987_State actualState = self->status.state;

    /* Variable to store the next state */
    Ifx_MHA_BEMFComparators_F16_TLE987_State nextState;

    /* Update sector when not in off state */
    if (actualState != Ifx_MHA_BEMFComparators_F16_TLE987_State_off)
    {
        Ifx_MHA_BEMFComparators_F16_TLE987_updateSector(self);
    }

    switch (actualState)
    {
        /* State machine is in init state */
        case Ifx_MHA_BEMFComparators_F16_TLE987_State_init:
            nextState = Ifx_MHA_BEMFComparators_F16_TLE987_stateInit(self);
            break;

        /* State machine is in off state */
        case Ifx_MHA_BEMFComparators_F16_TLE987_State_off:
            nextState = Ifx_MHA_BEMFComparators_F16_TLE987_stateOff(self);
            break;

        /* State machine is in debounce state */
        case Ifx_MHA_BEMFComparators_F16_TLE987_State_debounce:
            nextState = Ifx_MHA_BEMFComparators_F16_TLE987_stateDebounce(self);
            break;

        /* State machine is in On positive rotation state */
        case Ifx_MHA_BEMFComparators_F16_TLE987_State_onPos:
            nextState = Ifx_MHA_BEMFComparators_F16_TLE987_stateOn(self, actualState);
            break;

        /* State machine is in On negative rotation state */
        case Ifx_MHA_BEMFComparators_F16_TLE987_State_onNeg:
            nextState = Ifx_MHA_BEMFComparators_F16_TLE987_stateOn(self, actualState);
            break;

        /* do default transition to init */
        default:
            nextState = Ifx_MHA_BEMFComparators_F16_TLE987_State_init;
            break;
    }

    /* Update state and reset clear fault */
    self->status.state = nextState;
}
