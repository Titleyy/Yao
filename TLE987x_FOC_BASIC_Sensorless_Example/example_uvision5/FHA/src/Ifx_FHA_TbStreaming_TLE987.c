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
#include "Ifx_FHA_TbStreaming.h"
#include "Ifx_FHA_TbStreaming_TLE987_Cfg.h"

/* SDK */
#include "tle_device.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/* Maximum number of 16-Bit TX channels */
#define IFX_FHA_TBSTREAMING_TLE987_NUMBER_TX_CHANNELS_16_BIT_MAX 16

/* Minimum number of 16-Bit TX channels */
#define IFX_FHA_TBSTREAMING_TLE987_NUMBER_TX_CHANNELS_16_BIT_MIN 1

/* Maximum number of 8-Bit TX channels */
#define IFX_FHA_TBSTREAMING_TLE987_NUMBER_TX_CHANNELS_8_BIT_MAX  8

/* Minimum number of 8-Bit TX channels */
#define IFX_FHA_TBSTREAMING_TLE987_NUMBER_TX_CHANNELS_8_BIT_MIN  0

#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT < IFX_FHA_TBSTREAMING_TLE987_NUMBER_TX_CHANNELS_16_BIT_MIN)
#error Number of configured 16-Bit TbStreaming TX channels is lower than the minimum!
#elif (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT > IFX_FHA_TBSTREAMING_TLE987_NUMBER_TX_CHANNELS_16_BIT_MAX)
#error Number of configured 16-Bit TbStreaming TX channels is higher than the maximum!
#elif (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT < IFX_FHA_TBSTREAMING_TLE987_NUMBER_TX_CHANNELS_8_BIT_MIN)
#error Number of configured 8-Bit TbStreaming TX channels is lower than the minimum!
#elif (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT > IFX_FHA_TBSTREAMING_TLE987_NUMBER_TX_CHANNELS_8_BIT_MAX)
#error Number of configured 8-Bit TbStreaming TX channels is higher than the maximum!
#endif

/* Number of words in the TbStreaming TX buffer */
#define IFX_FHA_TBSTREAMING_TX_NUM_WORDS                \
    IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT + \
    IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT

/* *INDENT-OFF* */

/* Macros to define the component ID */
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_SOURCEID     ((uint8_t)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_LIBRARYID   \
 ((uint16_t)Ifx_ComponentID_LibraryID_mctrlHardwareAbstraction)
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_MODULEID     (0U)
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_COMPONENTID1 (1U)
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_COMPONENTID2 ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_MAJOR   (2U)
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_MINOR   (0U)
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_PATCH   (0U)
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_T       (4U)
#define IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_REV     (0U)

/* Component ID */
static const Ifx_ComponentID      Ifx_FHA_TbStreaming_TLE987_componentID = {
    .sourceID     = IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_SOURCEID,
    .libraryID    = IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_LIBRARYID,
    .moduleID     = IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_MODULEID,
    .componentID1 = IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_FHA_TBSTREAMING_TLE987_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_FHA_TbStreaming_TLE987_componentVersion = {
    .major = IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_MAJOR,
    .minor = IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_MINOR,
    .patch = IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_PATCH,
    .t     = IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_T,
    .rev   = IFX_FHA_TBSTREAMING_TLE987_COMPONENTVERSION_REV
};

#if (IFX_FHA_TBSTREAMING_TLE987_SSC == 1)
#define IFX_FHA_TBSTREAMING_SSC_PERIPHERAL                             SSC1
#elif (IFX_FHA_TBSTREAMING_TLE987_SSC == 2)
#define IFX_FHA_TBSTREAMING_SSC_PERIPHERAL                             SSC2
#else
#error Invalid SSC peripheral selected
#endif

/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/
/**
 * \brief Initialize TX data pointers with defined initial pointer.
 *
 *        This prevents from NULL pointer exceptions if TX data pointers are accessed.
 *        Make sure TX pointers point somewhere valid initially.
 */
static void Ifx_FHA_TbStreaming_TLE987_initDataPointers(void);

/**
 * \brief Initialize the DMA channels for data transmission and reception via tbstreaming interface.
 *
 *        DMA_Setup_Channel is not used due to bugs present
 *        (end ptr, Src_Inc/Dst_iInc not set correctly when increase not used).
 */
static void Ifx_FHA_TbStreaming_TLE987_initDMATransfer(void);

/**
 * \brief Initialize the SSC so that it will trigger DMA transfers via TIR event.
 */
static void Ifx_FHA_TbStreaming_TLE987_initSSCForDMATrigger(void);

/**
 * \brief Prepare TX data for transmission via tracebox streaming interface.
 *
 */
static void Ifx_FHA_TbStreaming_TLE987_prepareTxData(void);

/**
 * \brief Prepare the 16-Bit TX data for transmission.
 *
 */
static void Ifx_FHA_TbStreaming_TLE987_prepareTxData16b(void);

/**
 * \brief Prepare the 8-Bit TX data for transmission.
 *
 */
static void Ifx_FHA_TbStreaming_TLE987_prepareTxData8b(void);

/**
 * \brief Reset DMA channels.
 *
 *        Do not use DMA_Reset_Channel because it is not inlined and cannot handle two
 *        channels at once. Use predefined constant control register values instead of
 *        bit manipulation for performance.
 */
static void Ifx_FHA_TbStreaming_TLE987_resetDMAChannels(void);

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/* TX data from DUT buffer
 * The buffer is external global, so it can be used as CH2 source in SDKs Config Wizard DMA page.
 * TbStreaming will work without configuring DMA CH2 in Config wizard, but then it is not transparent in Config Wizard 
 * that the resource is used. */
uint16_t  Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TX_NUM_WORDS];

/* Having a separate uint8 array is a work around. It is not needed when MTB takes care of word alignment. */
static uint8_t  *Ifx_FHA_TbStreaming_g_txPointers_8Bit[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT];

/* Dummy pointer target used to make sure that TX pointers point somewhere valid initially */
static uint16_t  Ifx_FHA_TbStreaming_TLE987_g_sampleCounter = 0U;

/* Pointer to DMA CH2 */
static TDMA_Entry* Ifx_FHA_TbStreaming_TLE987_g_DMAEntryCh2Pri;

/* *INDENT-OFF* */
/* Constant of DMA control register for CH2 */
static const TControl Ifx_FHA_TbStreaming_TLE987_g_dmaCtrlCh2 = {
    .bit = {
        .Cycle_Ctrl    = (uint32_t)DMA_Cycle_Type_Basic,
        .Next_UseBurst = 0U,
        .N_Minus_1     = IFX_FHA_TBSTREAMING_TX_NUM_WORDS - 1U,
        .R_Power       = 0U,
        .Src_Prot_Ctrl = 7U,
        .Dst_Prot_Ctrl = 7U,
        .Src_Size      = (uint32_t)DMA_16Bit_Transfer,
        .Src_Inc       = 0b01U,    /* Increase 16 bit */
        .Dst_Size      = (uint32_t)DMA_16Bit_Transfer,
        .Dst_Inc       = 0b11U     /* Do not increase */
    }
};
/* *INDENT-ON* */
/******************************************************************************/
/*-----------------------Exported Variables/Constants-------------------------*/
/******************************************************************************/
__USED volatile uint16_t* Ifx_FHA_TbStreaming_g_txPointers[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT];

/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void Ifx_FHA_TbStreaming_init(void)
{
    Ifx_FHA_TbStreaming_TLE987_initDataPointers();
    Ifx_FHA_TbStreaming_TLE987_initDMATransfer();
    Ifx_FHA_TbStreaming_TLE987_initSSCForDMATrigger();
}


static void Ifx_FHA_TbStreaming_TLE987_initSSCForDMATrigger(void)
{
    /* Trigger one SSC transmission manually to generate the initial TIR event, which will trigger the DMA to transfer
     * the TX data to the SSC transmission buffer once the DMA channel has been enabled. All further TIR events will
     * then be generated automatically by the SSC. */
    Field_Wrt16(&IFX_FHA_TBSTREAMING_SSC_PERIPHERAL->TB.reg, (uint16_t)SSC1_TB_TB_VALUE_Pos,
        (uint16_t)SSC1_TB_TB_VALUE_Msk, *(Ifx_FHA_TbStreaming_g_txPointers[0]));
}


static void Ifx_FHA_TbStreaming_TLE987_initDataPointers(void)
{
    uint32_t i;

    /* Make sure TX pointers point somewhere valid initially */
    for (i = 0U; i < IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT; i++)
    {
        Ifx_FHA_TbStreaming_g_txPointers[i] = &Ifx_FHA_TbStreaming_TLE987_g_sampleCounter;
    }

    for (i = 0U; i < IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT; i++)
    {
        Ifx_FHA_TbStreaming_g_txPointers_8Bit[i] = (uint8_t*)&Ifx_FHA_TbStreaming_TLE987_g_sampleCounter;
    }
}


static void Ifx_FHA_TbStreaming_TLE987_initDMATransfer(void)
{
    Ifx_FHA_TbStreaming_TLE987_g_DMAEntryCh2Pri = (TDMA_Entry*)DMA_CHx_Entry_Pri(DMA_CH2);

    /* Set up DMA trigger */
    SCU->DMASRCSEL.bit.SSCTXSRCSEL = IFX_FHA_TBSTREAMING_TLE987_SSC - 1;

    /* Set up DMA channels. The SDK API DMA_Setup_Channel is not used because the end ptr, Src_Inc and Dst_Inc are not
     * set correctly when increase not used. */
    Ifx_FHA_TbStreaming_TLE987_g_DMAEntryCh2Pri->Src_End_Ptr =
        (uint32_t)&Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TX_NUM_WORDS - 1];
    Ifx_FHA_TbStreaming_TLE987_g_DMAEntryCh2Pri->Dst_End_Ptr = (uint32_t)&IFX_FHA_TBSTREAMING_SSC_PERIPHERAL->TB.reg;
    Ifx_FHA_TbStreaming_TLE987_g_DMAEntryCh2Pri->Control.reg = Ifx_FHA_TbStreaming_TLE987_g_dmaCtrlCh2.reg;
    DMA_Master_En();
}


__USED void Ifx_FHA_TbStreaming_tick(void)
{
    Ifx_FHA_TbStreaming_TLE987_prepareTxData();
    Ifx_FHA_TbStreaming_TLE987_g_sampleCounter++;

    /* Trigger DMA transfer */
    Ifx_FHA_TbStreaming_TLE987_resetDMAChannels();
}


static void Ifx_FHA_TbStreaming_TLE987_prepareTxData(void)
{
    Ifx_FHA_TbStreaming_TLE987_prepareTxData16b();
    Ifx_FHA_TbStreaming_TLE987_prepareTxData8b();
}


/* polyspace-begin CODE-METRICS:VOCF [Justified:Low] "Runtime optimized unrolled loop. Length is adapted with
 * user-configured preprocessor define. The function is still simple and thus easily maintainable." */
static void Ifx_FHA_TbStreaming_TLE987_prepareTxData16b(void)
{
    /* The following assignments are unrolled due to performance reasons. */
    Ifx_FHA_TbStreaming_TLE987_txBuffer[0] = *Ifx_FHA_TbStreaming_g_txPointers[0];
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 2U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[1] = *Ifx_FHA_TbStreaming_g_txPointers[1];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 3U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[2] = *Ifx_FHA_TbStreaming_g_txPointers[2];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 4U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[3] = *Ifx_FHA_TbStreaming_g_txPointers[3];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 5U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[4] = *Ifx_FHA_TbStreaming_g_txPointers[4];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 6U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[5] = *Ifx_FHA_TbStreaming_g_txPointers[5];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 7U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[6] = *Ifx_FHA_TbStreaming_g_txPointers[6];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 8U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[7] = *Ifx_FHA_TbStreaming_g_txPointers[7];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 9U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[8] = *Ifx_FHA_TbStreaming_g_txPointers[8];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 10U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[9] = *Ifx_FHA_TbStreaming_g_txPointers[9];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 11U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[10] = *Ifx_FHA_TbStreaming_g_txPointers[10];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 12U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[11] = *Ifx_FHA_TbStreaming_g_txPointers[11];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 13U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[12] = *Ifx_FHA_TbStreaming_g_txPointers[12];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 14U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[13] = *Ifx_FHA_TbStreaming_g_txPointers[13];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 15U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[14] = *Ifx_FHA_TbStreaming_g_txPointers[14];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT >= 16U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[15] = *Ifx_FHA_TbStreaming_g_txPointers[15];
#endif
}


static void Ifx_FHA_TbStreaming_TLE987_prepareTxData8b(void)
{
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT >= 1U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT + 0U] =
        (uint16_t)*Ifx_FHA_TbStreaming_g_txPointers_8Bit[0];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT >= 2U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT + 1U] =
        (uint16_t)*Ifx_FHA_TbStreaming_g_txPointers_8Bit[1];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT >= 3U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT + 2U] =
        (uint16_t)*Ifx_FHA_TbStreaming_g_txPointers_8Bit[2];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT >= 4U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT + 3U] =
        (uint16_t)*Ifx_FHA_TbStreaming_g_txPointers_8Bit[3];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT >= 5U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT + 4U] =
        (uint16_t)*Ifx_FHA_TbStreaming_g_txPointers_8Bit[4];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT >= 6U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT + 5U] =
        (uint16_t)*Ifx_FHA_TbStreaming_g_txPointers_8Bit[5];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT >= 7U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT + 6U] =
        (uint16_t)*Ifx_FHA_TbStreaming_g_txPointers_8Bit[6];
#endif
#if (IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT >= 8U)
    Ifx_FHA_TbStreaming_TLE987_txBuffer[IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT + 7U] =
        (uint16_t)*Ifx_FHA_TbStreaming_g_txPointers_8Bit[7];
#endif
}


/* polyspace-end CODE-METRICS:VOCF [Justified:Low] "Runtime optimized unrolled loop. Length is adapted with
 * user-configured preprocessor define. The functions are still simple and thus easily maintainable." */
static void Ifx_FHA_TbStreaming_TLE987_resetDMAChannels(void)
{
    /* Reset DMA control register for TX data transfer. */
    Ifx_FHA_TbStreaming_TLE987_g_DMAEntryCh2Pri->Control.reg = Ifx_FHA_TbStreaming_TLE987_g_dmaCtrlCh2.reg;

    /* Enable DMA channel for TX data transfer. */
    DMA_Channel_Enable_Set(DMA_MASK_CH2);
}


void Ifx_FHA_TbStreaming_setTxPtr16b(uint8_t channel, uint16_t* target)
{
    if (channel < IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT)
    {
        Ifx_FHA_TbStreaming_g_txPointers[channel] = target;
    }
}


void Ifx_FHA_TbStreaming_setTxPtr8b(uint8_t channel, uint8_t* target)
{
    uint8_t arrayIndex;
    uint8_t offset;
    offset = IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT;

    if (channel >= offset)
    {
        arrayIndex = channel - offset;

        if (arrayIndex < IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT)
        {
            Ifx_FHA_TbStreaming_g_txPointers_8Bit[arrayIndex] = target;
        }
        else /* channel number to high. Do nothing.*/
        {}
    }
    else     /* channel number to low. Do nothing.*/
    {}
}


void Ifx_FHA_TbStreaming_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_FHA_TbStreaming_TLE987_componentID;
}


void Ifx_FHA_TbStreaming_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_FHA_TbStreaming_TLE987_componentVersion;
}
