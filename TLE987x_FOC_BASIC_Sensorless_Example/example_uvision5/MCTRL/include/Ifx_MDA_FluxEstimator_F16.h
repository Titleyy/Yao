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
 * \file Ifx_MDA_FluxEstimator_F16.h
 * \brief Estimator for the rotor flux position and speed.
 * This module takes as input the stator voltage and current and outputs the estimated normalized speed and the
 * estimated position of rotor flux (in radians).
 * In order to perform the estimation, all the macros in the file <u>Ifx_MDA_FluxEstimatorF16_Cfg</u>.h must be
 * configured.
 */

#ifndef IFX_MDA_FLUXESTIMATOR_F16_H
#define IFX_MDA_FLUXESTIMATOR_F16_H
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "Ifx_Math.h"
#include "Ifx_Math_LowPass1st_F16.h"
#include "Ifx_Math_PLL_F16.h"

/**
 * Contains the module output variables.
 */
typedef struct Ifx_MDA_FluxEstimator_F16_Output
{
    /**
     * Estimated speed, normalized
     */
    Ifx_Math_Fract16 speedQ15;

    /**
     * Estimated position of the rotor flux
     */
    uint32_t anglePLL;
} Ifx_MDA_FluxEstimator_F16_Output;

/**
 * Parameter structure containing configuration of the component
 */
typedef struct Ifx_MDA_FluxEstimator_F16_Param
{
    /**
     * Time constant of first order low pass filter for the stator alpha components
     */
    uint32_t alphaFilter_timeConstant_us;

    /**
     * Time constant of first order low pass filter for the stator beta components
     */
    uint32_t betaFilter_timeConstant_us;

    /**
     * Time constant of first order low pass filter for the speed
     */
    uint32_t speedFilter_timeConstant_us;

    /**
     * Proportional gain of phase Locked Loop filter for the angle and speed estimation
     */
    uint32_t pllFilter_propGain;

    /**
     * Sampling time, in microseconds
     */
    uint16_t samplingTime_us;

    /**
     * Adjusted phase inductance.
     * Adjusted alpha and beta gains and inductance. The adjustment factor is defined because usually the filter time
     * constants are relatively large compared to the other time values on the system. Due to this fact, the system base
     * time might not be enough to normalize the  filter gains (which are the same as the time constants) and cause
     * overflows. If the flux amplitude needs to be calculated from SW values, this factor should be considered in the
     * scaling.
     */
    Ifx_Math_Fract16 phaseIndAdjustedQ15;

    /**
     * Adjusted alpha gain.
     */
    Ifx_Math_Fract16 alphaGainAdjustedQ14;

    /**
     * Adjusted beta gain.
     */
    Ifx_Math_Fract16 betaGainAdjustedQ14;

    /**
     * System base time as Q30
     */
    Ifx_Math_Fract32 systemBaseTimeQ30;

    /**
     * Phase resistance Q15
     */
    Ifx_Math_Fract16 phaseResQ15;
} Ifx_MDA_FluxEstimator_F16_Param;

/**
 * Modes of the Flux Estimator
 */
typedef enum Ifx_MDA_FluxEstimator_F16_Mode
{
    Ifx_MDA_FluxEstimator_F16_Mode_disable = 0, /**<Estimation is disabled, speed is set to zero and estimated angle is
                                                 * kept at last value*/
    Ifx_MDA_FluxEstimator_F16_Mode_enable  = 1, /**<Speed and angle estimation of the rotor flux is enabled*/
} Ifx_MDA_FluxEstimator_F16_Mode;

/**
 * \brief Data structure that stores all data of module instance.
 */
typedef struct Ifx_MDA_FluxEstimator_F16
{
    /**
     * First order low pass filter for the stator alpha components
     */
    Ifx_Math_LowPass1st_F16 p_alphaFilter;

    /**
     * First order low pass filter for the stator beta components
     */
    Ifx_Math_LowPass1st_F16 p_betaFilter;

    /**
     * First order low pass filter for the speed
     */
    Ifx_Math_LowPass1st_F16 p_speedFilter;

    /**
     * Phase Locked Loop filter for the angle and speed estimation
     */
    Ifx_Math_PLL_F16 p_pllFilter;

    /**
     * Contains the output variables of the module.
     */
    Ifx_MDA_FluxEstimator_F16_Output p_output;

    /**
     * Conversion factor from rad to rad/s
     */
    Ifx_Math_Fract16 p_radToRadPerSecondQ7;

    /**
     * Configured operation mode of the Flux Estimator
     */
    Ifx_MDA_FluxEstimator_F16_Mode p_mode;

    /**
     * Adjusted phase inductance. (copy of the dynamic parameter for improved execution time)
     */
    Ifx_Math_Fract16 p_phaseIndAdjustedQ15;

    /**
     * Phase resistance. (copy of the dynamic parameter for improved execution time)
     */
    Ifx_Math_Fract16 p_phaseResQ15;

    /**
     * Parameter structure
     */
    Ifx_MDA_FluxEstimator_F16_Param* param;
} Ifx_MDA_FluxEstimator_F16;

/**
 * Global variable which holds default parameter values
 */
extern Ifx_MDA_FluxEstimator_F16_Param Ifx_MDA_FluxEstimator_F16_g_defaultParam;

/**
 *  \brief Initialize the module to the default values and to the values configured in
 *  Config Wizard.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [inout] param Pointer to parameter structure containing configuration of the component
 */
void Ifx_MDA_FluxEstimator_F16_init(Ifx_MDA_FluxEstimator_F16* self, Ifx_MDA_FluxEstimator_F16_Param* param);

/**
 *  \brief Perform estimation of the rotor flux rotational speed and position.
 *
 *  If the module is enabled, this API performs the speed and angle estimation of
 *  the rotor flux. In case of mode disable, this API does not perform calculations,
 *  but sets the returned speed to zero and the returned estimated angle to the last
 *  value.
 *
 *  Inputs of this API:
 *  <ul>
 *  <li>Stator voltage in stator reference frame, normalized by CFG_BASE_VOLTAGE_V
 *  in Q15 format</li>
 *  <li>Stator current in stator reference frame, normalized by CFG_BASE_CURRENT_A
 *  in Q15 format</li>
 *  </ul>
 *
 *  Outputs of this API:
 *  <ul>
 *  <li>Estimated rotor flux speed, normalized by BASE_ELEC_SPEED_RADPS in Q15
 *  format</li>
 *  <li>Estimated rotor flux angle between 0 and 2*pi, normalized to 0 to
 *  2^32-1</li>
 *  </ul>
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] statorVoltage Structure containing the normalized stator voltage, in 1Q15 format
 *
 *  \param [in] statorCurrent Structure containing the normalized stator current, in 1Q15 format
 */
void Ifx_MDA_FluxEstimator_F16_execute(Ifx_MDA_FluxEstimator_F16* self, Ifx_Math_CmpFract16 statorVoltage,
                                       Ifx_Math_CmpFract16 statorCurrent);

/**
 *  \brief Configure the mode of the Flux Estimator.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] mode Control input to set the mode of the FluxEstimator
 */
static inline void Ifx_MDA_FluxEstimator_F16_configMode(Ifx_MDA_FluxEstimator_F16    * self,
                                                        Ifx_MDA_FluxEstimator_F16_Mode mode)
{
    self->p_mode = mode;
}


/**
 *  \brief Set the discrete sampling time period in microseconds, between [1us, 65535us].
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] samplingTime_us Sampling time, in microseconds
 */
void Ifx_MDA_FluxEstimator_F16_setSamplingTime(Ifx_MDA_FluxEstimator_F16* self, uint16_t samplingTime_us);

/**
 *  \brief Get the discrete sampling time period in microseconds.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Sampling time, in microseconds
 */
static inline uint16_t Ifx_MDA_FluxEstimator_F16_getSamplingTime(Ifx_MDA_FluxEstimator_F16* self)
{
    return self->param->samplingTime_us;
}


/**
 *  \brief Get the module output variables.
 *
 *  The output contains the estimated speed, represented in Q15, and the estimated
 *  flux angle.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [out] output Structure containing the module outputs
 */
static inline void Ifx_MDA_FluxEstimator_F16_getOutput(Ifx_MDA_FluxEstimator_F16       * self,
                                                       Ifx_MDA_FluxEstimator_F16_Output* output)
{
    *output = self->p_output;
}


/**
 *  \brief Returns the component ID
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 */
void Ifx_MDA_FluxEstimator_F16_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Get the active mode of the Flux Estimator.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Mode
 */
static inline Ifx_MDA_FluxEstimator_F16_Mode Ifx_MDA_FluxEstimator_F16_getMode(Ifx_MDA_FluxEstimator_F16* self)
{
    return self->p_mode;
}


/**
 *  \brief Returns the component version
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 */
void Ifx_MDA_FluxEstimator_F16_getVersion(const Ifx_ComponentVersion** componentVersion);

/**
 *  \brief Set the time constant of first order low pass filter for the stator alpha
 *  components in microseconds
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] alphaFilter_timeConstant_us Time constant of first order low pass filter for the stator alpha components
 */
void Ifx_MDA_FluxEstimator_F16_setAlphaTimeConstant(Ifx_MDA_FluxEstimator_F16* self, uint32_t
                                                    alphaFilter_timeConstant_us);

/**
 *  \brief Set the time constant of first order low pass filter for the stator beta
 *  components in microseconds
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] betaFilter_timeConstant_us Time constant of first order low pass filter for the stator beta components
 */
void Ifx_MDA_FluxEstimator_F16_setBetaTimeConstant(Ifx_MDA_FluxEstimator_F16* self, uint32_t
                                                   betaFilter_timeConstant_us);

/**
 *  \brief Set the time constant of first order low pass filter for the speed in
 *  microseconds
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] speedFilter_timeConstant_us Time constant of first order low pass filter for the speed
 */
void Ifx_MDA_FluxEstimator_F16_setSpeedFilterTimeConstant(Ifx_MDA_FluxEstimator_F16* self, uint32_t
                                                          speedFilter_timeConstant_us);

/**
 *  \brief Set the gain of phase locked loop filter for the angle and speed estimation.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] pllPropGain Proportional gain of phase Locked Loop filter for the angle and speed estimation
 */
void Ifx_MDA_FluxEstimator_F16_setPllFilterPropGain(Ifx_MDA_FluxEstimator_F16* self, uint32_t pllPropGain);

/**
 *  \brief Set the phase resistance for the angle and speed estimation.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] phaseResQ15 phase resistance for the angle and speed estimation
 */
void Ifx_MDA_FluxEstimator_F16_setPhaseRes(Ifx_MDA_FluxEstimator_F16* self, Ifx_Math_Fract16 phaseResQ15);

/**
 *  \brief Set the adjusted phase inductance for the angle and speed estimation.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] phaseIndAdjustedQ15 adjusted phase inductance for the angle and speed estimation
 */
void Ifx_MDA_FluxEstimator_F16_setPhaseIndAdjusted(Ifx_MDA_FluxEstimator_F16* self, Ifx_Math_Fract16
                                                   phaseIndAdjustedQ15);

#endif /*IFX_MDA_FLUXESTIMATOR_F16_H*/
