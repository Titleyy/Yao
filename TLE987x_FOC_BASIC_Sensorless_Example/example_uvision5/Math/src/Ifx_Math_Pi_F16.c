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
#include "Ifx_Math_Pi_F16.h"
#include "Ifx_Math_MulShR.h"
#include "Ifx_Math_Sub.h"
#include "Ifx_Math_Add.h"
#include "Ifx_Math_Sat.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/* Additional shift factor for the PI internal Q format which is a trade-off between range and resolution */
#define IFX_MATH_PI_F16_INT_Q_SHIFT           Ifx_Math_FractQFormat_q9

/* If the trapezoidal method is chosen, divide sampling time by 2 */
#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_TRAPEZOIDAL
#define IFX_MATH_PI_F16_SAMPLING_TIME_DIVIDER (1)
#else
#define IFX_MATH_PI_F16_SAMPLING_TIME_DIVIDER (0)
#endif

/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_TRAPEZOIDAL
#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC

/**
 *  \brief PI controller implementation using trapezoidal discretization and anti-windup back calculation
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Input error
 *
 *  \return PI controller output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_trapezoidal_backCalculation(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16
                                                                             errorValue);

#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP

/**
 *  \brief PI controller implementation using trapezoidal discretization and anti-windup clamp
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Input error
 *  \param [in] antiWindupCtrl Antiwindup control input
 *
 *  \return PI controller output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_trapezoidal_clamp(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue,
                                                                   Ifx_Math_Pi_F16_AntiWindupCtrl antiWindupCtrl);

#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_NONE

/**
 *  \brief PI controller implementation using trapezoidal discretization and no anti-windup
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Input error
 *
 *  \return PI controller output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_trapezoidal(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue);

#endif

#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_FORWARD
#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC

/**
 *  \brief PI controller implementation using Forward Euler discretization and anti-windup back calculation
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Input error
 *
 *  \return PI controller output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_forwardEuler_backCalculation(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16
                                                                              errorValue);

#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP

/**
 *  \brief PI controller implementation using Forward Euler discretization and anti-windup clamp
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Input error
 *  \param [in] antiWindupCtrl Antiwindup control input
 *
 *  \return PI controller output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_forwardEuler_clamp(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16
                                                                    errorValue, Ifx_Math_Pi_F16_AntiWindupCtrl
                                                                    antiWindupCtrl);

#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_NONE

/**
 *  \brief PI controller implementation using Forward Euler discretization and no anti-windup
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Input error
 *
 *  \return PI controller output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_forwardEuler(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue);

#endif

#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_BACKWARD
#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC

/**
 *  \brief PI controller implementation using Backward Euler discretization and anti-windup back calculation
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Input error
 *
 *  \return PI controller output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_backEuler_backCalculation(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16
                                                                           errorValue);

#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP

/**
 *  \brief PI controller implementation using Backward Euler discretization and anti-windup clamp
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Input error
 *  \param [in] antiWindupCtrl Antiwindup control input
 *
 *  \return PI controller output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_backEuler_clamp(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue,
                                                                 Ifx_Math_Pi_F16_AntiWindupCtrl antiWindupCtrl);

#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_NONE

/**
 *  \brief PI controller implementation using Backward Euler discretization and no anti-windup
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Input error
 *
 *  \return PI controller output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_backEuler(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue);

#endif

#endif

/**
 *  \brief Saturate the integral gain previous value between the defined limits
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 */
static inline void Ifx_Math_Pi_F16_p_saturatePreviousValue(Ifx_Math_Pi_F16* self);

/**
 *  \brief Saturate the output between the defined limits
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] out Output of the PI to be saturated
 *
 *  \return Saturated output
 */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_saturateOutput(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 out);

/**
 *  \brief Calculate the product between the error and the gain and return the intermediate Q format
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Error input to the PI controller
 *  \param [inout] propGainError Proportional gain multiplied by the input error
 *  \param [inout] integGainError Integral gain multiplied by the sampliing time and input error
 *
 */
static inline void Ifx_Math_Pi_F16_p_calculateGainsError(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue,
                                                         Ifx_Math_Fract32* propGainError,
                                                         Ifx_Math_Fract32* integGainError);

/**
 *  \brief Check whether integration should be enabled or disabled based on internal and external antiWindup flags
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Error input to the PI controller
 *  \param [in] antiWindupCtrl Antiwindup control input
 *
 * \return Signed integer with value 1 if integration is enabled and 0 if integration is disabled
 */
static inline int8_t Ifx_Math_Pi_F16_p_antiWindupControl(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue,
                                                         Ifx_Math_Pi_F16_AntiWindupCtrl antiWindupCtrl);

/**
 *  \brief Check whether integration should be enabled or disabled based on an internal antiWindup flag
 *
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Error input to the PI controller
 *
 * \return Signed integer with value 1 if integration is enabled and 0 if integration is disabled
 */
static inline int8_t Ifx_Math_Pi_F16_p_antiWindupCtrlIntern(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue);

/**
 *  \brief Check whether integration should be enabled or disabled based on an external antiWindup flag
 *
 *
 *  \param [in] errorValue Error input to the PI controller
 *  \param [in] antiWindupCtrl Antiwindup control input
 *
 * \return Signed integer with value 1 if integration is enabled and 0 if integration is disabled
 */
static inline int8_t Ifx_Math_Pi_F16_p_antiWindupCtrlExtern(Ifx_Math_Fract16               errorValue,
                                                            Ifx_Math_Pi_F16_AntiWindupCtrl antiWindupCtrl);
#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC

/**
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *  \param [in] errorValue Calculate the antiwindup part using the back calculation method
 *
 *  \return
 */
static inline Ifx_Math_Fract32 Ifx_Math_Pi_F16_p_backCalculation(Ifx_Math_Pi_F16* self, Ifx_Math_Fract32 errorValue);
#endif /* IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC */
/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
void Ifx_Math_Pi_F16_init(Ifx_Math_Pi_F16* self, Ifx_Math_Pi_F16_Qformats qformatsPi)
{
    /* Initialize limits to maximum */
    self->p_upperLimit = IFX_MATH_FRACT16_MAX;
    self->p_lowerLimit = IFX_MATH_FRACT16_MIN;

    /* Initialize integral previous value to 0 and saturation status to no */
    self->p_integPreviousValue = 0;
    self->p_saturationStatus   = Ifx_Math_Pi_F16_AntiWindupCtrl_no;

    /* Initialize qformat of error and the output */
    self->p_qFormatError  = qformatsPi.qFormatError;
    self->p_qFormatOutput = qformatsPi.qFormatOutput;

    /* Init. gains to default values */
    Ifx_Math_Fract16Q defaultGain = {
        .value = IFX_MATH_FRACT16_MAX, .qFormat = Ifx_Math_FractQFormat_q15
    };
    Ifx_Math_Pi_F16_setPropGain(self, defaultGain);
    Ifx_Math_Pi_F16_setIntegGainSamplingTime(self, defaultGain);
    Ifx_Math_Pi_F16_setAntiWindupGainSamplingTime(self, defaultGain);
}


Ifx_Math_Fract16 Ifx_Math_Pi_F16_execute(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue,
                                         Ifx_Math_Pi_F16_AntiWindupCtrl antiWindupCtrl)
{
    Ifx_Math_Fract16 result;

    /* Implements a trapezoidal discretization of the integrator */
#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_TRAPEZOIDAL

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC
    result = Ifx_Math_Pi_F16_p_trapezoidal_backCalculation(self, errorValue);
#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP
    result = Ifx_Math_Pi_F16_p_trapezoidal_clamp(self, errorValue, antiWindupCtrl);
#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_NONE
    result = Ifx_Math_Pi_F16_p_trapezoidal(self, errorValue);
#endif

    /* Implements a Forward Euler discretization of the integrator */
#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_FORWARD

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC
    result = Ifx_Math_Pi_F16_p_forwardEuler_backCalculation(self, errorValue);
#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP
    result = Ifx_Math_Pi_F16_p_forwardEuler_clamp(self, errorValue, antiWindupCtrl);
#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_NONE
    result = Ifx_Math_Pi_F16_p_forwardEuler(self, errorValue);
#endif

    /* Implements a Backward Euler discretization of the integrator */
#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_BACKWARD

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC
    result = Ifx_Math_Pi_F16_p_backEuler_backCalculation(self, errorValue);
#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP
    result = Ifx_Math_Pi_F16_p_backEuler_clamp(self, errorValue, antiWindupCtrl);
#elif IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_NONE
    result = Ifx_Math_Pi_F16_p_backEuler(self, errorValue);
#endif

#endif

    return result;
}


void Ifx_Math_Pi_F16_setPropGain(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16Q propGain)
{
    /* Update internal parameter */
    self->p_propGain = propGain;

    /* Update internal Q formats */
    uint8_t tempQFormat = (uint8_t)self->p_qFormatOutput + (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT;
    self->p_propGainInternQFormat = Ifx_Math_MulShR_ShiftMul(self->p_propGain.qFormat, self->p_qFormatError,
        (Ifx_Math_FractQFormat)tempQFormat);
}


void Ifx_Math_Pi_F16_setIntegGainSamplingTime(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16Q integGainSamplingTime)
{
    /* Update internal parameter */
    self->p_integGainSamplingTime = integGainSamplingTime;

    /* Update internal Q formats */
    uint8_t tempQFormat = (uint8_t)self->p_qFormatOutput + (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT;
    self->p_integGainInternQFormat = Ifx_Math_MulShR_ShiftMul(self->p_integGainSamplingTime.qFormat,
        self->p_qFormatError, (Ifx_Math_FractQFormat)tempQFormat) + IFX_MATH_PI_F16_SAMPLING_TIME_DIVIDER;
}


void Ifx_Math_Pi_F16_setAntiWindupGainSamplingTime(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16Q
                                                   antiWindupGainSamplingTime)
{
    /* Update internal parameter */
    self->p_antiWindupGainSamplingTime = antiWindupGainSamplingTime;

    /* Update internal Q formats */
    uint8_t tempQFormat = (uint8_t)self->p_qFormatOutput + (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT;
    self->p_antiWindupGainInternQFormat = Ifx_Math_MulShR_ShiftMul(self->p_antiWindupGainSamplingTime.qFormat,
        (Ifx_Math_FractQFormat)tempQFormat, (Ifx_Math_FractQFormat)tempQFormat) +
                                          IFX_MATH_PI_F16_SAMPLING_TIME_DIVIDER;
}


/* Saturate the integral gain previous value between the defined limits */
static inline void Ifx_Math_Pi_F16_p_saturatePreviousValue(Ifx_Math_Pi_F16* self)
{
    /* Previous value */
    Ifx_Math_Fract32 previousValueTemp;

    /* Upper limit for the previous value, in the intermediate Q format */
    Ifx_Math_Fract32 previousValueTempUpp;

    /* Lower limit for the previous value, in the intermediate Q format */
    Ifx_Math_Fract32 previousValueTempLow;

    /* Read previous value */
    previousValueTemp = self->p_integPreviousValue;

    /* Previous value saturation */
    previousValueTempUpp = Ifx_Math_ShL_F32(self->p_upperLimit, (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT);
    previousValueTempLow = Ifx_Math_ShL_F32(self->p_lowerLimit, (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT);

    if (previousValueTemp > previousValueTempUpp)
    {
        self->p_integPreviousValue = previousValueTempUpp;
    }
    else if (previousValueTemp < previousValueTempLow)
    {
        self->p_integPreviousValue = previousValueTempLow;
    }
    else
    {
        self->p_integPreviousValue = previousValueTemp;
    }
}


/* Saturate the output between the defined limits */
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_saturateOutput(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 out)
{
    Ifx_Math_Fract16 outSat;

    if (out >= self->p_upperLimit)
    {
        outSat                   = self->p_upperLimit;
        self->p_saturationStatus = Ifx_Math_Pi_F16_AntiWindupCtrl_pos;
    }
    else if (out <= self->p_lowerLimit)
    {
        outSat                   = self->p_lowerLimit;
        self->p_saturationStatus = Ifx_Math_Pi_F16_AntiWindupCtrl_neg;
    }
    else
    {
        outSat                   = out;
        self->p_saturationStatus = Ifx_Math_Pi_F16_AntiWindupCtrl_no;
    }

    return outSat;
}


/* Calculate the product between the error and the gain and return the intermediate Q format */
static inline void Ifx_Math_Pi_F16_p_calculateGainsError(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue,
                                                         Ifx_Math_Fract32* propGainError,
                                                         Ifx_Math_Fract32* integGainError)
{
    /* Calculate proportional part (U_p) */
    *propGainError = Ifx_Math_MulShR_F32_F16F16(self->p_propGain.value, errorValue, self->p_propGainInternQFormat);

    /* Calculate integral part (U_i) */
    *integGainError = Ifx_Math_MulShR_F32_F16F16(self->p_integGainSamplingTime.value, errorValue,
        self->p_integGainInternQFormat);
}


/* Check internal antiwindup conditions */
static inline int8_t Ifx_Math_Pi_F16_p_antiWindupCtrlIntern(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue)
{
    int8_t antiWindupCtrlIntern = 0;

    if ((Ifx_Math_Pi_F16_AntiWindupCtrl_no == self->p_saturationStatus)
        || ((0 > errorValue)
            && (Ifx_Math_Pi_F16_AntiWindupCtrl_pos == self->p_saturationStatus))
        || ((0 < errorValue)
            && (Ifx_Math_Pi_F16_AntiWindupCtrl_neg == self->p_saturationStatus)))
    {
        antiWindupCtrlIntern = 1;
    }

    return antiWindupCtrlIntern;
}


/* Check external antiwindup conditions */
static inline int8_t Ifx_Math_Pi_F16_p_antiWindupCtrlExtern(Ifx_Math_Fract16               errorValue,
                                                            Ifx_Math_Pi_F16_AntiWindupCtrl antiWindupCtrl)
{
    int8_t antiWindupCtrlExtern = 0;

    /* Check external antiwindup control */
    if (((Ifx_Math_Pi_F16_AntiWindupCtrl_no == antiWindupCtrl)
         || ((0 > errorValue)
             && (Ifx_Math_Pi_F16_AntiWindupCtrl_pos == antiWindupCtrl))
         || ((0 < errorValue)
             && (Ifx_Math_Pi_F16_AntiWindupCtrl_neg == antiWindupCtrl))))
    {
        antiWindupCtrlExtern = 1;
    }

    return antiWindupCtrlExtern;
}


/* Check internal and external antiwindup conditions */
static inline int8_t Ifx_Math_Pi_F16_p_antiWindupControl(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue,
                                                         Ifx_Math_Pi_F16_AntiWindupCtrl antiWindupCtrl)
{
    /* Internal variable */
    int8_t antiWindupSwitch;
    int8_t antiWindupCtrlExtern = Ifx_Math_Pi_F16_p_antiWindupCtrlExtern(errorValue, antiWindupCtrl);
    int8_t antiWindupCtrlintern = Ifx_Math_Pi_F16_p_antiWindupCtrlIntern(self, errorValue);
    antiWindupSwitch = antiWindupCtrlExtern * antiWindupCtrlintern;

    return antiWindupSwitch;
}


#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC

/* Calculate the antiwindup part using the back calculation method */
static inline Ifx_Math_Fract32 Ifx_Math_Pi_F16_p_backCalculation(Ifx_Math_Pi_F16* self, Ifx_Math_Fract32 errorValue)
{
    /* Upper limit for the previous integral value */
    Ifx_Math_Fract32 integStateTempUpp;

    /* Lower limit for the previous integral value */
    Ifx_Math_Fract32 integStateTempLow;

    /* Anti windup coefficient */
    Ifx_Math_Fract32 coefficient;
    integStateTempUpp = Ifx_Math_ShL_F32(self->p_upperLimit, (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT);
    integStateTempLow = Ifx_Math_ShL_F32(self->p_lowerLimit, (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT);

    /* Back calculation */
    if (errorValue > integStateTempUpp)
    {
        /* Calculate anti-windup part (U_aw)  */
        coefficient = Ifx_Math_MulShR_F32(self->p_antiWindupGainSamplingTime.value, Ifx_Math_Sub_F32(
            integStateTempUpp, errorValue), self->p_antiWindupGainInternQFormat);
    }
    else if (errorValue < integStateTempLow)
    {
        /* Calculate anti-windup part (U_aw) */
        coefficient = Ifx_Math_MulShR_F32(self->p_antiWindupGainSamplingTime.value, Ifx_Math_Sub_F32(
            integStateTempLow, errorValue), self->p_antiWindupGainInternQFormat);
    }
    else
    {
        coefficient = 0;
    }

    return coefficient;
}


#endif /* IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC */

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_TRAPEZOIDAL
#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_NONE
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_trapezoidal(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue)
{
    /* (Proportional gain * input error) */
    Ifx_Math_Fract32 propGainError;

    /* (Integral gain * input error) */
    Ifx_Math_Fract32 integGainError;

    /* (Integral gain * input error) + previous integral value */
    Ifx_Math_Fract32 integGainErrorPrevVal;

    /* Unsaturated output */
    Ifx_Math_Fract16 out;

    /* Saturated output */
    Ifx_Math_Fract16 outSat;
    (void)Ifx_Math_Pi_F16_p_calculateGainsError(self, errorValue, &propGainError, &integGainError);
    integGainErrorPrevVal = Ifx_Math_Add_F32(integGainError, self->p_integPreviousValue);

    /* Calculate output (U) */
    out    = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(Ifx_Math_Add_F32(propGainError, integGainErrorPrevVal),
        (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT));
    outSat = Ifx_Math_Pi_F16_p_saturateOutput(self, out);

    /* Update state */
    self->p_integPreviousValue = Ifx_Math_Add_F32(integGainError, integGainErrorPrevVal);

    return outSat;
}


#endif /* IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_NONE */

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_trapezoidal_clamp(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue,
                                                                   Ifx_Math_Pi_F16_AntiWindupCtrl antiWindupCtrl)
{
    /* Unsaturated output */
    Ifx_Math_Fract16 out;

    /* Calculate output using regular Trapezoidal method */
    /* (Proportional gain * input error) */
    Ifx_Math_Fract32 propGainError;

    /* (Integral gain * input error) */
    Ifx_Math_Fract32 integGainError;

    /* Saturated output */
    Ifx_Math_Fract16 outSat;

    /* Calculate gains times error */
    (void)Ifx_Math_Pi_F16_p_calculateGainsError(self, errorValue, &propGainError, &integGainError);

    /* Check antiwindup status */
    int8_t antiWindupSwitch = Ifx_Math_Pi_F16_p_antiWindupControl(self, errorValue, antiWindupCtrl);

    /* First half integration */
    self->p_integPreviousValue = Ifx_Math_Add_F32(integGainError * antiWindupSwitch, self->p_integPreviousValue);

    /* Saturate previous value after first half integration */
    Ifx_Math_Pi_F16_p_saturatePreviousValue(self);

    /* Calculate output (U) */
    out = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(Ifx_Math_Add_F32(propGainError, self->p_integPreviousValue),
        (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT));

    /* Second half integration */
    self->p_integPreviousValue = Ifx_Math_Add_F32(integGainError * antiWindupSwitch, self->p_integPreviousValue);

    /* Saturate previous value after second half integration */
    Ifx_Math_Pi_F16_p_saturatePreviousValue(self);

    /* Saturate output */
    outSat = Ifx_Math_Pi_F16_p_saturateOutput(self, out);

    return outSat;
}


#endif /* IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP */

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_trapezoidal_backCalculation(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16
                                                                             errorValue)
{
    /* Unsaturated output */
    Ifx_Math_Fract16 out;

    /* Saturated output */
    Ifx_Math_Fract16 outSat;

    /* (Proportional gain * input error) */
    Ifx_Math_Fract32 propGainError;

    /* (Integral gain * input error) */
    Ifx_Math_Fract32 integGainError;

    /* (Integral gain * input error) + previous integral value */
    Ifx_Math_Fract32 integGainErrorPrevVal;

    /* (Proportional gain * input error) + previous integral value */
    Ifx_Math_Fract32 propGainErrorPrevVal;

    /* Anti windup coefficient */
    Ifx_Math_Fract32 antiWindup;

    /* Q format for intermediate calculations */
    Ifx_Math_Pi_F16_p_calculateGainsError(self, errorValue, &propGainError, &integGainError);

    /* (Proportional gain * input error) + previous integral value */
    propGainErrorPrevVal = Ifx_Math_Add_F32(propGainError, self->p_integPreviousValue);

    /* Calculate anti windup coefficient */
    antiWindup            = Ifx_Math_Pi_F16_p_backCalculation(self, propGainErrorPrevVal);
    integGainErrorPrevVal = Ifx_Math_Add_F32(antiWindup, Ifx_Math_Add_F32(integGainError,
        self->p_integPreviousValue));

    /* Calculate output (U) */
    out    = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(Ifx_Math_Add_F32(propGainError, integGainErrorPrevVal),
        (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT));
    outSat = Ifx_Math_Pi_F16_p_saturateOutput(self, out);

    /* Update state */
    self->p_integPreviousValue = Ifx_Math_Add_F32(Ifx_Math_Add_F32(antiWindup, integGainError),
        integGainErrorPrevVal);

    return outSat;
}


#endif /* IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC */

#endif

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_FORWARD

static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_forwardEuler(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue)
{
    /* Unsaturated output */
    Ifx_Math_Fract16 out;

    /* Saturated output */
    Ifx_Math_Fract16 outSat;

    /* (Proportional gain * input error) */
    Ifx_Math_Fract32 propGainError;

    /* (Integral gain * input error) */
    Ifx_Math_Fract32 integGainError;
    (void)Ifx_Math_Pi_F16_p_calculateGainsError(self, errorValue, &propGainError, &integGainError);

    /* Calculate output (U) */
    out    = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(Ifx_Math_Add_F32(propGainError, self->p_integPreviousValue),
        (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT));
    outSat = Ifx_Math_Pi_F16_p_saturateOutput(self, out);

    /* Update state */
    self->p_integPreviousValue = Ifx_Math_Add_F32(integGainError, self->p_integPreviousValue);

    return outSat;
}


#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_forwardEuler_clamp(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16
                                                                    errorValue, Ifx_Math_Pi_F16_AntiWindupCtrl
                                                                    antiWindupCtrl)
{
    /* Unsaturated output */
    Ifx_Math_Fract16 out;

    /* Saturated output */
    Ifx_Math_Fract16 outSat;

    /* (Proportional gain * input error) */
    Ifx_Math_Fract32 propGainError;

    /* (Integral gain * input error) */
    Ifx_Math_Fract32 integGainError;
    (void)Ifx_Math_Pi_F16_p_calculateGainsError(self, errorValue, &propGainError, &integGainError);

    /* Calculate output (U) */
    out = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(Ifx_Math_Add_F32(propGainError, self->p_integPreviousValue),
        (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT));

    /* Saturate output */
    outSat = Ifx_Math_Pi_F16_p_saturateOutput(self, out);

    /* Check antiwindup status */
    int8_t antiWindupSwitch = Ifx_Math_Pi_F16_p_antiWindupControl(self, errorValue, antiWindupCtrl);

    /* Integrate */
    self->p_integPreviousValue = Ifx_Math_Add_F32(integGainError * antiWindupSwitch, self->p_integPreviousValue);

    /* Clamp integrator if it exceeds limit */
    Ifx_Math_Pi_F16_p_saturatePreviousValue(self);

    return outSat;
}


#endif /* IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP */

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_forwardEuler_backCalculation(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16
                                                                              errorValue)
{
    /* (Proportional gain * input error) */
    Ifx_Math_Fract32 propGainError;

    /* (Integral gain * input error) */
    Ifx_Math_Fract32 integGainError;

    /* Anti windup coefficient */
    Ifx_Math_Fract32 antiWindup;

    /* Unsaturated output (32 bit) */
    Ifx_Math_Fract32 outF32;

    /* Unsaturated output (16 bit)*/
    Ifx_Math_Fract16 outF16;

    /* Saturated output */
    Ifx_Math_Fract16 outSat;

    /* Q format for intermediate calculations */
    Ifx_Math_Pi_F16_p_calculateGainsError(self, errorValue, &propGainError, &integGainError);

    /* Calculate the first output, before back calculation */
    outF32 = Ifx_Math_Add_F32(propGainError, self->p_integPreviousValue);

    /* Calculate anti windup coefficient using back calculation */
    antiWindup = Ifx_Math_Pi_F16_p_backCalculation(self, outF32);

    /* Calculate the unsaturated output */
    outF16 = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(outF32, (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT));
    outSat = Ifx_Math_Pi_F16_p_saturateOutput(self, outF16);

    /* Update previous value */
    self->p_integPreviousValue = Ifx_Math_Add_F32(self->p_integPreviousValue, Ifx_Math_Add_F32(integGainError,
        antiWindup));

    return outSat;
}


#endif /* IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC */

#endif

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_DISC == IFX_MATH_USROPT_PI_DISC_BACKWARD
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_backEuler(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue)
{
    /* Unsaturated output */
    Ifx_Math_Fract16 out;

    /* Saturated output */
    Ifx_Math_Fract16 outSat;

    /* (Proportional gain * input error) */
    Ifx_Math_Fract32 propGainError;

    /* (Integral gain * input error) */
    Ifx_Math_Fract32 integGainError;
    (void)Ifx_Math_Pi_F16_p_calculateGainsError(self, errorValue, &propGainError, &integGainError);

    /* Update previous value */
    self->p_integPreviousValue = Ifx_Math_Add_F32(integGainError, self->p_integPreviousValue);

    /* Calculate output (U) */
    out    = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(Ifx_Math_Add_F32(propGainError, self->p_integPreviousValue),
        (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT));
    outSat = Ifx_Math_Pi_F16_p_saturateOutput(self, out);

    return outSat;
}


#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_backEuler_clamp(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16 errorValue,
                                                                 Ifx_Math_Pi_F16_AntiWindupCtrl antiWindupCtrl)
{
    /* Unsaturated output */
    Ifx_Math_Fract16 out;

    /* Saturated output */
    Ifx_Math_Fract16 outSat;

    /* (Proportional gain * input error) */
    Ifx_Math_Fract32 propGainError;

    /* (Integral gain * input error) */
    Ifx_Math_Fract32 integGainError;
    (void)Ifx_Math_Pi_F16_p_calculateGainsError(self, errorValue, &propGainError, &integGainError);

    /* Check antiwindup status */
    int8_t           antiWindupSwitch = Ifx_Math_Pi_F16_p_antiWindupControl(self, errorValue, antiWindupCtrl);

    /* Integrate */
    self->p_integPreviousValue = Ifx_Math_Add_F32(integGainError * antiWindupSwitch, self->p_integPreviousValue);

    /* Saturate state */
    Ifx_Math_Pi_F16_p_saturatePreviousValue(self);

    /* Calculate output (U) */
    out    = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(Ifx_Math_Add_F32(propGainError, self->p_integPreviousValue),
        (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT));
    outSat = Ifx_Math_Pi_F16_p_saturateOutput(self, out);

    return outSat;
}


#endif /* IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_CLAMP */

#if IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC
static inline Ifx_Math_Fract16 Ifx_Math_Pi_F16_p_backEuler_backCalculation(Ifx_Math_Pi_F16* self, Ifx_Math_Fract16
                                                                           errorValue)
{
    /* (Proportional gain * input error) + previous integral value */
    Ifx_Math_Fract32 propGainErrorPrevVal;

    /* Unsaturated output */
    Ifx_Math_Fract16 out;

    /* (Proportional gain * input error) */
    Ifx_Math_Fract32 propGainError;

    /* (Integral gain * input error) */
    Ifx_Math_Fract32 integGainError;

    /* Anti windup coefficient */
    Ifx_Math_Fract32 antiWindup;

    /* Q format for intermediate calculations */
    Ifx_Math_Pi_F16_p_calculateGainsError(self, errorValue, &propGainError, &integGainError);

    /* (Proportional gain * input error) + previous value */
    propGainErrorPrevVal = Ifx_Math_Add_F32(propGainError, self->p_integPreviousValue);

    /* Calculate anti windup coefficient using back calculation */
    antiWindup = Ifx_Math_Pi_F16_p_backCalculation(self, propGainErrorPrevVal);

    /* Update previous value */
    self->p_integPreviousValue = Ifx_Math_Add_F32(self->p_integPreviousValue, Ifx_Math_Add_F32(integGainError,
        antiWindup));

    /* Calculate output (U) */
    out = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(Ifx_Math_Add_F32(propGainError, self->p_integPreviousValue),
        (uint8_t)IFX_MATH_PI_F16_INT_Q_SHIFT));

    return Ifx_Math_Pi_F16_p_saturateOutput(self, out);
}


#endif /* IFX_MATH_ADVANCEDMATH_F16_CFG_PI_ANTI_WIND == IFX_MATH_USROPT_PI_ANTI_WIND_BACK_CALC */

#endif
