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
 * \file Ifx_MDA_FocController_F16.h
 * \brief 16-bit implementation of the field oriented controller.
 * This module takes as input the currents in alpha-beta format, the direct and quadrature reference currents and the
 * rotor flux angle, performs the current control using two PI controllers, and outputs the voltage command, in polar
 * format. It also takes the electrical speed as input and performs dq decoupling if enabled​.
 */

#ifndef IFX_MDA_FOCCONTROLLER_F16_H
#define IFX_MDA_FOCCONTROLLER_F16_H
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "Ifx_MDA_FocController_F16_Cfg.h"
#include "Ifx_Math.h"
#include "Ifx_Math_DqDecoupling_F16.h"
#include "Ifx_Math_Pi_F16.h"

/**
 * Contains the module output variables.
 */
typedef struct Ifx_MDA_FocController_F16_Output
{
    /**
     * Voltage command in polar form
     */
    Ifx_Math_PolarFract16 voltageCommandPolar;
} Ifx_MDA_FocController_F16_Output;

/**
 * Parameter structure containing configuration of the component
 */
typedef struct Ifx_MDA_FocController_F16_Param
{
    /**
     * Direct current PI proportional gain
     */
    uint32_t currentDPiPropGain;

    /**
     * Direct current PI integral gain * sampling time
     */
    uint32_t currentDPiIntegGainSamplingTime;

    /**
     * Direct current PI antiwindup gain * sampling time
     */
    uint32_t currentDPiAntiWindupGainSamplingTime;

    /**
     * Direct current PI upper limit
     */
    Ifx_Math_Fract16 currentDPiUpperLimitQ15;

    /**
     * Direct current PI lower limit
     */
    Ifx_Math_Fract16 currentDPiLowerLimitQ15;

    /**
     * Quadrature current PI proportional gain
     */
    uint32_t currentQPiPropGain;

    /**
     * Quadrature current PI integral gain * sampling time
     */
    uint32_t currentQPiIntegGainSamplingTime;

    /**
     * Quadrature current PI antiwindup gain * sampling time
     */
    uint32_t currentQPiAntiWindupGainSamplingTime;

    /**
     * Quadrature current PI upper limit
     */
    Ifx_Math_Fract16 currentQPiUpperLimitQ15;

    /**
     * Quadrature current PI lower limit
     */
    Ifx_Math_Fract16 currentQPiLowerLimitQ15;
} Ifx_MDA_FocController_F16_Param;

/**
 * \brief Data structure that stores all data of module instance.
 */
typedef struct Ifx_MDA_FocController_F16
{
    /**
     * Pointer to parameter structure
     */
    Ifx_MDA_FocController_F16_Param* param;

    /**
     * Instance of PI for the direct current
     */
    Ifx_Math_Pi_F16 currentDPi;

    /**
     * Current in the d-q reference frame
     */
    Ifx_Math_CmpFract16 currentDQ;

    /**
     * Instance of PI for the quadrature current
     */
    Ifx_Math_Pi_F16 currentQPi;

    /**
     * Instance of dqDecoupling
     */
    Ifx_Math_DqDecoupling_F16 dqDecoupling;

    /**
     * Contains the module output variables
     */
    Ifx_MDA_FocController_F16_Output p_output;

    /**
     * Voltage in the d-q reference frame
     */
    Ifx_Math_CmpFract16 voltageDQ;

    /**
     * D-PI controller  antiWindup control
     */
    Ifx_Math_Pi_F16_AntiWindupCtrl p_dAntiwindupCtrl;

    /**
     * Q-PI controller  antiWindup control
     */
    Ifx_Math_Pi_F16_AntiWindupCtrl p_qAntiwindupCtrl;
} Ifx_MDA_FocController_F16;

/**
 * Global variable which holds default parameter values
 */
extern Ifx_MDA_FocController_F16_Param Ifx_MDA_FocController_F16_g_defaultParam;

/**
 *  \brief Returns the component ID
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 */
void Ifx_MDA_FocController_F16_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Returns the component version
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 */
void Ifx_MDA_FocController_F16_getVersion(const Ifx_ComponentVersion** componentVersion);

/**
 *  \brief Initialize the module to the default values.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [inout] param Pointer to parameter structure containing configuration of the component
 */
void Ifx_MDA_FocController_F16_init(Ifx_MDA_FocController_F16* self, Ifx_MDA_FocController_F16_Param* param);

/**
 *  \brief Execute the current regulation and output the voltage command.
 *
 *  This API executes the current regulation of the Field Oriented Controller (FOC).
 *  It performs the Park transformation and executes the PI controllers of the D and
 *  Q currents and outputs the resulting voltage vector in polar coordinates.
 *  Additionally, if enabled, it compensates for the d-q decoupling, and if enabled,
 *  it limits the amplitude of the output voltage vector according to the available
 *  DC link voltage with a D component prioritization.
 *
 *  Inputs to this API:
 *  <ul>
 *  <li>Reference D and Q currents, normalized in Q15 format</li>
 *  <li>Measured alpha and beta currents, normalized in Q15 format</li>
 *  <li>Rotor flux angle between 0 and 2*pi, normalized to 0 to 2^32-1</li>
 *  <li>Electrical speed, needed to compensate for the d-q decoupling, normalized in
 *  Q15 format</li>
 *  <li>DC Link voltage, needed to limit the amplitude of the output voltage vector,
 *  normalized in Q15 format</li>
 *  </ul>
 *
 *  Outputs to this API:
 *  <ul>
 *  <li>Calculated voltage vector in polar coordinates containing the amplitude
 *  normalized in Q15 format and the angle between 0 and 2*pi normalized to 0 to
 *  2^32-1</li>
 *  </ul>
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] currentAlphaBeta Measured current, in alpha-beta reference frame
 *
 *  \param [in] dqCommand Current command, in d-q reference frame
 *
 *  \param [in] rotorFluxAngle Rotor flux angle
 *
 *  \param [in] electricalSpeed Normalized electrical speed
 *
 *  \param [in] dcLinkVoltageQ15 Normalized dcLinkVoltage
 */
void Ifx_MDA_FocController_F16_execute(Ifx_MDA_FocController_F16* self, Ifx_Math_CmpFract16 currentAlphaBeta,
                                       Ifx_Math_CmpFract16 dqCommand, uint32_t rotorFluxAngle, Ifx_Math_Fract16
                                       electricalSpeed, Ifx_Math_Fract16
                                       dcLinkVoltageQ15);

/**
 *  \brief Resets the FOC controller intermediate variables, outputs and PI controller
 *  previous values to 0.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 */
void Ifx_MDA_FocController_F16_reset(Ifx_MDA_FocController_F16* self);

/**
 *  \brief Get the module output variable, containing the voltage command.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [out] output Structure containing the module output
 */
static inline void Ifx_MDA_FocController_F16_getOutput(Ifx_MDA_FocController_F16       * self,
                                                       Ifx_MDA_FocController_F16_Output* output)
{
    *output = self->p_output;
}


/**
 *  \brief Set the output lower limit of the direct current PI
 *  controller
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] currentDPiLowerLimit Lower limit of the direct current PI controller, normalized, in Q15
 */
void Ifx_MDA_FocController_F16_setCurrentDPiLowerLimit(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                       currentDPiLowerLimit);

/**
 *  \brief Set the output upper limit of the direct current PI
 *  controller
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] currentDPiUpperLimit Upper limit of the direct current PI controller, normalized, in Q15
 */
void Ifx_MDA_FocController_F16_setCurrentDPiUpperLimit(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                       currentDPiUpperLimit);

/**
 *  \brief Set the output lower limit of the quadrature current PI
 *  controller
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] currentQPiLowerLimit Lower limit of the quadrature current PI controller, normalized, in Q15
 */
void Ifx_MDA_FocController_F16_setCurrentQPiLowerLimit(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                       currentQPiLowerLimit);

/**
 *  \brief Set the output upper limit of the quadrature current PI
 *  controller
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] currentQPiUpperLimit Upper limit of the quadrature current PI controller, normalized, in Q15
 */
void Ifx_MDA_FocController_F16_setCurrentQPiUpperLimit(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                       currentQPiUpperLimit);

/**
 *  \brief Get the saturation status of the voltage on the Q axis
 *
 *  Q voltage is in saturation if at least:
 *  <ul>
 *  <li>Q Pi controller is in saturation</li>
 *  <li>Q voltage has exceeded the maximum allowable voltage</li>
 *  </ul>
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Saturation status of the voltage on the Q axis
 */
Ifx_Math_Pi_F16_AntiWindupCtrl Ifx_MDA_FocController_F16_getQVoltageSatStatus(Ifx_MDA_FocController_F16* self);

/**
 *  \brief Set the proportional gain of the direct current PI controller
 *
 *  This function calculates the proportional gain in Q format and value from the
 *  input propGain and sets the proportional gain of the direct current PI
 *  controller.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] propGain Proportional gain
 */
void Ifx_MDA_FocController_F16_setCurrentDPiPropGain(Ifx_MDA_FocController_F16* self, uint32_t propGain);

/**
 *  \brief Get the direct current PI proportional gain
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Speed PI proportional gain in unsigned 32-bit format
 */
uint32_t Ifx_MDA_FocController_F16_getCurrentDPiPropGain(Ifx_MDA_FocController_F16* self);

/**
 *  \brief Set the product of integral gain and sample time of the direct current PI
 *  controller
 *
 *  This function calculates the product of integral gain and sample time in Q
 *  format and value from the input integGainSamplingTime and sets the gain of the
 *  direct current PI controller.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] integGainSamplingTime Product of integral gain and sample time
 */
void Ifx_MDA_FocController_F16_setCurrentDPiIntegGainSamplingTime(Ifx_MDA_FocController_F16* self, uint32_t
                                                                  integGainSamplingTime);

/**
 *  \brief Get the direct current PI product of integral gain and sampling time.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Direct current PI product of integral gain and sampling time in unsigned 32-bit
 * format
 */
uint32_t Ifx_MDA_FocController_F16_getCurrentDPiIntegGainSamplingTime(Ifx_MDA_FocController_F16* self);

/**
 *  \brief Set the proportional gain of the quadrature current PI controller
 *
 *  This function calculates the proportional gain in Q format and value from the
 *  input propGain and sets the proportional gain of the quadrature current PI
 *  controller.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] propGain Proportional gain
 */
void Ifx_MDA_FocController_F16_setCurrentQPiPropGain(Ifx_MDA_FocController_F16* self, uint32_t propGain);

/**
 *  \brief Get the quadrature current PI proportional gain
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Speed PI proportional gain in unsigned 32-bit format
 */
uint32_t Ifx_MDA_FocController_F16_getCurrentQPiPropGain(Ifx_MDA_FocController_F16* self);

/**
 *  \brief Set the product of integral gain and sample time of the quadrature current PI
 *  controller
 *
 *  This function calculates the product of integral gain and sample time in Q
 *  format and value from the input integGainSamplingTime and sets the gain of the
 *  quadrature current PI controller.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] integGainSamplingTime Product of integral gain and sample time
 */
void Ifx_MDA_FocController_F16_setCurrentQPiIntegGainSamplingTime(Ifx_MDA_FocController_F16* self, uint32_t
                                                                  integGainSamplingTime);

/**
 *  \brief Get the quadrature current PI product of integral gain and sampling time.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return Direct current PI product of integral gain and sampling time in unsigned 32-bit
 * format
 */
uint32_t Ifx_MDA_FocController_F16_getCurrentQPiIntegGainSamplingTime(Ifx_MDA_FocController_F16* self);

#endif /*IFX_MDA_FOCCONTROLLER_F16_H*/
