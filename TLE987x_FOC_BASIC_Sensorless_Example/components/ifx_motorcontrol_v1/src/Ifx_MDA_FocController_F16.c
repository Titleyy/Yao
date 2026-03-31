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

#include "Ifx_MDA_FocController_F16.h"
#include "Ifx_MDA_FocController_F16_Cfg.h"

/* C library */
#include "stddef.h"

/* Math library includes */
#include "Ifx_Math_CartToPolar.h"
#include "Ifx_Math_Interp1DLut.h"
#include "Ifx_Math_Park.h"
#include "Ifx_Math_PolarToCart.h"
#include "Ifx_Math_Arithmetic_F16.h"

/* CMSIS includes */
#include "cmsis_compiler.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/* Macros to define the component ID */
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_SOURCEID     ((uint8_t)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_LIBRARYID    ((uint16_t)Ifx_ComponentID_LibraryID_mctrlDriveAlgorithm)
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_MODULEID     (1U)
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_COMPONENTID1 (1U)
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_COMPONENTID2 ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_MAJOR   (2U)
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_MINOR   (0U)
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_PATCH   (0U)
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_T       (4U)
#define IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_REV     (0U)

/* Shift factor for getting index from Amplitude table */
#define IFX_MDA_FOCCONTROLLER_F16_SHIFT \
    (15 - IFX_MATH_MOTORCONTROLMATH_F16_CFG_CARTTOPOLAR_LUT_SIZE)

/* Modulation index max for linear region, normalized by 2/pi */
#define IFX_MDA_FOCCONTROLLER_F16_CONST_MI_0907_Q15        (18921)

/* Scaling factor used to up shift 16bits of Ifx_Math_Fract16Q(variable Qformat) value to store it in a uint32 variable
 * for transfer.
 * Todo: Move as fixed 16 bit shift into math function and remove as parameter? Is independent of TU decimal places.*/
#define IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU     65536U

/******************************************************************************/
/*-----------------------Private Function Prototypes--------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/**
 * \brief Uses the Math ConvToQForm function to convert the gains of the D current PI controller to Q format
 */
static inline void Ifx_MDA_FocController_F16_convDPiGainsToQ(Ifx_MDA_FocController_F16_Param* param,
                                                             Ifx_Math_Fract16Q* propGain,
                                                             Ifx_Math_Fract16Q* integGainSamplingTime,
                                                             Ifx_Math_Fract16Q* antiWindupGainSamplingTime);
/**
 * \brief Initializes the dynamic parameters of the D current PI
 */
static inline void Ifx_MDA_FocController_F16_initDPiDynParam(Ifx_MDA_FocController_F16* self,
                                                             Ifx_Math_Fract16Q propGain,
                                                             Ifx_Math_Fract16Q integGainSamplingTime,
                                                             Ifx_Math_Fract16Q antiWindupGainSamplingTime);
/**
 * \brief Uses the Math ConvToQForm function to convert the gains of the Q current PI controller to Q format
 */
static inline void Ifx_MDA_FocController_F16_convQPiGainsToQ(Ifx_MDA_FocController_F16_Param* param,
                                                             Ifx_Math_Fract16Q* propGain, 
                                                             Ifx_Math_Fract16Q* integGainSamplingTime,
                                                             Ifx_Math_Fract16Q* antiWindupGainSamplingTime);
/**
 * \brief Initializes the dynamic parameters of the Q current PI
 */
static inline void Ifx_MDA_FocController_F16_initQPiDynParam(Ifx_MDA_FocController_F16* self,
                                                             Ifx_Math_Fract16Q propGain,
                                                             Ifx_Math_Fract16Q integGainSamplingTime,
                                                             Ifx_Math_Fract16Q antiWindupGainSamplingTime);
/* *INDENT-ON* */

/**
 * \brief Calculate and return the difference between the dq current command and the actual dq currents
 */
static inline Ifx_Math_CmpFract16 Ifx_MDA_FocController_F16_calcCurrentErrorsQ14(Ifx_Math_CmpFract16 currentDQ,
                                                                                 Ifx_Math_CmpFract16 dqCommand);

#if IFX_MDA_FOCCONTROLLER_F16_CFG_DQDECOUPLINGENABLE

/* Local function to execute d-q decoupling */
static inline void Ifx_MDA_FocController_F16_dqDecoupling(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                          electricalSpeed);
#endif /* IFX_MDA_FOCCONTROLLER_F16_CFG_DQDECOUPLINGENABLE */
/* Local functions to initialize d and q PI controllers */
static inline void Ifx_MDA_FocController_F16_initDPi(Ifx_MDA_FocController_F16* self);
static inline void Ifx_MDA_FocController_F16_initQPi(Ifx_MDA_FocController_F16* self);

#if IFX_MDA_FOCCONTROLLER_F16_CFG_LIMIT_VOLT_VECTOR_D_PRIO

/* Local functions to limit D and Q voltages */
static inline void Ifx_MDA_FocController_F16_limitDQVoltage(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                            dcLinkVoltageQ15);
static inline void Ifx_MDA_FocController_F16_limitDVoltage(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                           maxAmpQ15);
static inline void Ifx_MDA_FocController_F16_limitQVoltage(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                           maxAmpQ15);
#endif /* IFX_MDA_FOCCONTROLLER_F16_CFG_LIMIT_VOLT_VECTOR_D_PRIO */
/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Component ID */
static const Ifx_ComponentID      Ifx_MDA_FocController_F16_componentID = {
    .sourceID     = IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_SOURCEID,
    .libraryID    = IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_LIBRARYID,
    .moduleID     = IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_MODULEID,
    .componentID1 = IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_MDA_FOCCONTROLLER_F16_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MDA_FocController_F16_componentVersion = {
    .major = IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_MAJOR,
    .minor = IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_MINOR,
    .patch = IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_PATCH,
    .t     = IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_T,
    .rev   = IFX_MDA_FOCCONTROLLER_F16_COMPONENTVERSION_REV
};

#if IFX_MDA_FOCCONTROLLER_F16_CFG_LIMIT_VOLT_VECTOR_D_PRIO

/* Link to lookup table */
static const Ifx_Math_Interp1DLut_Type focControllerInterp1DLut = {
    .data = &Ifx_Math_Lut_CartToPolar_F16_table[0],
    .size = (((uint16_t)1 << IFX_MATH_MOTORCONTROLMATH_F16_CFG_CARTTOPOLAR_LUT_SIZE) + 1)
};

#endif /* IFX_MDA_FOCCONTROLLER_F16_CFG_LIMIT_VOLT_VECTOR_D_PRIO */

/******************************************************************************/
/*-----------------------Exported Variables/Constants-------------------------*/
/******************************************************************************/
/* temporary solution, XML_VERSION indicates if static cfg is generated by cfgwiz */
#ifdef IFX_MDA_FOCCONTROLLER_F16_CFG_XML_VERSION
/* Default parameters initialized with CfgWizard defines */
Ifx_MDA_FocController_F16_Param Ifx_MDA_FocController_F16_g_defaultParam =
{
    .currentDPiPropGain                   = IFX_MDA_FOCCONTROLLER_F16_CFG_ID_PI_PROPGAIN_TU,
    .currentDPiIntegGainSamplingTime      = IFX_MDA_FOCCONTROLLER_F16_CFG_ID_PI_KI_TS_TU,
    .currentDPiAntiWindupGainSamplingTime = IFX_MDA_FOCCONTROLLER_F16_CFG_ID_PI_KAW_TS_TU,
    .currentDPiUpperLimitQ15              = IFX_MDA_FOCCONTROLLER_F16_CFG_ID_PI_OUT_UPP_LIMIT_Q,
    .currentDPiLowerLimitQ15              = IFX_MDA_FOCCONTROLLER_F16_CFG_ID_PI_OUT_LOW_LIMIT_Q,
    .currentQPiPropGain                   = IFX_MDA_FOCCONTROLLER_F16_CFG_IQ_PI_PROPGAIN_TU,
    .currentQPiIntegGainSamplingTime      = IFX_MDA_FOCCONTROLLER_F16_CFG_IQ_PI_KI_TS_TU,
    .currentQPiAntiWindupGainSamplingTime = IFX_MDA_FOCCONTROLLER_F16_CFG_IQ_PI_KAW_TS_TU,
    .currentQPiUpperLimitQ15              = IFX_MDA_FOCCONTROLLER_F16_CFG_IQ_PI_OUT_UPP_LIMIT_Q,
    .currentQPiLowerLimitQ15              = IFX_MDA_FOCCONTROLLER_F16_CFG_IQ_PI_OUT_LOW_LIMIT_Q,
};
#endif
/* *INDENT-ON* */
/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
/* Function to get the component ID */
void Ifx_MDA_FocController_F16_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MDA_FocController_F16_componentID;
}


/* Function to get the component version */
void Ifx_MDA_FocController_F16_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MDA_FocController_F16_componentVersion;
}


void Ifx_MDA_FocController_F16_init(Ifx_MDA_FocController_F16* self, Ifx_MDA_FocController_F16_Param* param)
{
    /* Initialize pointer to parameter structure */
    self->param = param;

    /* Initialize internal variables and outputs to 0 */
    self->currentDQ.real                         = 0;
    self->currentDQ.imag                         = 0;
    self->voltageDQ.real                         = 0;
    self->voltageDQ.imag                         = 0;
    self->p_output.voltageCommandPolar.amplitude = 0;
    self->p_output.voltageCommandPolar.angle     = 0;

    /* Initialize Id PI controller */
    Ifx_MDA_FocController_F16_initDPi(self);

    /* Initialize Iq PI controller */
    Ifx_MDA_FocController_F16_initQPi(self);

    /* Initialize d-q decoupling */
#if IFX_MDA_FOCCONTROLLER_F16_CFG_DQDECOUPLINGENABLE
    Ifx_Math_DqDecoupling_F16_init(&(self->dqDecoupling));
    Ifx_Math_DqDecoupling_F16_setInductanceD(&(self->dqDecoupling),
        IFX_MDA_FOCCONTROLLER_F16_CFG_DIRECT_INDUCTANCE_Q15);
    Ifx_Math_DqDecoupling_F16_setInductanceQ(&(self->dqDecoupling),
        IFX_MDA_FOCCONTROLLER_F16_CFG_QUADRATURE_INDUCTANCE_Q15);
#endif /* IFX_MDA_FOCCONTROLLER_F16_CFG_DQDECOUPLINGENABLE */
}


static inline Ifx_Math_CmpFract16 Ifx_MDA_FocController_F16_calcCurrentErrorsQ14(Ifx_Math_CmpFract16 currentDQ,
                                                                                 Ifx_Math_CmpFract16 dqCommand)
{
    Ifx_Math_CmpFract16 errorCurrentDQ;
    errorCurrentDQ.real = Ifx_Math_Sub_F16(Ifx_Math_ShR_F16(dqCommand.real, 1U), Ifx_Math_ShR_F16(currentDQ.real,
        1U));
    errorCurrentDQ.imag = Ifx_Math_Sub_F16(Ifx_Math_ShR_F16(dqCommand.imag, 1U), Ifx_Math_ShR_F16(currentDQ.imag,
        1U));

    return errorCurrentDQ;
}


/* polyspace +2 CODE-METRIC:PARAM [Justified:Medium] "Increased complexity is justified to keep the functionality of
 * the component the same." */
__USED void Ifx_MDA_FocController_F16_execute(Ifx_MDA_FocController_F16* self, Ifx_Math_CmpFract16 currentAlphaBeta,
                                              Ifx_Math_CmpFract16 dqCommand, uint32_t rotorFluxAngle, Ifx_Math_Fract16
                                              electricalSpeed, Ifx_Math_Fract16
                                              dcLinkVoltageQ15)
{
    /* Current Park trans. (alpha-beta to d-q) */
    self->currentDQ = Ifx_Math_Park_F16(currentAlphaBeta, rotorFluxAngle);

    /* Calculate the current errors */
    Ifx_Math_CmpFract16 currentErrorsQ14 = Ifx_MDA_FocController_F16_calcCurrentErrorsQ14(self->currentDQ, dqCommand);

    /* Execute current PI controllers */
    self->voltageDQ.real = Ifx_Math_Pi_F16_execute(&(self->currentDPi), currentErrorsQ14.real,
        self->p_dAntiwindupCtrl);
    self->voltageDQ.imag = Ifx_Math_Pi_F16_execute(&(self->currentQPi), currentErrorsQ14.imag,
        self->p_qAntiwindupCtrl);

#if IFX_MDA_FOCCONTROLLER_F16_CFG_DQDECOUPLINGENABLE

    /* Execute d-q decoupling */
    Ifx_MDA_FocController_F16_dqDecoupling(self, electricalSpeed);
#else

    /* Suppress compiler warning */
    (void)electricalSpeed;
#endif /* IFX_MDA_FOCCONTROLLER_F16_CFG_DQDECOUPLINGENABLE */

#if IFX_MDA_FOCCONTROLLER_F16_CFG_LIMIT_VOLT_VECTOR_D_PRIO

    /* Limit calculated D and Q voltages according to the available DC link voltage with a D component prioritization */
    Ifx_MDA_FocController_F16_limitDQVoltage(self, dcLinkVoltageQ15);
#endif /* IFX_MDA_FOCCONTROLLER_F16_CFG_LIMIT_VOLT_VECTOR_D_PRIO */
    /* Voltage cartesian to polar (d-q to angle-amp) */
    self->p_output.voltageCommandPolar = Ifx_Math_CartToPolar_F16(self->voltageDQ);

    /* Add rotor flux angle to convert from d-q to alpha-beta */
    self->p_output.voltageCommandPolar.angle += rotorFluxAngle;
}


void Ifx_MDA_FocController_F16_reset(Ifx_MDA_FocController_F16* self)
{
    /* Reset intermediate variables and outputs */
    self->currentDQ.real                         = 0;
    self->currentDQ.imag                         = 0;
    self->voltageDQ.real                         = 0;
    self->voltageDQ.imag                         = 0;
    self->p_output.voltageCommandPolar.amplitude = 0;
    self->p_output.voltageCommandPolar.angle     = 0;

    /* Reset PI controllers previous values */
    Ifx_Math_Pi_F16_setIntegPreviousValue(&(self->currentDPi), 0);
    Ifx_Math_Pi_F16_setIntegPreviousValue(&(self->currentQPi), 0);
}


static inline void Ifx_MDA_FocController_F16_convDPiGainsToQ(Ifx_MDA_FocController_F16_Param* param,
                                                             Ifx_Math_Fract16Q              * propGain,
                                                             Ifx_Math_Fract16Q              * integGainSamplingTime,
                                                             Ifx_Math_Fract16Q              *
                                                             antiWindupGainSamplingTime)
{
    *propGain                   = Ifx_Math_ConvToQForm_F16(param->currentDPiPropGain,
        IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);
    *integGainSamplingTime      = Ifx_Math_ConvToQForm_F16(param->currentDPiIntegGainSamplingTime,
        IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);
    *antiWindupGainSamplingTime = Ifx_Math_ConvToQForm_F16(param->currentDPiAntiWindupGainSamplingTime,
        IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);
}


static inline void Ifx_MDA_FocController_F16_initDPiDynParam(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16Q
                                                             propGain, Ifx_Math_Fract16Q integGainSamplingTime,
                                                             Ifx_Math_Fract16Q antiWindupGainSamplingTime)
{
    /* Set the dynamic parameters of the D current PI */
    Ifx_Math_Pi_F16_setPropGain(&(self->currentDPi), propGain);
    Ifx_Math_Pi_F16_setIntegGainSamplingTime(&(self->currentDPi), integGainSamplingTime);
    Ifx_Math_Pi_F16_setAntiWindupGainSamplingTime(&(self->currentDPi), antiWindupGainSamplingTime);
    Ifx_Math_Pi_F16_setUpperLimit(&(self->currentDPi), self->param->currentDPiUpperLimitQ15);
    Ifx_Math_Pi_F16_setLowerLimit(&(self->currentDPi), self->param->currentDPiLowerLimitQ15);
}


static inline void Ifx_MDA_FocController_F16_initDPi(Ifx_MDA_FocController_F16* self)
{
    /* Convert the gains to Q format */
    Ifx_Math_Fract16Q propGain;
    Ifx_Math_Fract16Q integGainSamplingTime;
    Ifx_Math_Fract16Q antiWindupGainSamplingTime;
    Ifx_MDA_FocController_F16_convDPiGainsToQ(self->param, &propGain, &integGainSamplingTime,
        &antiWindupGainSamplingTime);

    /* Initialize the q formats for the D current PI controller */
    Ifx_Math_Pi_F16_Qformats piQForm = {
        .qFormatError = Ifx_Math_FractQFormat_q14, .qFormatOutput = Ifx_Math_FractQFormat_q15
    };

    /* Initialize the q formats for the D current PI controller */
    Ifx_Math_Pi_F16_init(&(self->currentDPi), piQForm);

    /* Initialize the dynamic parameters of the D current PI controller */
    Ifx_MDA_FocController_F16_initDPiDynParam(self, propGain, integGainSamplingTime, antiWindupGainSamplingTime);

    /* Set antiwindup flag to inactive */
    self->p_dAntiwindupCtrl = Ifx_Math_Pi_F16_AntiWindupCtrl_no;
}


static inline void Ifx_MDA_FocController_F16_convQPiGainsToQ(Ifx_MDA_FocController_F16_Param* param,
                                                             Ifx_Math_Fract16Q              * propGain,
                                                             Ifx_Math_Fract16Q              * integGainSamplingTime,
                                                             Ifx_Math_Fract16Q              *
                                                             antiWindupGainSamplingTime)
{
    *propGain                   = Ifx_Math_ConvToQForm_F16(param->currentQPiPropGain,
        IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);
    *integGainSamplingTime      = Ifx_Math_ConvToQForm_F16(param->currentQPiIntegGainSamplingTime,
        IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);
    *antiWindupGainSamplingTime = Ifx_Math_ConvToQForm_F16(param->currentQPiAntiWindupGainSamplingTime,
        IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);
}


static inline void Ifx_MDA_FocController_F16_initQPiDynParam(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16Q
                                                             propGain, Ifx_Math_Fract16Q integGainSamplingTime,
                                                             Ifx_Math_Fract16Q antiWindupGainSamplingTime)
{
    /* Set the dynamic parameters of the current PI */
    Ifx_Math_Pi_F16_setPropGain(&(self->currentQPi), propGain);
    Ifx_Math_Pi_F16_setIntegGainSamplingTime(&(self->currentQPi), integGainSamplingTime);
    Ifx_Math_Pi_F16_setAntiWindupGainSamplingTime(&(self->currentQPi), antiWindupGainSamplingTime);
    Ifx_Math_Pi_F16_setUpperLimit(&(self->currentQPi), self->param->currentQPiUpperLimitQ15);
    Ifx_Math_Pi_F16_setLowerLimit(&(self->currentQPi), self->param->currentQPiLowerLimitQ15);
}


static inline void Ifx_MDA_FocController_F16_initQPi(Ifx_MDA_FocController_F16* self)
{
    /* Convert the gains to Q format */
    Ifx_Math_Fract16Q propGain;
    Ifx_Math_Fract16Q integGainSamplingTime;
    Ifx_Math_Fract16Q antiWindupGainSamplingTime;
    Ifx_MDA_FocController_F16_convQPiGainsToQ(self->param, &propGain, &integGainSamplingTime,
        &antiWindupGainSamplingTime);

    /* Initialize the q formats for the Q current PI controller */
    Ifx_Math_Pi_F16_Qformats piQForm = {
        .qFormatError = Ifx_Math_FractQFormat_q14, .qFormatOutput = Ifx_Math_FractQFormat_q15
    };

    /* Initialize the q formats for the Q current PI controller */
    Ifx_Math_Pi_F16_init(&(self->currentQPi), piQForm);

    /* Initialize the dynamic parameters of the Q current PI controller */
    Ifx_MDA_FocController_F16_initQPiDynParam(self, propGain, integGainSamplingTime, antiWindupGainSamplingTime);

    /* Set antiwindup flag to inactive */
    self->p_qAntiwindupCtrl = Ifx_Math_Pi_F16_AntiWindupCtrl_no;
}


#if IFX_MDA_FOCCONTROLLER_F16_CFG_DQDECOUPLINGENABLE

/* Functions called by Ifx_MDA_FocController_F16_execute() */
static inline void Ifx_MDA_FocController_F16_dqDecoupling(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                          electricalSpeed)
{
    /* Calculate and apply d-q decoupling */
    Ifx_Math_CmpFract16 compensationVoltageDQ = Ifx_Math_DqDecoupling_F16_execute(&(self->dqDecoupling),
        self->currentDQ, electricalSpeed);
    self->voltageDQ.real = Ifx_Math_SubSat_F16(self->voltageDQ.real, compensationVoltageDQ.real);
    self->voltageDQ.imag = Ifx_Math_AddSat_F16(self->voltageDQ.imag, compensationVoltageDQ.imag);
}


#endif /* IFX_MDA_FOCCONTROLLER_F16_CFG_DQDECOUPLINGENABLE */

#if IFX_MDA_FOCCONTROLLER_F16_CFG_LIMIT_VOLT_VECTOR_D_PRIO
static inline void Ifx_MDA_FocController_F16_limitDQVoltage(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                            dcLinkVoltageQ15)
{
    /* Calculate maximum modulation amplitude based on actual DC-Link voltage  */
    Ifx_Math_Fract16 maxAmpQ15 = Ifx_Math_MulSat_F16(dcLinkVoltageQ15, IFX_MDA_FOCCONTROLLER_F16_CONST_MI_0907_Q15);

    /* Limit D voltage to max. amplitude */
    Ifx_MDA_FocController_F16_limitDVoltage(self, maxAmpQ15);

    /* Limit Q voltage to the remaining available voltage vector */
    Ifx_MDA_FocController_F16_limitQVoltage(self, maxAmpQ15);
}


static inline void Ifx_MDA_FocController_F16_limitDVoltage(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                           maxAmpQ15)
{
    if (Ifx_Math_Abs_F16(self->voltageDQ.real) >= maxAmpQ15)
    {
        /* Check D voltage sign */
        if (self->voltageDQ.real >= 0)
        {
            /* Set D antiwindup flag to positive saturation */
            self->p_dAntiwindupCtrl = Ifx_Math_Pi_F16_AntiWindupCtrl_pos;

            /* Limit positive D */
            self->voltageDQ.real = maxAmpQ15;
        }
        else
        {
            /* Set D antiwindup flag to negative saturation */
            self->p_dAntiwindupCtrl = Ifx_Math_Pi_F16_AntiWindupCtrl_neg;

            /* Limit negative D */
            self->voltageDQ.real = Ifx_Math_Neg_F16(maxAmpQ15);
        }
    }
    else
    {
        /* Set D antiwindup flag to no saturation */
        self->p_dAntiwindupCtrl = Ifx_Math_Pi_F16_AntiWindupCtrl_no;
    }
}


static inline void Ifx_MDA_FocController_F16_limitQVoltage(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                           maxAmpQ15)
{
    /* Calculate square of maximum modulation amplitude */
    Ifx_Math_Fract16 maxAmpSqrQ15 = Ifx_Math_MulShR_F16(maxAmpQ15, maxAmpQ15, 15u);

    /* ud^2 = ud*ud */
    Ifx_Math_Fract16 realVoltSqr = Ifx_Math_MulShR_F16(self->voltageDQ.real, self->voltageDQ.real, 15u);

    /* uq^2 = ulim^2 -ud^2 */
    Ifx_Math_Fract16 imagVoltSqr = Ifx_Math_Sub_F16(maxAmpSqrQ15, realVoltSqr);

    /* uq = sqrt(uq^2) */
    Ifx_Math_Fract16 imagVoltAbs = Ifx_Math_Interp1DLut_F16(imagVoltSqr, focControllerInterp1DLut,
        IFX_MDA_FOCCONTROLLER_F16_SHIFT);

    /* Limit Q voltage to sqrt(Umax^2-ud^2) */
    if (Ifx_Math_Abs_F16(self->voltageDQ.imag) >= imagVoltAbs)
    {
        /* Check Q voltage sign */
        if (self->voltageDQ.imag >= 0)
        {
            /* Set Q antiwindup flag to positive saturation */
            self->p_qAntiwindupCtrl = Ifx_Math_Pi_F16_AntiWindupCtrl_pos;

            /* Limit positive Q */
            self->voltageDQ.imag = imagVoltAbs;
        }
        else
        {
            /* Set Q antiwindup flag to negative saturation */
            self->p_qAntiwindupCtrl = Ifx_Math_Pi_F16_AntiWindupCtrl_neg;

            /* Limit negative Q */
            self->voltageDQ.imag = Ifx_Math_Neg_F16(imagVoltAbs);
        }
    }
    else
    {
        /* Set Q antiwindup flag to no saturation */
        self->p_qAntiwindupCtrl = Ifx_Math_Pi_F16_AntiWindupCtrl_no;
    }
}


#endif /* IFX_MDA_FOCCONTROLLER_F16_CFG_LIMIT_VOLT_VECTOR_D_PRIO */
void Ifx_MDA_FocController_F16_setCurrentDPiLowerLimit(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                       currentDPiLowerLimit)
{
    /* Set internal parameter */
    self->param->currentDPiLowerLimitQ15 = currentDPiLowerLimit;

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setLowerLimit(&(self->currentDPi), currentDPiLowerLimit);
}


void Ifx_MDA_FocController_F16_setCurrentDPiUpperLimit(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                       currentDPiUpperLimit)
{
    /* Set internal parameter */
    self->param->currentDPiUpperLimitQ15 = currentDPiUpperLimit;

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setUpperLimit(&(self->currentDPi), currentDPiUpperLimit);
}


void Ifx_MDA_FocController_F16_setCurrentQPiLowerLimit(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                       currentQPiLowerLimit)
{
    /* Set internal parameter */
    self->param->currentQPiLowerLimitQ15 = currentQPiLowerLimit;

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setLowerLimit(&(self->currentQPi), currentQPiLowerLimit);
}


void Ifx_MDA_FocController_F16_setCurrentQPiUpperLimit(Ifx_MDA_FocController_F16* self, Ifx_Math_Fract16
                                                       currentQPiUpperLimit)
{
    /* Set internal parameter */
    self->param->currentQPiUpperLimitQ15 = currentQPiUpperLimit;

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setUpperLimit(&(self->currentQPi), currentQPiUpperLimit);
}


uint32_t Ifx_MDA_FocController_F16_getCurrentDPiPropGain(Ifx_MDA_FocController_F16* self)
{
    return self->param->currentDPiPropGain;
}


void Ifx_MDA_FocController_F16_setCurrentDPiPropGain(Ifx_MDA_FocController_F16* self, uint32_t propGain)
{
    /* Set internal parameter */
    self->param->currentDPiPropGain = propGain;

    /* Get the gain in Q format */
    Ifx_Math_Fract16Q propGainQ = Ifx_Math_ConvToQForm_F16(propGain, IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setPropGain(&(self->currentDPi), propGainQ);
}


uint32_t Ifx_MDA_FocController_F16_getCurrentDPiIntegGainSamplingTime(Ifx_MDA_FocController_F16* self)
{
    return self->param->currentDPiIntegGainSamplingTime;
}


void Ifx_MDA_FocController_F16_setCurrentDPiIntegGainSamplingTime(Ifx_MDA_FocController_F16* self, uint32_t
                                                                  integGainSamplingTime)
{
    /* Set internal parameter */
    self->param->currentDPiIntegGainSamplingTime = integGainSamplingTime;

    /* Get the gain in Q format */
    Ifx_Math_Fract16Q integGainSamplingTimeQ = Ifx_Math_ConvToQForm_F16(integGainSamplingTime,
        IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setIntegGainSamplingTime(&(self->currentDPi), integGainSamplingTimeQ);
}


uint32_t Ifx_MDA_FocController_F16_getCurrentQPiPropGain(Ifx_MDA_FocController_F16* self)
{
    return self->param->currentQPiPropGain;
}


void Ifx_MDA_FocController_F16_setCurrentQPiPropGain(Ifx_MDA_FocController_F16* self, uint32_t propGain)
{
    /* Set internal parameter */
    self->param->currentQPiPropGain = propGain;

    /* Get the gain in Q format */
    Ifx_Math_Fract16Q propGainQ = Ifx_Math_ConvToQForm_F16(propGain, IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setPropGain(&(self->currentQPi), propGainQ);
}


uint32_t Ifx_MDA_FocController_F16_getCurrentQPiIntegGainSamplingTime(Ifx_MDA_FocController_F16* self)
{
    return self->param->currentQPiIntegGainSamplingTime;
}


void Ifx_MDA_FocController_F16_setCurrentQPiIntegGainSamplingTime(Ifx_MDA_FocController_F16* self, uint32_t
                                                                  integGainSamplingTime)
{
    /* Set internal parameter */
    self->param->currentQPiIntegGainSamplingTime = integGainSamplingTime;

    /* Get the gain in Q format */
    Ifx_Math_Fract16Q integGainSamplingTimeQ = Ifx_Math_ConvToQForm_F16(integGainSamplingTime,
        IFX_MDA_FOCCONTROLLER_F16_PI_SCALING_FACTOR_TU);

    /* Call PI controller setter */
    Ifx_Math_Pi_F16_setIntegGainSamplingTime(&(self->currentQPi), integGainSamplingTimeQ);
}


Ifx_Math_Pi_F16_AntiWindupCtrl Ifx_MDA_FocController_F16_getQVoltageSatStatus(Ifx_MDA_FocController_F16* self)
{
    Ifx_Math_Pi_F16_AntiWindupCtrl qVoltageSatStatus;

    /* Get the saturation status of the Q PI controller */
    Ifx_Math_Pi_F16_AntiWindupCtrl qVoltageSatStatusIntern = Ifx_Math_Pi_F16_getSaturationStatus(&(self->currentQPi));

    /* Check internal and external saturation status */
    if ((qVoltageSatStatusIntern == Ifx_Math_Pi_F16_AntiWindupCtrl_no)
        && (self->p_qAntiwindupCtrl == Ifx_Math_Pi_F16_AntiWindupCtrl_no))
    {
        qVoltageSatStatus = Ifx_Math_Pi_F16_AntiWindupCtrl_no;
    }
    else if ((qVoltageSatStatusIntern == Ifx_Math_Pi_F16_AntiWindupCtrl_neg)
             || (self->p_qAntiwindupCtrl == Ifx_Math_Pi_F16_AntiWindupCtrl_neg))
    {
        qVoltageSatStatus = Ifx_Math_Pi_F16_AntiWindupCtrl_neg;
    }
    else
    {
        qVoltageSatStatus = Ifx_Math_Pi_F16_AntiWindupCtrl_pos;
    }

    return qVoltageSatStatus;
}
