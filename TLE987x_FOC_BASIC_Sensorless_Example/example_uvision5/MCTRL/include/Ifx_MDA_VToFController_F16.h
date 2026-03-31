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
 * \file Ifx_MDA_VToFController_F16.h
 * \brief The V/f (also called voltage to frequency) scalar control module.
 */

#ifndef IFX_MDA_VTOFCONTROLLER_F16_H
#define IFX_MDA_VTOFCONTROLLER_F16_H
#include "Ifx_ComponentID.h"
#include "Ifx_ComponentVersion.h"
#include "Ifx_Math.h"

/**
 * Contains the module output variables.
 */
typedef struct Ifx_MDA_VToFController_F16_Output
{
    /**
     * Angle and normalized amplitude of the output voltage vector, in Q15
     */
    Ifx_Math_PolarFract16 voltageVector;
} Ifx_MDA_VToFController_F16_Output;

/**
 * Structure to hold the normalized parameters of the V/f LUT, in Q15
 */
typedef struct Ifx_MDA_VToFController_F16_VToFLutNorm
{
    /**
     * Normalized rated speed
     */
    Ifx_Math_Fract16 ratedSpeedQ15;

    /**
     * Normalized corner speed
     */
    Ifx_Math_Fract16 cornerSpeedQ15;

    /**
     * Normalized minimum voltage
     */
    Ifx_Math_Fract16 minVoltQ15;

    /**
     * Normalized corner voltage
     */
    Ifx_Math_Fract16 cornerVoltQ15;

    /**
     * Normalized rated voltage
     */
    Ifx_Math_Fract16 ratedVoltQ15;
} Ifx_MDA_VToFController_F16_VToFLutNorm;

/**
 * Parameter structure containing configuration of the component
 */
typedef struct Ifx_MDA_VToFController_F16_Param
{
    /**
     * VToF lookup table object containing the normalized points of the LUT
     */
    Ifx_MDA_VToFController_F16_VToFLutNorm VToFLUTnorm;

    /**
     * Angle increment value, in Q14
     */
    Ifx_Math_Fract16 angleIncrementQ14;
} Ifx_MDA_VToFController_F16_Param;

/**
 * \brief Data structure that stores all data of module instance.
 */
typedef struct Ifx_MDA_VToFController_F16
{
    /**
     * Parameter structure
     */
    Ifx_MDA_VToFController_F16_Param* param;

    /**
     * Contains the module output variables
     */
    Ifx_MDA_VToFController_F16_Output p_output;

    /**
     * Previous value (state) for the angle integrator
     */
    uint32_t p_anglePreviousValue;
} Ifx_MDA_VToFController_F16;

/**
 * Global variable which holds default parameter values
 */
extern Ifx_MDA_VToFController_F16_Param Ifx_MDA_VToFController_F16_g_defaultParam;

/**
 *  \brief The initialization API of the V2f module.
 *
 *  The normalized parameters are initialized according to the data entered in
 *  Config Wizard.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [inout] param Pointer to parameter structure containing configuration of the component
 */
void Ifx_MDA_VToFController_F16_init(Ifx_MDA_VToFController_F16* self, Ifx_MDA_VToFController_F16_Param* param);

/**
 *  \brief The cyclic execution API of the VToF module.
 *
 *  This API calculates the angle by integrating the speed with respect to time, and
 *  calculates the command voltage output based on the input reference speed by
 *  linear interpolation, using the defined lookup table VToFLUT.
 *
 *  Inputs of this API
 *  <ul>
 *  <li>Reference speed, normalized by BASE_SPEED_RADPS in Q15 format</li>
 *  </ul>
 *
 *  Outputs of this API
 *  <ul>
 *  <li>Calculated voltage amplitude, normalized by CFG_BASE_VOLTAGE_V in Q15
 *  format</li>
 *  <li>Calculated angle between 0 and 2*pi, normalized to 0 to 2^32-1</li>
 *  </ul>
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] speedQ15 Input speed, normalized
 */
void Ifx_MDA_VToFController_F16_execute(Ifx_MDA_VToFController_F16* self, Ifx_Math_Fract16 speedQ15);

/**
 *  \brief Set the previous value of the angle.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [in] anglePreviousValue The value to which the previous value of the angle must be set
 */
static inline void Ifx_MDA_VToFController_F16_setAnglePreviousValue(Ifx_MDA_VToFController_F16* self, uint32_t
                                                                    anglePreviousValue)
{
    self->p_anglePreviousValue = anglePreviousValue;
}


/**
 *  \brief Get the angle previous value.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \return The angle previous value
 */
static inline uint32_t Ifx_MDA_VToFController_F16_getAnglePreviousValue(Ifx_MDA_VToFController_F16* self)
{
    return self->p_anglePreviousValue;
}


/**
 *  \brief Get the module output variables, containing the voltage vector.
 *
 *  \param [inout] self Reference to structure that contains instance data members
 *
 *  \param [out] output Structure containing the module output
 */
static inline void Ifx_MDA_VToFController_F16_getOutput(Ifx_MDA_VToFController_F16       * self,
                                                        Ifx_MDA_VToFController_F16_Output* output)
{
    *output = self->p_output;
}


/**
 *  \brief Returns the component ID
 *
 *  \param [out] *componentID Variable to store the address of the component ID
 */
void Ifx_MDA_VToFController_F16_getID(const Ifx_ComponentID** componentID);

/**
 *  \brief Returns the component version
 *
 *  \param [out] *componentVersion Variable to store the address of the component version
 */
void Ifx_MDA_VToFController_F16_getVersion(const Ifx_ComponentVersion** componentVersion);

#endif /*IFX_MDA_VTOFCONTROLLER_F16_H*/
