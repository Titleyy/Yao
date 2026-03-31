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
#include "Ifx_CHA_DTI.h"
#include "Ifx_CHA_DTI_TLE987.h"

/* C standard library */
#include <stdint.h>
#include <stddef.h>

/* DTI library */
#include "dti.h"
#include "messageHandler_api.h"
#include "signalHandler_api.h"
#include "traceHandler_api.h"

/* Timestamp driver */
#include "TSdriver.h "

/* SDK */
#include "tle987x.h "
#include "uart.h "

/* polyspace-begin Custom:2.1 [Justified:Low] "Macro name set by external DTI stack" */
/** Fast process timer has time unit of 50us -> target frequency is 20000 Hz */
#define TARGETINTERFACE_TSdriver_TIMESCALE_Hz 20000

/* polyspace-end Custom:2.1 [Justified:Low] "Macro name set by external DTI stack" */
/* *INDENT-OFF* */
/* Macros to define the component ID */
#define IFX_CHA_DTI_TLE987_COMPONENTID_SOURCEID     ((uint8_t)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_CHA_DTI_TLE987_COMPONENTID_LIBRARYID    ((uint16_t)Ifx_ComponentID_LibraryID_comHardwareAbstraction)
#define IFX_CHA_DTI_TLE987_COMPONENTID_MODULEID     (0U)
#define IFX_CHA_DTI_TLE987_COMPONENTID_COMPONENTID1 (0U)
#define IFX_CHA_DTI_TLE987_COMPONENTID_COMPONENTID2 ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_CHA_DTI_TLE987_COMPONENTVERSION_MAJOR   (2U)
#define IFX_CHA_DTI_TLE987_COMPONENTVERSION_MINOR   (0U)
#define IFX_CHA_DTI_TLE987_COMPONENTVERSION_PATCH   (0U)
#define IFX_CHA_DTI_TLE987_COMPONENTVERSION_T       (4U)
#define IFX_CHA_DTI_TLE987_COMPONENTVERSION_REV     (0U)

/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/

/**
 * \brief Private function. Get data from UART buffer
 *
 *
 */
static void Ifx_CHA_DTI_TLE987_bufferRead(Buffer* buffer, uint8* data);

/**
 * \brief Private function. Write data to UART buffer
 *
 *
 */
static void Ifx_CHA_DTI_TLE987_bufferWrite(Buffer* buffer, uint8 data);

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/* Component ID */
static const Ifx_ComponentID      Ifx_CHA_DTI_TLE987_componentID = {
    .sourceID     = IFX_CHA_DTI_TLE987_COMPONENTID_SOURCEID,
    .libraryID    = IFX_CHA_DTI_TLE987_COMPONENTID_LIBRARYID,
    .moduleID     = IFX_CHA_DTI_TLE987_COMPONENTID_MODULEID,
    .componentID1 = IFX_CHA_DTI_TLE987_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_CHA_DTI_TLE987_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_CHA_DTI_TLE987_componentVersion = {
    .major = IFX_CHA_DTI_TLE987_COMPONENTVERSION_MAJOR,
    .minor = IFX_CHA_DTI_TLE987_COMPONENTVERSION_MINOR,
    .patch = IFX_CHA_DTI_TLE987_COMPONENTVERSION_PATCH,
    .t     = IFX_CHA_DTI_TLE987_COMPONENTVERSION_T,
    .rev   = IFX_CHA_DTI_TLE987_COMPONENTVERSION_REV
};

/** Buffers for UART handling */
Buffer receiveBuffer, sendBuffer;

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
/* polyspace-begin Custom:7.1 [Justified:Low] "Function names "DTI..." are fixed by external library (DTI)"
 * */
void DTI_phylayer_init(void)
{
    /* Initialize the UART module */
    UART2_Init();

    /* Set transfer interrupt bit */
    UART2->SCON.bit.TI = 1U;

    /* Initialize send and receive buffers */
    sendBuffer.startIndex    = 0U;
    sendBuffer.endIndex      = 0U;
    receiveBuffer.startIndex = 0U;
    receiveBuffer.endIndex   = 0U;
} /* end of void TARGETINTERFACE_phylayer_init()*/


uint32_t DTI_phylayer_bytesToRead(void)
{
    uint32_t retVal;

    if (receiveBuffer.startIndex == receiveBuffer.endIndex)
    {
        retVal = 0;
    }
    else
    {
        retVal = 1;
    }

    return retVal;
}


uint32_t DTI_phylayer_read(void)
{
    uint8_t data;
    Ifx_CHA_DTI_TLE987_bufferRead(&receiveBuffer, &data);

    return (uint32_t)data;
}


void DTI_phylayer_write(uint32_t data)
{
    Ifx_CHA_DTI_TLE987_bufferWrite(&sendBuffer, (uint8_t)data);
}

uint32_t DTI_TSdriver_getTimeStamp(void)
{
    return 0;
}


uint32_t DTI_TSdriver_getTimeStampScale(void)
{
    return 1;
}


uint32_t DTI_TSdriver_getTargetTime(void)
{
    return getRuntimeCounter();
}


uint32_t DTI_TSdriver_getTargetTimeScale(void)
{
    return TARGETINTERFACE_TSdriver_TIMESCALE_Hz;
}

/* polyspace-end Custom:7.1 [Justified:Low] "Function names "DTI..." are fixed by external library (DTI)" */


void Ifx_CHA_DTI_TLE987_getByteFromUART(void)
{
    if (UART2_isByteReceived() == true)
    {
        Ifx_CHA_DTI_TLE987_bufferWrite(&receiveBuffer, UART2_Get_Byte());
    }
}


void Ifx_CHA_DTI_TLE987_sendByteToUART(void)
{
    uint8 data;

    if ((UART2_isByteTransmitted() == true)
        && (sendBuffer.startIndex != sendBuffer.endIndex))
    {
        Ifx_CHA_DTI_TLE987_bufferRead(&sendBuffer, &data);
        UART2_Send_Byte(data);
    }
}
/* *INDENT-ON* */
void Ifx_CHA_DTI_init(void)
{
    /* inits arbiter, router, linklayer, physical layer*/
    DTI_interface_init();
    MSGH_Polling_Init();
    TRCH_Init();
}


void Ifx_CHA_DTI_process(void)
{
    DTI_interface_DD();                                             /* receive and process UART frames */
    (void)TRCH_ProcessTrace();
    int32_t receivedBytes = (int32_t)MSGH_Polling_ProcessMessage(); /* process UART frames and call
                                                                     * ParameterHandler */
    DTI_interface_DU();                                             /* send UART frames */
}


/* Function to get the component ID */
void Ifx_CHA_DTI_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_CHA_DTI_TLE987_componentID;
}


/* Function to get the component version */
void Ifx_CHA_DTI_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_CHA_DTI_TLE987_componentVersion;
}


static void Ifx_CHA_DTI_TLE987_bufferWrite(Buffer* buffer, uint8 data)
{
    /* Store data in the buffer */
    buffer->data[buffer->endIndex] = data;

    /* Reset the end index if reached the end of the buffer */
    if (buffer->endIndex == (BUFFER_SIZE - 1))
    {
        buffer->endIndex = 0;
    }
    else
    {
        buffer->endIndex++;
    }
}


static void Ifx_CHA_DTI_TLE987_bufferRead(Buffer* buffer, uint8* data)
{
    /* Get the oldest value from the data buffer */
    *data = buffer->data[buffer->startIndex];

    /* Reset the start index if reached the end of the buffer */
    if (buffer->startIndex == (BUFFER_SIZE - 1))
    {
        buffer->startIndex = 0;
    }
    else
    {
        buffer->startIndex++;
    }
}


/* polyspace-begin Custom:7.1 [Justified:Low] "Function names "TRCH..." are fixed by external library (DTI)" */

/** Dummy Implementation of sampling service API
 */
const void *TRCH_getSignalptr(uint32_t idx_target)
{
    return NULL;
}


/** Dummy Implementation of sampling service API
 */
TRCH_SignalType_t TRCH_getSignalType(uint32_t trch_signal)
{
    return TRCH_SignalType_16bit;
}


/* polyspace-end Custom:7.1 [Justified:Low] "Function names "TRCH..." are fixed by external library (DTI)" */
