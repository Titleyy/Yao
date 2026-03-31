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
 * \file Ifx_FRW_ParamTable.h
 * \brief A module which can handle read and write accesses to parameters.
 */

#ifndef IFX_FRW_PARAMTABLE_H
#define IFX_FRW_PARAMTABLE_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include <stdint.h>
#include <stdbool.h>

#include "Ifx_Common.h"
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "Ifx_Math.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define IFX_FRW_PARAMTABLE_MAX_NUMBYTES 4u

#define IFX_FRW_PARAMTABLE_MASK_RDAC    0x0001u
#define IFX_FRW_PARAMTABLE_SBIT_RDAC    0u
#define IFX_FRW_PARAMTABLE_MASK_WRAC    0x0002u
#define IFX_FRW_PARAMTABLE_SBIT_WRAC    1u
#define IFX_FRW_PARAMTABLE_MASK_ISFL    0x0004u
#define IFX_FRW_PARAMTABLE_SBIT_ISFL    2u
#define IFX_FRW_PARAMTABLE_MASK_ISSIGN  0x0008u
#define IFX_FRW_PARAMTABLE_SBIT_ISSIGN  3u
#define IFX_FRW_PARAMTABLE_MASK_ELMSIZE 0x0030u
#define IFX_FRW_PARAMTABLE_SBIT_ELMSIZE 4u
#define IFX_FRW_PARAMTABLE_MASK_PARTYPE 0x00C0u
#define IFX_FRW_PARAMTABLE_SBIT_PARTYPE 6u

/******************************************************************************/
/*------------------------------Type Definitions------------------------------*/
/******************************************************************************/
/* Generic function pointer type */
typedef void (* functionPointer)(void);

/******************************************************************************/
/*-------------------------------Enumerations---------------------------------*/
/******************************************************************************/
/* Error Type for parameter handler. New errors will be added when checks and safety functions are added */
typedef enum Ifx_FRW_ParamTable_Errors
{
    Ifx_FRW_ParamTable_ERR_NoError            = 0, /* No Error in parameter handler*/
    Ifx_FRW_ParamTable_ERR_Error              = 1, /* Error in parameter handler */
    Ifx_FRW_ParamTable_ERR_NotImplemented     = 2, Ifx_FRW_ParamTable_ERR_ReadOnly = 3,
    Ifx_FRW_ParamTable_ERR_ParamNotFoundError = 4, Ifx_FRW_ParamTable_ERR_ParamDoesNotExist = 5,
    Ifx_FRW_ParamTable_ERR_ValueTooHigh       = 6, Ifx_FRW_ParamTable_ERR_ValueTooLow = 7,
    Ifx_FRW_ParamTable_ERR_SubIndexUnkown     = 8, Ifx_FRW_ParamTable_ERR_DenominatorZero = 9,
    Ifx_FRW_ParamTable_ERR_MotorRunning       = 10, Ifx_FRW_ParamTable_ERR_NumeratorsOverflow = 11,
} Ifx_FRW_ParamTable_Errors;
typedef enum Ifx_FRW_ParamTable_ParamType
{
    ParamType_Number = 0, ParamType_Bitfield = 1, ParamType_Enum = 2,
} Ifx_FRW_ParamTable_ParamType;

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Parameter Entry struct for each parameter in the Application table */
typedef struct Ifx_FRW_ParamTable_paramEntry {
    uint16_t index; /* Index of paramEntry */
    union{
        uint16_t properties;    /* Bit 0:     readAccess 
                             * Bit 1:     writeAccess
                             * Bit 2:     isFloat (not used for current supported hw)
                             * Bit 3:     isSignedDU (to be decided wether this is needed)
                             * Bit 4-5    sizeDU in Bytes (0: 1byte, ... , 3: 4Byte) (size 2 not supported)
                             * Bit 6-7:   sizeTU in Bytes (0: 1byte, ... , 3: 4Byte) (size 2 not supported)
                             * Bit 8-9:   parameterType (0: number, 1: bitfield, 2: enum)
                             * Bit 10:    changeWhileMotorRunning
                             * Bit 11:    multiInstance
                             * Bit 12:    isSingedTU
                             * Bit 12-15:  reserved */
        struct
        {
            bool  readAccess :1;
            bool  writeAccess :1;
            bool  isFloat :1;
            bool  isSignedDU :1;
            uint16_t  sizeDU :2;
            uint16_t  sizeTU :2;
            Ifx_FRW_ParamTable_ParamType  parameterType :2;
            bool changeWhileMotorRunning :1;
            bool multiInstance: 1;
            bool isSignedTU: 1;
            uint16_t reserved :3;
        }bits;
    };
    union
    {
        int32_t sMin_DU; /* min digital unit (internal) (Assumption: if parameter is float already, transfer calculation is not needed)*/
        uint32_t uMin_DU;
    };
    union
    {
        int32_t sMax_DU; /* max digital unit (internal)*/
        uint32_t uMax_DU;
    };
    union
    {
        int32_t sMin_TU; /* min transfer unit (external)*/
        uint32_t uMin_TU;
    };
    union
    {
        int32_t sMax_TU; /* max transfer unit (external)*/
        uint32_t uMax_TU;
    };


    void *address;       /* address used for access patterns by (array (8 bit subindex)) address */
    uint8_t maxSubIndex; /* maximum subindex to prevent wrong access */

    functionPointer   getParam; /* pointer to the get function of the parameter */
    functionPointer   setParam; /* pointer to the set function of the parameter */
    void*   instance; /* pointer to self struct of instance */
} Ifx_FRW_ParamTable_paramEntry;
/* *INDENT-ON* */
typedef struct Ifx_FRW_ParamTable_paramRanges
{
    int64_t min_TU; /* min transfer unit (external) */
    int64_t max_TU; /* max transfer unit (external) */
    int64_t min_DU; /* min digital unit (internal) */
    int64_t max_DU; /* max digital unit (internal) */
} Ifx_FRW_ParamTable_paramRanges;
typedef struct Ifx_FRW_ParamTable_fractionParts
{
    int64_t numerator1;
    int64_t numerator2;
    int64_t denominator;
} Ifx_FRW_ParamTable_fractionParts;

/******************************************************************************/
/*-------------------Global Exported Variables/Constants----------------------*/
/******************************************************************************/
extern const Ifx_ComponentID      Ifx_FRW_paramTable_componentID;
extern const Ifx_ComponentVersion Ifx_FRW_paramTable_componentVersion;

/******************************************************************************/
/*-------------------------Inline Function Prototypes-------------------------*/
/******************************************************************************/
/******************************************************************************/
/*-------------------------Global Function Prototypes-------------------------*/
/******************************************************************************/

/**
 *  \brief Accesses a parameter from the parameter table and reads the value to the output buffer
 *
 *  \param [in] targetIdx variable that contains the 24bit index of the targeted parameter
 *  \param [out] *outputBuffer Variable to store the value of the parameter that is read
 *
 */
Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_readParam(uint32_t targetIndex, uint8_t* outputBuffer);

/**
 *  \brief Accesses a parameter from the parameter table and writes the input buffer into the parameter value
 *
 *
 *  \param [in] targetIdx variable that contains the 24bit index of the targeted parameter
 *  \param [out] *inputBuffer Variable that contains the value that is written to the parameter value
 *  \param [in] motorRunning flag that indicates if the motor is running
 *
 */
Ifx_FRW_ParamTable_Errors Ifx_FRW_ParamTable_writeParam(uint32_t targetIndex, uint8_t* inputBuffer, bool
                                                        motorRunning);

/**
 *  \brief Returns the size of the target parameter in byte
 *
 *
 *  \param [in] targetIdx variable that contains the 24bit index of the targeted parameter
 *
 */
uint8_t Ifx_FRW_ParamTable_getParamSize(uint32_t targetIndex);

/**
 *  \brief Returns the component ID
 *
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 *
 */
void Ifx_FRW_paramTable_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Returns the component version
 *
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 *
 */
void Ifx_FRW_paramTable_getVersion(const Ifx_ComponentVersion** componentVersion);

/******************************************************************************/
/*---------------------Inline Function Implementations------------------------*/
/******************************************************************************/
#endif /* IFX_FRW_PARAMTABLE_H */
