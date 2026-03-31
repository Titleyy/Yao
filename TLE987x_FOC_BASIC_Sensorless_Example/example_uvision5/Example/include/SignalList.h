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
 * \file SignalList.h
 * \brief Init signal list and add all variables/signals that are transmitted using tracebox streaming.
 */

#ifndef SIGNALLIST_H
#define SIGNALLIST_H

/******************************************************************************/
/*----------------------------------Includes----------------------------------*/
/******************************************************************************/


/******************************************************************************/
/*-------------------------Global Function Prototypes-------------------------*/
/******************************************************************************/

/**
 * \brief Initialize SignalList for traceboxstreaming.
 */
extern void SignalList_init(void);

#endif /* SIGNALLIST_H */
