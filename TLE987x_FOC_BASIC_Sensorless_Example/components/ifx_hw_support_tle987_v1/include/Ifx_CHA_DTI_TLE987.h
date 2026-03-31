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
 * \file Ifx_CHA_DTI_TLE987.h
 * \brief Hardware driver for DTI on TLE987
 */

#ifndef Ifx_CHA_DTI_TLE987_H
#define Ifx_CHA_DTI_TLE987_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/
#include "Ifx_CHA_DTI.h"

/* SDK */
#include "uart.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
#define BUFFER_SIZE 64

/******************************************************************************/
/*-----------------------------Data Structures--------------------------------*/
/******************************************************************************/
typedef struct
{
    uint16 startIndex;
    uint16 endIndex;
    uint8  data[BUFFER_SIZE];
} Buffer;

/******************************************************************************/
/*-----------------------Public Variables/Constants---------------------------*/
/******************************************************************************/
extern Buffer receiveBuffer, sendBuffer;

/******************************************************************************/
/*-------------------------Public Unit Specification--------------------------*/
/******************************************************************************/
void Ifx_CHA_DTI_TLE987_getByteFromUART(void);
void Ifx_CHA_DTI_TLE987_sendByteToUART(void);

/**
 *  \brief Initialize the physical layer.
 *
 */
void DTI_phylayer_init(void);

/**
 *  \brief Get the number of bytes left to read from the physical layer.
 *
 *  \return Number of bytes left to read
 */
uint32_t DTI_phylayer_bytesToRead(void);

/**
 *  \brief Read a byte from the phisical layer.
 *
 *  \return Byte data
 */
uint32_t DTI_phylayer_read(void);

/**
 *  \brief Write a byte into the physical layer.
 *
 *  \param[in] data Byte to write
 */
void DTI_phylayer_write(uint32_t data);

#endif /* Ifx_CHA_DTI_TLE987_H */
