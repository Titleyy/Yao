/*
 * Copyright (c) 2024 Infineon Technologies AG. All Rights Reserved.
 *
 * Use of this file is subject to the terms of use agreed between (i) you or the company in which ordinary course of
 * business you are acting and (ii) Infineon Technologies AG, its affiliates or its licensees. If and as long as no
 * such terms of use are agreed, use of this file is subject to the Evaluation Software License Agreement distributed
 * along with this file within the software delivery package.
 *
 */

#include "ComApp.h"
#include "Ifx_FRW_ParamTable.h"
#include "Ifx_FRW_ParamTable_Cfg.h"
#include "Ifx_MS_FocSolution_F16_Cfg.h"
#include <stdint.h>
#include "Ifx_CHA_DTI_TLE987.h"
#include "ident.h"
#include "MctrlExample.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define COM_CFG_TIMEOUT_US 10000000
#define COM_COUNTER_MAX    (COM_CFG_TIMEOUT_US / IFX_MS_FOCSOLUTION_F16_CFG_SPEED_LOOP_PERIOD_US)

/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/

/**
 * \brief Function to reset timeout counter
 */
static void ComApp_resetTimeout(void);

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
static uint16_t   com_g_timeoutCounter       = 0;
static bool       com_g_firstPackageReceived = false;
volatile uint32_t main_fastProcessTimer_50us = 0;

/******************************************************************************/
/*------------------------------Type Definitions------------------------------*/
/******************************************************************************/
typedef union _Updesc_BitConvertBuffer
{
    bool     boolValue;
    uint8_t  uint8Value;
    uint16_t uint16Value;
    int16_t  int16Value;
    uint32_t uint32Value;
    int32_t  int32Value;
    int32_t  fractQ32_15Value;
} pdesc_BitConvertBuffer;

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void ComApp_init(void)
{
    Ifx_CHA_DTI_init();
}


void ComApp_executeTaskSlow(void)
{
    Ifx_CHA_DTI_process();
}


/* get function called by DTI sdoservice for parameter requests */
PARH_Errors PARH_get(uint32_t index, uint16_t num_bytes, void* result)
{
    (void)num_bytes;
    Ifx_FRW_ParamTable_Errors responseCode = Ifx_FRW_ParamTable_ERR_Error;

    switch (index)
    {
        /* catch imageID request */
        case 0x000000u:
            *(uint32_t*)result = id_imageID;
            responseCode       = Ifx_FRW_ParamTable_ERR_NoError;
            break;

        /* catch major version request */
        case 0x000100u:
            *(uint16_t*)result = id_exampleProjectVersion.major;
            responseCode       = Ifx_FRW_ParamTable_ERR_NoError;
            break;

        /* catch minor version request */
        case 0x000200u:
            *(uint16_t*)result = id_exampleProjectVersion.minor;
            responseCode       = Ifx_FRW_ParamTable_ERR_NoError;
            break;

        default:
            responseCode = Ifx_FRW_ParamTable_readParam(index, result);
            break;
    }

    ComApp_resetTimeout();
    com_g_firstPackageReceived = true;

    return (PARH_Errors)responseCode;
}


static void ComApp_resetTimeout(void)
{
    com_g_timeoutCounter = 0;
}


/* set function called by DTI sdoservice for parameter requests */
PARH_Errors PARH_set(uint32_t index, uint16_t num_bytes, void* value)
{
    (void)num_bytes;
    ComApp_resetTimeout();
    com_g_firstPackageReceived = true;

    return (PARH_Errors)Ifx_FRW_ParamTable_writeParam(index, value, MctrlExample_isMotorRunning());
}


/* Function dummy since only expedited transfer is supported */
PARH_Errors PARH_paramAddress(uint32_t index, void** pointer)
{
    (void)index;
    (void)pointer;

    return PARH_Errors_No_Error;
}


uint32_t PARH_paramMaxNumberBytes(uint32_t index)
{
    uint8_t retVal;

    switch (index)
    {
        case 0x000000:
            retVal = 4u;
            break;

        case 0x000100:
            retVal = 2u;
            break;

        case 0x000200:
            retVal = 2u;
            break;

        default:
            retVal = Ifx_FRW_ParamTable_getParamSize(index);
            break;
    }

    return retVal;
}


void ComApp_executeTaskPwm(void)
{
    main_fastProcessTimer_50us++;

    /*For Baudrate 115200Baud. Polling UART for next byte. Safe polling rate 50us should be used to poll two times in
     * one UART frame time of 86.8us. */
    Ifx_CHA_DTI_TLE987_getByteFromUART();
}


void ComApp_executeTaskFast(void)
{
    Ifx_CHA_DTI_TLE987_sendByteToUART();
}


uint32_t getRuntimeCounter()
{
    return main_fastProcessTimer_50us;
}


bool ComApp_executeTaskMid(void)
{
    /* Check if the communication has been lost for 1 second */
    if (com_g_timeoutCounter < COM_COUNTER_MAX)
    {
        if (com_g_firstPackageReceived == true)
        {
            com_g_timeoutCounter++;
        }

        return false;
    }
    else
    {
        return true;
    }
}


bool ComApp_getComTimeout(void)
{
    if (com_g_timeoutCounter >= COM_COUNTER_MAX)
    {
        return true;
    }
    else
    {
        return false;
    }
}
