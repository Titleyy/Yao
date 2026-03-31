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
 * \file Ifx_Math_ConvToQForm.h
 * \brief Provides functions to calculate the Q format and the value in that Q format of an input
 */

#ifndef IFX_MATH_CONVTOQFORM_H
#define IFX_MATH_CONVTOQFORM_H
#include "Ifx_Math.h"
#include "arm_math.h"

/**
 * Offset to use __CLZ function in 16-bit format
 */
#define IFX_MATH_CONVTOQFORM_OFFSET_SIGNED_16BIT (17)

/**
 *  \brief Calculates the value and Q format of a 32-bit unsigned number
 *
 *  This function calculates the Q format and the value in the calculated Q format
 *  in 16-bit representation of the ratio of the input and the factor. The factor is
 *  a power of 10 and represents the decimal places (1, 10, 100...). The function
 *  saturates the output to the maximum representable value of signed 16-bit format.
 *
 *  \param [in] input 32-bit unsigned number
 *
 *  \param [in] factor Power of 10 representing the decimal places (1, 10, 100...), should not be 0
 *
 *  \return Q format and value in calculated Q format of the ratio of the input and factor
 */
static inline Ifx_Math_Fract16Q Ifx_Math_ConvToQForm_F16(uint32_t input, uint32_t factor)
{
    Ifx_Math_Fract16Q output;

    /* Precalculate the ratio to check for the maximum */
    uint32_t          ratio = input / factor;

    if (ratio > IFX_MATH_FRACT16_MAX)
    {
        output.qFormat = Ifx_Math_FractQFormat_q0;
        output.value   = IFX_MATH_FRACT16_MAX;
    }
    else
    {
        output.qFormat = (Ifx_Math_FractQFormat)(__CLZ((ratio)) - IFX_MATH_CONVTOQFORM_OFFSET_SIGNED_16BIT);
        output.value   = (Ifx_Math_Fract16)((uint32_t)(input * (1 << output.qFormat)) / factor);
    }

    return output;
}


#endif /*IFX_MATH_CONVTOQFORM_H*/
