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
 * \file Ifx_FHA_TbStreaming_TLE987_Cfg.h
 * \brief Tracebox Streaming Interface component.
 **/

#ifndef IFX_FHA_TBSTREAMING_TLE987_CFG_H
#define IFX_FHA_TBSTREAMING_TLE987_CFG_H

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/

/*
 * The maximum number of streaming channels which can be used for the default configuration is calculated as follows:
 *
 * maxNumberOfChannels =
 *      floor((tickInterval_us - tickJitter_us - blockEndGap_us) / (transmitChannelTime_us + transmitGap_us)
 *
 * with: tickIntervall_us = 150us (at 20kHz PWM frequency and current loop factor 3)
 *       tickJitter_us = 1us (measured value when motor is running)
 *       blockEndGap_us = 2.5us (required by TraceBox)
 *       transmitChannelTime_us = 16 bit / (5 * 10^6 bit/s) = 3.2us (at 5 MBaud)
 *       transmitGap_us = 0.1us (safety gap for uncertainties in SPI DMA bus timing.)
 *
 *      => maxNumberOfChannels = floor((150 - 1 - 2.5) / (16 / 5 + 0.1)) = 44
 *
 * Note: More channels than 24 could be transmitted in the time frame.
 *       Still total maximum supported by this component and TraceBox is 24.
 */

/* Number of 16-Bit TX channels, Range = [1; 16] */
#define IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_16_BIT 12U

/* Number of 8-Bit TX channels, Range = [0; 8] */
#define IFX_FHA_TBSTREAMING_TLE987_CFG_TX_CHANNELS_8_BIT  0

/* SSC to use (1 or 2) - SSC settings must be configured in Config Wizard */
#define IFX_FHA_TBSTREAMING_TLE987_SSC                    1

#endif /* IFX_FHA_TBSTREAMING_TLE987_CFG_H */
