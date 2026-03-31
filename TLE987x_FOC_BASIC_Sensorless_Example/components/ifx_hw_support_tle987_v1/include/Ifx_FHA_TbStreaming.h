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
 * \file Ifx_FHA_TbStreaming.h
 * \brief Tracebox Streaming Interface component.
 * Default signal for all channels is a sample counter.
 **/

#ifndef IFX_FHA_TBSTREAMING_H
#define IFX_FHA_TBSTREAMING_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "stdint.h"

/******************************************************************************/
/*-----------------------Public Variables/Constants---------------------------*/
/******************************************************************************/

/**
 * \brief Array is part of MotorTestBench automation interface. It is used to
 * change streaming signals at runtime.
 * extern declaration is needed to ensure that a symbol is created in axf file that
 * can be found by the MTB scripts.
 */
extern volatile uint16_t* Ifx_FHA_TbStreaming_g_txPointers[];
extern uint16_t           Ifx_FHA_TbStreaming_TLE987_txBuffer[];

/******************************************************************************/
/*-------------------------Public Unit Specification--------------------------*/
/******************************************************************************/

/**
 *  \brief Returns the component ID
 *
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 *
 */
void Ifx_FHA_TbStreaming_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Returns the component version
 *
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 *
 */
void Ifx_FHA_TbStreaming_getVersion(const Ifx_ComponentVersion** componentVersion);

/**
 * \brief Initialize the TbStreaming interface.
 *
 * The initialization includes enabling of the DMA and should be called once after program start.
 *
 */
void Ifx_FHA_TbStreaming_init(void);

/**
 * \brief Handle data transfer via TbStreaming interface.
 *
 * This function should be called after every control cycle to stream the latest data. It is optimized for execution
 * time and does not perform checks for ongoing or just completed transmissions. This allows for its usage in
 * time-critical scenarios where optimization is essential.
 *
 * \attention The configured number of streaming channels is crucial to ensure that a transmission always completes
 * before the next call to the tick function and that the blockEndGap_us is produced on the SPI bus. The appropriate
 * configuration can be calculated using the following formula:
 *
 * (transmitChannelTime_us + transmitGap_us) * numberOfChannels < tickInterval_us - tickJitter_us - blockEndGap_us
 *
 *  => maxNumberOfChannels =
 *          floor((tickInterval_us - tickJitter_us - blockEndGap_us) / (transmitChannelTime_us + transmitGap_us)
 *
 * with tickInterval_us: Regular period of the tick function call
 *      tickJitter_us: Maximum jitter between consecutive tick function calls
 *      blockEndGap_us: 2.5us (required by TraceBox)
 *      transmitChannelTime_us: 16 bit / baudrate_MHz (3.2us at 5 MBaud)
 *      transmitGap_us: Gap in SPI signals between each 16 bit data frame. On TLE987x there is no measurable gap
 *                      To be safe for uncertainties in DMA bus timing use 0.1us.
 *
 */
void Ifx_FHA_TbStreaming_tick(void);

/**
 * \brief Set which data (16-Bit) to transmit on a specific channel.
 *
 * \param[in] channel Specific channel to transmit data
 * \param[in] target Data to transmit
 */
void Ifx_FHA_TbStreaming_setTxPtr16b(uint8_t channel, uint16_t* target);

/**
 * \brief Set which data (8-Bit) to transmit on a specific channel.
 *
 * \param[in] channel Specific channel to transmit data
 * \param[in] target Data to transmit
 */
void Ifx_FHA_TbStreaming_setTxPtr8b(uint8_t channel, uint8_t* target);

#endif /* IFX_FHA_TBSTREAMING_H */
