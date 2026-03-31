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
#include <string.h>
#include "Ifx_FRW_ParamTable.h"
#include "Ifx_FRW_ParamTable_Cfg.h"
#include "cmsis_compiler.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Macros to define the component ID */
#define IFX_FRW_PARAMTABLE_COMPONENTID_SOURCEID     ((uint8_t)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_FRW_PARAMTABLE_COMPONENTID_LIBRARYID    ((uint16_t) Ifx_ComponentID_LibraryID_framework)
#define IFX_FRW_PARAMTABLE_COMPONENTID_MODULEID     (0U)
#define IFX_FRW_PARAMTABLE_COMPONENTID_COMPONENTID1 (0U)
#define IFX_FRW_PARAMTABLE_COMPONENTID_COMPONENTID2 ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_FRW_PARAMTABLE_COMPONENTVERSION_MAJOR   (1U)
#define IFX_FRW_PARAMTABLE_COMPONENTVERSION_MINOR   (0U)
#define IFX_FRW_PARAMTABLE_COMPONENTVERSION_PATCH   (0U)
#define IFX_FRW_PARAMTABLE_COMPONENTVERSION_T       (4U)
#define IFX_FRW_PARAMTABLE_COMPONENTVERSION_REV     (0U)

/* Component ID */
const Ifx_ComponentID      Ifx_FRW_paramTable_componentID = {
    .sourceID     = IFX_FRW_PARAMTABLE_COMPONENTID_SOURCEID,
    .libraryID    = IFX_FRW_PARAMTABLE_COMPONENTID_LIBRARYID,
    .moduleID     = IFX_FRW_PARAMTABLE_COMPONENTID_MODULEID,
    .componentID1 = IFX_FRW_PARAMTABLE_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_FRW_PARAMTABLE_COMPONENTID_COMPONENTID2,
};

/* Component Version */
const Ifx_ComponentVersion Ifx_FRW_paramTable_componentVersion = {
    .major = IFX_FRW_PARAMTABLE_COMPONENTVERSION_MAJOR,
    .minor = IFX_FRW_PARAMTABLE_COMPONENTVERSION_MINOR,
    .patch = IFX_FRW_PARAMTABLE_COMPONENTVERSION_PATCH,
    .t     = IFX_FRW_PARAMTABLE_COMPONENTVERSION_T,
    .rev   = IFX_FRW_PARAMTABLE_COMPONENTVERSION_REV
};
/* *INDENT-ON* */

/* Cast a void pointer element back into a specific type and then convert it into int64_t so that signedness is handled
 * correctly and memory range is respected.  */
#define IFX_FRW_PARAMTABLE_CAST_TO_I64(IFX_TYPE, IFX_POINTER) ((int64_t)(*((IFX_TYPE*)IFX_POINTER)))

/* polyspace +11 MISRA-C3:11.1 [Justified:Low] "Purpose of following macros is to cast functions back into there
 * original type using a type configuration in paramEntry. Then call them to access a parameter in the system.
 * Only alternative would be generating switch structures mentioning get and set functions explicitly.
 * But this is currently not supported by paramTable generator." */
#define IFX_FRW_PARAMTABLE_GET_SELF(IFX_GETFUNC_SELF, IFX_PARAMENTRY) \
    ((int64_t)((IFX_GETFUNC_SELF)IFX_PARAMENTRY.getParam)(IFX_PARAMENTRY.instance))
#define IFX_FRW_PARAMTABLE_GET(IFX_GETFUNC, IFX_PARAMENTRY) \
    ((int64_t)((IFX_GETFUNC)paramEntry.getParam)())
#define IFX_FRW_PARAMTABLE_SET_SELF(IFX_SETFUNC_SELF, IFX_PARAMENTRY, IFX_VALUE) \
    ((IFX_SETFUNC_SELF)IFX_PARAMENTRY.setParam)(IFX_PARAMENTRY.instance, IFX_VALUE)
#define IFX_FRW_PARAMTABLE_SET(IFX_SETFUNC, IFX_PARAMENTRY, IFX_VALUE) \
    ((IFX_SETFUNC)IFX_PARAMENTRY.setParam)(IFX_VALUE)

/******************************************************************************/
/*------------------------------Type Definitions------------------------------*/
/******************************************************************************/
/* Function pointer types to handle get functions of multi instance components */
typedef int8_t   (* getFunc_self_i8)(void* self);
typedef int16_t  (* getFunc_self_i16)(void* self);
typedef int32_t  (* getFunc_self_i32)(void* self);
typedef uint8_t  (* getFunc_self_u8)(void* self);
typedef uint16_t (* getFunc_self_u16)(void* self);
typedef uint32_t (* getFunc_self_u32)(void* self);

/* Function pointer types to handle simple get functions */
typedef int8_t   (* getFunc_i8)(void);
typedef int16_t  (* getFunc_i16)(void);
typedef int32_t  (* getFunc_i32)(void);
typedef uint8_t  (* getFunc_u8)(void);
typedef uint16_t (* getFunc_u16)(void);
typedef uint32_t (* getFunc_u32)(void);

/* Function pointer types to handle set functions of multi instance components  */
typedef void (* setFunc_self_u8)(void* self, uint8_t value_DU);
typedef void (* setFunc_self_u16)(void* self, uint16_t value_DU);
typedef void (* setFunc_self_u32)(void* self, uint32_t value_DU);
typedef void (* setFunc_self_i8)(void* self, int8_t value_DU);
typedef void (* setFunc_self_i16)(void* self, int16_t value_DU);
typedef void (* setFunc_self_i32)(void* self, int32_t value_DU);

/* Function pointer types to handle simple set functions  */
typedef void (* setFunc_u8)(uint8_t value_DU);
typedef void (* setFunc_u16)(uint16_t value_DU);
typedef void (* setFunc_u32)(uint32_t value_DU);
typedef void (* setFunc_i8)(int8_t value_DU);
typedef void (* setFunc_i16)(int16_t value_DU);
typedef void (* setFunc_i32)(int32_t value_DU);

/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/

/**
 * \brief Extracts the 16bit parameter index from the 24bit target index
 **/
static uint16_t Ifx_FRW_ParamTable_paramIndex(uint32_t const targetIdx);

/**
 * \brief Extracts the 8bit subindex from the 24bit target index
 **/
static uint8_t Ifx_FRW_ParamTable_getSubIndex(uint32_t const targetIdx);

/**
 * \brief Searches the paramEntry pointer with the target index  and stores it in paramEntry pointer.
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_searchParamTable(uint16_t lowLimit, uint16_t highLimit, uint16_t
                                                                     targetIndex,
                                                                     Ifx_FRW_ParamTable_paramEntry* pEntry);

/**
 * \brief Check if the subIndex exceeds maxSubIndex of paramEntry
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_checkSubIndex(uint8_t subIndex, uint8_t maxSubIndex);

/**
 * \brief Get the valueDU for a parameter considering all paramEntry settings.
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_getValueDU(Ifx_FRW_ParamTable_paramEntry paramEntry,
                                                               int64_t                     * valueDU);

/**
 * \brief Calls the specific Get function of the parameter
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_getValueDU_getParam(Ifx_FRW_ParamTable_paramEntry paramEntry,
                                                                        int64_t                     * valueDU);

/**
 * \brief Calls the specific Get function of the parameter for multi instance components that require a self pointer.
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_getValueDU_getParamInstance(Ifx_FRW_ParamTable_paramEntry
                                                                                paramEntry, int64_t* valueDU);

/**
 * \brief Calls the specific Get function of the parameter for not instantiable components.
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_getValuDU_getParamSingleton(Ifx_FRW_ParamTable_paramEntry
                                                                                paramEntry, int64_t* valueDU);

/**
 *
 * \brief Returns scaled value in transfer unit at result pointer
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_scaleDUtoTU(const Ifx_FRW_ParamTable_paramEntry* paramEntry,
                                                                int64_t* pResult, int64_t inputValueDU);

/**
 * \brief Returns scaled value in digital unit at result pointer
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_scaleTUtoDU(void* inputBuffer, const
                                                                Ifx_FRW_ParamTable_paramEntry* paramEntry,
                                                                int64_t* pResult);

/**
 * \brief Resolve and return parameter ranges based on the paramEntry
 */
static Ifx_FRW_ParamTable_paramRanges Ifx_FRW_ParamTable_getParamRanges(const
                                                                        Ifx_FRW_ParamTable_paramEntry* paramEntry);

/**
 * \brief Check if scaling between DU and TU is required for this parameter
 */
static inline bool Ifx_FRW_ParamTable_checkIfScalingIsRequired(Ifx_FRW_ParamTable_paramRanges paramRanges);

/**
 * \brief Calculate the fraction parts needed for scaling a parameter from TU to DU
 */
static Ifx_FRW_ParamTable_fractionParts Ifx_FRW_ParamTable_calcFractionPartsTUtoDU(Ifx_FRW_ParamTable_paramRanges
                                                                                   paramRanges, int64_t inputValueTU);

/**
 * \brief Calculate the fraction parts needed for scaling a parameter from DU to TU
 */
static Ifx_FRW_ParamTable_fractionParts Ifx_FRW_ParamTable_calcFractionPartsDUtoTU(Ifx_FRW_ParamTable_paramRanges
                                                                                   paramRanges, int64_t inputValueDU);

/**
 * \brief Check whether the result of the multiplication of both inputs fits into int64_t
 */
static inline bool Ifx_FRW_ParamTable_checkIfResultFitsIntoInt64(int64_t numerator1, int64_t numerator2);

/**
 * \brief Calculates the scaling of a fraction with rounding.
 *
 * \note The function only rounds if it is possible without overflowing the int64_t data type.
 */
static int64_t Ifx_FRW_ParamTable_calcScalingWithRounding(Ifx_FRW_ParamTable_fractionParts fractionParts, int64_t
                                                          min_TU);

/**
 * \brief Casts input element to int64_t according to datatype defined by signed and elementSize
 */
static int64_t Ifx_FRW_ParamTable_castToInt64(void* element, bool isSigned, uint8_t elementSize);

/**
 * \brief Casts input byte element to int64_t according to isSigned.
 */
static int64_t Ifx_FRW_ParamTable_castByteToInt64(void* element, bool isSigned);

/**
 * \brief Casts input word element to int64_t according to isSigned.
 */
static int64_t Ifx_FRW_ParamTable_castWordToInt64(void* element, bool isSigned);

/**
 * \brief Casts input double word element to int64_t according to isSigned.
 */
static int64_t Ifx_FRW_ParamTable_castDWordToInt64(void* element, bool isSigned);

/**
 * \brief Checks if inputBuffer is within min and max of transfer unit and returns specific error type
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_checkMinMaxTU(const Ifx_FRW_ParamTable_paramEntry* paramEntry,
                                                                  int64_t                              inputValue);

/**
 * \brief Checks if inputBuffer is within min and max of digital unit and returns specific error type
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_checkMinMaxDU(const Ifx_FRW_ParamTable_paramEntry* paramEntry,
                                                                  int64_t                              inputValue);

/**
 * \brief Calls the specific Set function of the parameter
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_setValueDU_setParam(Ifx_FRW_ParamTable_paramEntry paramEntry,
                                                                        int64_t                       valueDU);

/**
 * \brief Calls the specific Set function of the parameter for multi instance components that require a self pointer.
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_setValueDU_setParamInstance(Ifx_FRW_ParamTable_paramEntry
                                                                                paramEntry, int64_t valueDU);

/**
 * \brief Calls the specific Set function of the parameter for not instantiable components.
 **/
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_setValueDU_setParamSingleton(Ifx_FRW_ParamTable_paramEntry
                                                                                 paramEntry, int64_t valueDU);

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_readParam(uint32_t targetIndex, uint8_t* outputBuffer)
{
    Ifx_FRW_ParamTable_Errors     responseCode = Ifx_FRW_ParamTable_ERR_Error;
    Ifx_FRW_ParamTable_paramEntry paramEntry;
    uint16_t                      paramIndex   = Ifx_FRW_ParamTable_paramIndex(targetIndex);
    uint8_t                       subIndex     = Ifx_FRW_ParamTable_getSubIndex(targetIndex);
    int64_t                       valueDU;
    int64_t                       valueTU;

    /* Set outputBuffer to zero */
    for (uint16_t i = 0; i < IFX_FRW_PARAMTABLE_MAX_NUMBYTES; i++)
    {
        outputBuffer[i] = 0;
    }

    /* Get entry from paramTable by binary search */
    responseCode = Ifx_FRW_ParamTable_searchParamTable(0, IFX_FRW_PARAMTABLE_NUMPARAM - 1, paramIndex, &paramEntry);

    /* Run parameter attribute checks */
    if (responseCode == Ifx_FRW_ParamTable_ERR_NoError)
    {
        responseCode = Ifx_FRW_ParamTable_checkSubIndex(subIndex, paramEntry.maxSubIndex);
    }

    /* Retrieve the value from the component */
    if (responseCode == Ifx_FRW_ParamTable_ERR_NoError)
    {
        responseCode = Ifx_FRW_ParamTable_getValueDU(paramEntry, &valueDU);
    }

    /* scale value from digital units to transfer units */
    if (responseCode == Ifx_FRW_ParamTable_ERR_NoError)
    {
        responseCode = Ifx_FRW_ParamTable_scaleDUtoTU(&paramEntry, &valueTU, valueDU);
    }

    /* Store valueTU in outputBuffer */
    if (responseCode == Ifx_FRW_ParamTable_ERR_NoError)
    {
        uint8_t bytes = paramEntry.bits.sizeTU + 1U;
        memcpy((void*)outputBuffer, (void*)&valueTU, bytes);
    }

    return responseCode;
}


static uint16_t Ifx_FRW_ParamTable_paramIndex(uint32_t const targetIdx)
{
    return (uint16_t)(((targetIdx) >> 8u) & 0xFFFFu);
}


static uint8_t Ifx_FRW_ParamTable_getSubIndex(uint32_t const targetIdx)
{
    return (uint8_t)(targetIdx & 0xFFu);
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_searchParamTable(uint16_t lowLimit, uint16_t highLimit, uint16_t
                                                                     targetIndex,
                                                                     Ifx_FRW_ParamTable_paramEntry* pEntry)
{
    uint16_t                  mid          = 0;
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_ParamNotFoundError;

    while (lowLimit <= highLimit)
    {
        mid = lowLimit + (highLimit - lowLimit) / 2u;

        if (Ifx_FRW_ParamTable_Table[mid].index == targetIndex)
        {
            responseCode = Ifx_FRW_ParamTable_ERR_NoError;
            *pEntry      = Ifx_FRW_ParamTable_Table[mid];
            break;
        }
        else
        {
            if (Ifx_FRW_ParamTable_Table[mid].index < targetIndex)
            {
                lowLimit = mid + 1u;
            }
            else
            {
                highLimit = mid - 1u;
            }
        }
    }

    return responseCode;
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_checkSubIndex(uint8_t subIndex, uint8_t maxSubIndex)
{
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_Error;

    if (subIndex > maxSubIndex)
    {
        responseCode = Ifx_FRW_ParamTable_ERR_SubIndexUnkown;
    }
    else
    {
        responseCode = Ifx_FRW_ParamTable_ERR_NoError;
    }

    return responseCode;
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_getValueDU(Ifx_FRW_ParamTable_paramEntry paramEntry,
                                                               int64_t                     * valueDU)
{
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_NoError;

    if (paramEntry.getParam != NULL)
    {
        /* Read the parameter value by calling the get-function in getParam. */
        responseCode = Ifx_FRW_ParamTable_getValueDU_getParam(paramEntry, valueDU);
    }
    else
    {
        /* Copy the value at the given address for this parameter to the outputBuffer (elementSize is given 0 =
         * 1byte
         * ... 3= 4byte) */
        *valueDU = Ifx_FRW_ParamTable_castToInt64(paramEntry.address, paramEntry.bits.isSignedDU,
            paramEntry.bits.sizeDU);
    }

    return responseCode;
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_getValueDU_getParam(Ifx_FRW_ParamTable_paramEntry paramEntry,
                                                                        int64_t                     * valueDU)
{
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_NoError;

    if (paramEntry.bits.isFloat)
    {
        responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
    }
    else
    {
        if (paramEntry.bits.multiInstance == true)
        {
            /* Call get with self ptr */
            responseCode = Ifx_FRW_ParamTable_getValueDU_getParamInstance(paramEntry, valueDU);
        }
        else
        {
            /* Call get without self ptr */
            responseCode = Ifx_FRW_ParamTable_getValuDU_getParamSingleton(paramEntry, valueDU);
        }
    }

    return responseCode;
}


/* polyspace-begin CODE-METRIC:VOCF [Justified:Low] "Handling of parameter attribute options leads to large but still
 * simple and repetitive structures. Further splitting of this option handling functions would reduce readability again
 * and kill the benefit of this metric which is improved readability and maintainability." */

/* polyspace-begin DEFECT:FUNC_CAST [Justified:Low] "See justification MISRA-C3:11.1" */
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_getValueDU_getParamInstance(Ifx_FRW_ParamTable_paramEntry
                                                                                paramEntry, int64_t* valueDU)
{
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_NoError;

    if (paramEntry.bits.isSignedDU == true)
    {
        switch (paramEntry.bits.sizeDU)
        {
            case 0:
                *valueDU = IFX_FRW_PARAMTABLE_GET_SELF(getFunc_self_i8, paramEntry);
                break;

            case 1:
                *valueDU = IFX_FRW_PARAMTABLE_GET_SELF(getFunc_self_i16, paramEntry);
                break;

            case 3:
                *valueDU = IFX_FRW_PARAMTABLE_GET_SELF(getFunc_self_i32, paramEntry);
                break;

            default:
                responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
                break;
        }
    }
    else
    {
        /* unsigned with self */
        switch (paramEntry.bits.sizeDU)
        {
            case 0:
                *valueDU = IFX_FRW_PARAMTABLE_GET_SELF(getFunc_self_u8, paramEntry);
                break;

            case 1:
                *valueDU = IFX_FRW_PARAMTABLE_GET_SELF(getFunc_self_u16, paramEntry);
                break;

            case 3:
                *valueDU = IFX_FRW_PARAMTABLE_GET_SELF(getFunc_self_u32, paramEntry);
                break;

            default:
                responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
                break;
        }
    }

    return responseCode;
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_getValuDU_getParamSingleton(Ifx_FRW_ParamTable_paramEntry
                                                                                paramEntry, int64_t* valueDU)
{
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_NoError;

    if (paramEntry.bits.isSignedDU == true)
    {
        switch (paramEntry.bits.sizeDU)
        {
            case 0:
                *valueDU = IFX_FRW_PARAMTABLE_GET(getFunc_i8, paramEntry);
                break;

            case 1:
                *valueDU = IFX_FRW_PARAMTABLE_GET(getFunc_i16, paramEntry);
                break;

            case 3:
                *valueDU = IFX_FRW_PARAMTABLE_GET(getFunc_i32, paramEntry);
                break;

            default:
                responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
                break;
        }
    }
    else
    {
        /* unsigned without self ptr */
        switch (paramEntry.bits.sizeDU)
        {
            case 0:
                *valueDU = IFX_FRW_PARAMTABLE_GET(getFunc_u8, paramEntry);
                break;

            case 1:
                *valueDU = IFX_FRW_PARAMTABLE_GET(getFunc_u16, paramEntry);
                break;

            case 3:
                *valueDU = IFX_FRW_PARAMTABLE_GET(getFunc_u32, paramEntry);
                break;

            default:
                responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
                break;
        }
    }

    return responseCode;
}


/* polyspace-end DEFECT:FUNC_CAST [Justified:Low] "See justification MISRA-C3:11.1" */
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_scaleDUtoTU(const Ifx_FRW_ParamTable_paramEntry* paramEntry,
                                                                int64_t* pResult, int64_t inputValueDU)
{
    Ifx_FRW_ParamTable_Errors        responseCode = Ifx_FRW_ParamTable_ERR_NoError;
    Ifx_FRW_ParamTable_fractionParts fractionParts;
    Ifx_FRW_ParamTable_paramRanges   paramRanges  = Ifx_FRW_ParamTable_getParamRanges(paramEntry);
    *pResult = inputValueDU;

    if (Ifx_FRW_ParamTable_checkIfScalingIsRequired(paramRanges))
    {
        fractionParts = Ifx_FRW_ParamTable_calcFractionPartsDUtoTU(paramRanges, inputValueDU);

        if (fractionParts.denominator == 0)
        {
            responseCode = Ifx_FRW_ParamTable_ERR_DenominatorZero;
        }
        else if (Ifx_FRW_ParamTable_checkIfResultFitsIntoInt64(fractionParts.numerator1, fractionParts.numerator2))
        {
            *pResult = Ifx_FRW_ParamTable_calcScalingWithRounding(fractionParts, paramRanges.min_TU);
        }
        else
        {
            *pResult     = paramRanges.max_TU;
            responseCode = Ifx_FRW_ParamTable_ERR_NumeratorsOverflow;
        }
    }

    return responseCode;
}


Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_writeParam(uint32_t targetIndex, uint8_t* inputBuffer, bool motorRunning)
{
    Ifx_FRW_ParamTable_Errors     responseCode = Ifx_FRW_ParamTable_ERR_NoError;
    Ifx_FRW_ParamTable_paramEntry paramEntry;
    int64_t                       writeValue;
    uint16_t                      paramIndex = Ifx_FRW_ParamTable_paramIndex(targetIndex);
    uint8_t                       subIndex   = Ifx_FRW_ParamTable_getSubIndex(targetIndex);

    /* Get entry from paramTable by binary search */
    responseCode = Ifx_FRW_ParamTable_searchParamTable(0, IFX_FRW_PARAMTABLE_NUMPARAM - 1, paramIndex, &paramEntry);

    if (responseCode == Ifx_FRW_ParamTable_ERR_NoError)
    {
        responseCode = Ifx_FRW_ParamTable_checkSubIndex(subIndex, paramEntry.maxSubIndex);
    }

    if (responseCode == Ifx_FRW_ParamTable_ERR_NoError)
    {
        responseCode = Ifx_FRW_ParamTable_scaleTUtoDU(inputBuffer, &paramEntry, &writeValue);
    }

    if ((paramEntry.bits.writeAccess != true)
        && (responseCode == Ifx_FRW_ParamTable_ERR_NoError))
    {
        responseCode = Ifx_FRW_ParamTable_ERR_ReadOnly;
    }
    else if ((motorRunning == true)
             && (paramEntry.bits.changeWhileMotorRunning == false))
    {
        responseCode = Ifx_FRW_ParamTable_ERR_MotorRunning;
    }
    else
    {
        /* we keep potential errors from before */
    }

    if (responseCode == Ifx_FRW_ParamTable_ERR_NoError)
    {
        if (paramEntry.setParam != NULL)
        {
            /* Pass the inputBuffer to the given set-function for this parameter */
            responseCode = Ifx_FRW_ParamTable_setValueDU_setParam(paramEntry, writeValue);
        }
        else
        {
            uint8_t bytes = paramEntry.bits.sizeDU + 1U;

            /* Copy inputBuffer to parameter value */
            memcpy((void*)paramEntry.address, (void*)&writeValue, bytes);
            responseCode = Ifx_FRW_ParamTable_ERR_NoError;
        }
    }

    return responseCode;
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_scaleTUtoDU(void* inputBuffer, const
                                                                Ifx_FRW_ParamTable_paramEntry* paramEntry,
                                                                int64_t* pResult)
{
    Ifx_FRW_ParamTable_Errors      responseCode;
    int64_t                        resultTemp;
    int64_t                        inputValueTU;
    Ifx_FRW_ParamTable_paramRanges paramRanges = Ifx_FRW_ParamTable_getParamRanges(paramEntry);
    *pResult     = (*(int64_t*)inputBuffer);
    inputValueTU = Ifx_FRW_ParamTable_castToInt64(inputBuffer, paramEntry->bits.isSignedTU, paramEntry->bits.sizeTU);
    responseCode = Ifx_FRW_ParamTable_checkMinMaxTU(paramEntry, inputValueTU);

    if ((responseCode == Ifx_FRW_ParamTable_ERR_NoError)
        && Ifx_FRW_ParamTable_checkIfScalingIsRequired(paramRanges))
    {
        Ifx_FRW_ParamTable_fractionParts fractionParts = Ifx_FRW_ParamTable_calcFractionPartsTUtoDU(paramRanges,
            inputValueTU);

        if (fractionParts.denominator == 0)
        {
            responseCode = Ifx_FRW_ParamTable_ERR_DenominatorZero;
        }
        else if (Ifx_FRW_ParamTable_checkIfResultFitsIntoInt64(fractionParts.numerator1, fractionParts.numerator2))
        {
            resultTemp   = (fractionParts.numerator1 * fractionParts.numerator2);
            resultTemp   = resultTemp / fractionParts.denominator;
            resultTemp   = resultTemp + paramRanges.min_DU;
            responseCode = Ifx_FRW_ParamTable_checkMinMaxDU(paramEntry, resultTemp);
            *pResult     = resultTemp;
        }
        else
        {
            responseCode = Ifx_FRW_ParamTable_ERR_NumeratorsOverflow;
        }
    }
    else
    {
        /* skip scaling */
    }

    return responseCode;
}


static Ifx_FRW_ParamTable_paramRanges Ifx_FRW_ParamTable_getParamRanges(const
                                                                        Ifx_FRW_ParamTable_paramEntry* paramEntry)
{
    Ifx_FRW_ParamTable_paramRanges paramRanges;

    /* use uint32 max value if max value is too big for int32 */
    if (paramEntry->bits.isSignedDU)
    {
        paramRanges.min_DU = paramEntry->sMin_DU;
        paramRanges.max_DU = paramEntry->sMax_DU;
    }
    else
    {
        paramRanges.min_DU = (int64_t)paramEntry->uMin_DU;
        paramRanges.max_DU = (int64_t)paramEntry->uMax_DU;
    }

    if (paramEntry->bits.isSignedTU)
    {
        paramRanges.min_TU = paramEntry->sMin_TU;
        paramRanges.max_TU = paramEntry->sMax_TU;
    }
    else
    {
        paramRanges.min_TU = (int64_t)paramEntry->uMin_TU;
        paramRanges.max_TU = (int64_t)paramEntry->uMax_TU;
    }

    return paramRanges;
}


static inline bool Ifx_FRW_ParamTable_checkIfScalingIsRequired(Ifx_FRW_ParamTable_paramRanges paramRanges)
{
    bool scalingIsRequired;

    if ((paramRanges.max_TU != paramRanges.max_DU)
        || (paramRanges.min_TU != paramRanges.min_DU))
    {
        scalingIsRequired = true;
    }
    else
    {
        scalingIsRequired = false;
    }

    return scalingIsRequired;
}


static Ifx_FRW_ParamTable_fractionParts Ifx_FRW_ParamTable_calcFractionPartsTUtoDU(Ifx_FRW_ParamTable_paramRanges
                                                                                   paramRanges, int64_t inputValueTU)
{
    Ifx_FRW_ParamTable_fractionParts fractionParts;
    fractionParts.numerator1  = paramRanges.max_DU - paramRanges.min_DU;
    fractionParts.numerator2  = inputValueTU - paramRanges.min_TU;
    fractionParts.denominator = paramRanges.max_TU - paramRanges.min_TU;

    return fractionParts;
}


static Ifx_FRW_ParamTable_fractionParts Ifx_FRW_ParamTable_calcFractionPartsDUtoTU(Ifx_FRW_ParamTable_paramRanges
                                                                                   paramRanges, int64_t inputValueDU)
{
    Ifx_FRW_ParamTable_fractionParts fractionParts;
    fractionParts.numerator1  = paramRanges.max_TU - paramRanges.min_TU;
    fractionParts.numerator2  = inputValueDU - paramRanges.min_DU;
    fractionParts.denominator = paramRanges.max_DU - paramRanges.min_DU;

    return fractionParts;
}


static inline bool Ifx_FRW_ParamTable_checkIfResultFitsIntoInt64(int64_t numerator1, int64_t numerator2)
{
    bool resultFitsIntoInt64;

    if ((numerator1 == 0)
        || (numerator2 == 0))
    {
        resultFitsIntoInt64 = true;
    }
    else
    {
        if (numerator1 > (INT64_MAX / numerator2))
        {
            resultFitsIntoInt64 = false;
        }
        else if (numerator1 < (INT64_MIN / numerator2))
        {
            resultFitsIntoInt64 = false;
        }
        else
        {
            resultFitsIntoInt64 = true;
        }
    }

    return resultFitsIntoInt64;
}


static int64_t Ifx_FRW_ParamTable_calcScalingWithRounding(Ifx_FRW_ParamTable_fractionParts fractionParts, int64_t
                                                          min_TU)
{
    int64_t roundingOffset = (fractionParts.denominator / 2);
    int64_t resultTemp     = (fractionParts.numerator1 * fractionParts.numerator2);

    /* Scale with rounding requires to add an offset for positive input values or subtract it for negative
     * values before division */
    if ((resultTemp < 0)
        && (resultTemp >= (INT64_MIN + roundingOffset)))
    {
        resultTemp = (resultTemp - roundingOffset) / fractionParts.denominator;
    }
    else if ((resultTemp > 0)
             && (resultTemp <= (INT64_MAX - roundingOffset)))
    {
        resultTemp = (resultTemp + roundingOffset) / fractionParts.denominator;
    }
    else
    {
        resultTemp = resultTemp / fractionParts.denominator;
    }

    resultTemp = resultTemp + min_TU;

    return resultTemp;
}


static int64_t Ifx_FRW_ParamTable_castToInt64(void* element, bool isSigned, uint8_t elementSize)
{
    int64_t result = 0;

    switch (elementSize)
    {
        case 0:
            result = Ifx_FRW_ParamTable_castByteToInt64(element, isSigned);
            break;

        case 1:
            result = Ifx_FRW_ParamTable_castWordToInt64(element, isSigned);
            break;

        /* Because of the uint32_t case, result must be int64_t*/
        case 3:
            result = Ifx_FRW_ParamTable_castDWordToInt64(element, isSigned);
            break;

        default:
            result = 0;
            break;
    }

    return result;
}


static int64_t Ifx_FRW_ParamTable_castByteToInt64(void* element, bool isSigned)
{
    return (isSigned == true) ? IFX_FRW_PARAMTABLE_CAST_TO_I64(int8_t, element) : IFX_FRW_PARAMTABLE_CAST_TO_I64(
        uint8_t, element);
}


static int64_t Ifx_FRW_ParamTable_castWordToInt64(void* element, bool isSigned)
{
    return (isSigned == true) ? IFX_FRW_PARAMTABLE_CAST_TO_I64(int16_t, element) : IFX_FRW_PARAMTABLE_CAST_TO_I64(
        uint16_t, element);
}


static int64_t Ifx_FRW_ParamTable_castDWordToInt64(void* element, bool isSigned)
{
    return (isSigned == true) ? IFX_FRW_PARAMTABLE_CAST_TO_I64(int32_t, element) : IFX_FRW_PARAMTABLE_CAST_TO_I64(
        uint32_t, element);
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_checkMinMaxTU(const Ifx_FRW_ParamTable_paramEntry* paramEntry,
                                                                  int64_t                              inputValue)
{
    Ifx_FRW_ParamTable_Errors responseCode;

    if (paramEntry->bits.isSignedTU)
    {
        if (inputValue > paramEntry->sMax_TU)
        {
            responseCode = Ifx_FRW_ParamTable_ERR_ValueTooHigh;
        }
        else
        {
            if (inputValue < paramEntry->sMin_TU)
            {
                responseCode = Ifx_FRW_ParamTable_ERR_ValueTooLow;
            }
            else
            {
                /* No error, value can be scaled to DU */
                responseCode = Ifx_FRW_ParamTable_ERR_NoError;
            }
        }
    }
    else
    {
        if ((uint32_t)inputValue > paramEntry->uMax_TU)
        {
            responseCode = Ifx_FRW_ParamTable_ERR_ValueTooHigh;
        }
        else
        {
            if ((uint32_t)inputValue < paramEntry->uMin_TU)
            {
                responseCode = Ifx_FRW_ParamTable_ERR_ValueTooLow;
            }
            else
            {
                /* No error, value can be scaled to DU */
                responseCode = Ifx_FRW_ParamTable_ERR_NoError;
            }
        }
    }

    return responseCode;
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_checkMinMaxDU(const Ifx_FRW_ParamTable_paramEntry* paramEntry,
                                                                  int64_t                              inputValue)
{
    Ifx_FRW_ParamTable_Errors responseCode;

    if (paramEntry->bits.isSignedDU)
    {
        if (inputValue > paramEntry->sMax_DU)
        {
            responseCode = Ifx_FRW_ParamTable_ERR_ValueTooHigh;
        }
        else
        {
            if (inputValue < paramEntry->sMin_DU)
            {
                responseCode = Ifx_FRW_ParamTable_ERR_ValueTooLow;
            }
            else
            {
                /* No error, value can be scaled to DU */
                responseCode = Ifx_FRW_ParamTable_ERR_NoError;
            }
        }
    }
    else
    {
        if ((uint32_t)inputValue > paramEntry->uMax_DU)
        {
            responseCode = Ifx_FRW_ParamTable_ERR_ValueTooHigh;
        }
        else
        {
            if ((uint32_t)inputValue < paramEntry->uMin_DU)
            {
                responseCode = Ifx_FRW_ParamTable_ERR_ValueTooLow;
            }
            else
            {
                /* No error, value can be scaled to DU */
                responseCode = Ifx_FRW_ParamTable_ERR_NoError;
            }
        }
    }

    return responseCode;
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_setValueDU_setParam(Ifx_FRW_ParamTable_paramEntry paramEntry,
                                                                        int64_t                       valueDU)
{
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_NoError;

    if (paramEntry.bits.isFloat)
    {
        responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
    }
    else
    {
        if (paramEntry.bits.multiInstance == true)
        {
            responseCode = Ifx_FRW_ParamTable_setValueDU_setParamInstance(paramEntry, valueDU);
        }
        else
        {
            responseCode = Ifx_FRW_ParamTable_setValueDU_setParamSingleton(paramEntry, valueDU);
        }
    }

    return responseCode;
}


/* polyspace-begin DEFECT:FUNC_CAST [Justified:Low] "See justification MISRA-C3:11.1" */
static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_setValueDU_setParamInstance(Ifx_FRW_ParamTable_paramEntry
                                                                                paramEntry, int64_t valueDU)
{
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_NoError;

    if (paramEntry.bits.isSignedDU == true)
    {
        /* signed with self */
        switch (paramEntry.bits.sizeDU)
        {
            case 0:
                IFX_FRW_PARAMTABLE_SET_SELF(setFunc_self_i8, paramEntry, (int8_t)valueDU);
                break;

            case 1:
                IFX_FRW_PARAMTABLE_SET_SELF(setFunc_self_i16, paramEntry, (int16_t)valueDU);
                break;

            case 3:
                IFX_FRW_PARAMTABLE_SET_SELF(setFunc_self_i32, paramEntry, (int32_t)valueDU);
                break;

            default:
                responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
                break;
        }
    }
    else
    {
        /* unsigned with self */
        switch (paramEntry.bits.sizeDU)
        {
            case 0:
                IFX_FRW_PARAMTABLE_SET_SELF(setFunc_self_u8, paramEntry, (uint8_t)valueDU);
                break;

            case 1:
                IFX_FRW_PARAMTABLE_SET_SELF(setFunc_self_u16, paramEntry, (uint16_t)valueDU);
                break;

            case 3:
                IFX_FRW_PARAMTABLE_SET_SELF(setFunc_self_u32, paramEntry, (uint32_t)valueDU);
                break;

            default:
                responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
                break;
        }
    }

    return responseCode;
}


static Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_setValueDU_setParamSingleton(Ifx_FRW_ParamTable_paramEntry
                                                                                 paramEntry, int64_t valueDU)
{
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_NoError;

    if (paramEntry.bits.isSignedDU == true)
    {
        /* signed without self ptr */
        switch (paramEntry.bits.sizeDU)
        {
            case 0:
                IFX_FRW_PARAMTABLE_SET(setFunc_i8, paramEntry, (int8_t)valueDU);
                break;

            case 1:
                IFX_FRW_PARAMTABLE_SET(setFunc_i16, paramEntry, (int16_t)valueDU);
                break;

            case 3:
                IFX_FRW_PARAMTABLE_SET(setFunc_i32, paramEntry, (int32_t)valueDU);
                break;

            default:
                responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
                break;
        }
    }
    else
    {
        /* unsigned without self ptr */
        switch (paramEntry.bits.sizeDU)
        {
            case 0:
                IFX_FRW_PARAMTABLE_SET(setFunc_u8, paramEntry, (uint8_t)valueDU);
                break;

            case 1:
                IFX_FRW_PARAMTABLE_SET(setFunc_u16, paramEntry, (uint16_t)valueDU);
                break;

            case 3:
                IFX_FRW_PARAMTABLE_SET(setFunc_u32, paramEntry, (uint32_t)valueDU);
                break;

            default:
                responseCode = Ifx_FRW_ParamTable_ERR_NotImplemented;
                break;
        }
    }

    return responseCode;
}


/* polyspace-end DEFECT:FUNC_CAST [Justified:Low] "See justification MISRA-C3:11.1" */

/* polyspace-end CODE-METRIC:VOCF [Justified:Low] "Handling of parameter attribute options leads to large but still
 * simple and repetitive structures. Further splitting of this option handling functions would reduce readabilty again
 * and kill the benefit of this metric which is improved readability and maintainability." */
uint8_t Ifx_FRW_ParamTable_getParamSize(uint32_t targetIndex)
{
    uint8_t                       retVal;
    uint16_t                      paramIndex = Ifx_FRW_ParamTable_paramIndex(targetIndex);
    Ifx_FRW_ParamTable_paramEntry paramEntry;
    Ifx_FRW_ParamTable_Errors     responseCode;

    /* Get entry from paramTable by binary search */
    responseCode = Ifx_FRW_ParamTable_searchParamTable(0, IFX_FRW_PARAMTABLE_NUMPARAM - 1, paramIndex, &paramEntry);

    if (responseCode == Ifx_FRW_ParamTable_ERR_NoError)
    {
        retVal = paramEntry.bits.sizeTU + 1u;
    }
    else
    {
        retVal = 1u;
    }

    return retVal;
}
