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
#include "Ifx_MHA_MeasurementADC_F16_TLE987.h"
#include "Ifx_MHA_MeasurementADC_F16_TLE987_Cfg.h"

/* Math library */
#include "Ifx_Math_Sat.h"
#include "Ifx_Math_ShR.h"

/* CMSIS */
#include "cmsis_compiler.h"

/* SDK */
#include "adc1.h"

/******************************************************************************/
/*-----------------------------------Macros-----------------------------------*/
/******************************************************************************/
/* Macros to define the component ID */
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_SOURCEID \
    ((uint8_t)Ifx_ComponentID_SourceID_infineonTechnologiesAG)
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_LIBRARYID \
    ((uint16_t)Ifx_ComponentID_LibraryID_mctrlHardwareAbstraction)
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_MODULEID     (1U)
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_COMPONENTID1 (1U)
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_COMPONENTID2 ((uint8_t)Ifx_ComponentID_ComponentID2_basic)

/* Macros to define the component version */
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_MAJOR   (2U)
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_MINOR   (0U)
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_PATCH   (0U)
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_T       (4U)
#define IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_REV     (0U)

/******************************************************************************/
/*-----------------------Static Function Prototypes---------------------------*/
/******************************************************************************/
/* Swap write index of the current measurement buffer after receiving second current measurement */
static inline void Ifx_MHA_MeasurementADC_F16_TLE987_swapCurrentMeasurementWriteIndex(
    Ifx_MHA_MeasurementADC_F16_TLE987* self);

/* DC link voltage and shunt currents calculation*/
static inline void Ifx_MHA_MeasurementADC_F16_TLE987_calc(Ifx_MHA_MeasurementADC_F16_TLE987* self);

/* Scale DC-Link currents to Q15 */
static inline Ifx_Math_Fract16 Ifx_MHA_MeasurementADC_F16_TLE987_scaleCurrent(Ifx_MHA_MeasurementADC_F16_TLE987* self,
                                                                              uint16_t
                                                                              rawCurrentMeasurement);

/* Execute function in state on */
static inline Ifx_MHA_MeasurementADC_F16_TLE987_State Ifx_MHA_MeasurementADC_F16_TLE987_stateOn(
    Ifx_MHA_MeasurementADC_F16_TLE987* self);

/**
 * \brief Configure the input range of the VDH Monitoring Input Attenuator
 *
 * \param attenuatorRange Configured attenuator range for VDH
 */
static inline void Ifx_MHA_MeasurementADC_F16_TLE987_setVDHRange(uint32_t attenuatorRange);

/******************************************************************************/
/*------------------------Private Variables/Constants-------------------------*/
/******************************************************************************/
/* *INDENT-OFF* */
/* Component ID */
static const Ifx_ComponentID      Ifx_MHA_MeasurementADC_F16_TLE987_componentID = {
    .sourceID     = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_SOURCEID,
    .libraryID    = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_LIBRARYID,
    .moduleID     = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_MODULEID,
    .componentID1 = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_COMPONENTID1,
    .componentID2 = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTID_COMPONENTID2,
};

/* Component Version */
static const Ifx_ComponentVersion Ifx_MHA_MeasurementADC_F16_TLE987_componentVersion = {
    .major = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_MAJOR,
    .minor = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_MINOR,
    .patch = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_PATCH,
    .t     = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_T,
    .rev   = IFX_MHA_MEASUREMENTADC_F16_TLE987_COMPONENTVERSION_REV
};
/* *INDENT-ON* */
/******************************************************************************/
/*-------------------------Function Implementations---------------------------*/
/******************************************************************************/
/* Function to get the component ID */
void Ifx_MHA_MeasurementADC_F16_TLE987_getID(const Ifx_ComponentID** componentID)
{
    *componentID = &Ifx_MHA_MeasurementADC_F16_TLE987_componentID;
}


/* Function to get the component version */
void Ifx_MHA_MeasurementADC_F16_TLE987_getVersion(const Ifx_ComponentVersion** componentVersion)
{
    *componentVersion = &Ifx_MHA_MeasurementADC_F16_TLE987_componentVersion;
}


void Ifx_MHA_MeasurementADC_F16_TLE987_init(Ifx_MHA_MeasurementADC_F16_TLE987* self)
{
    /* Reset variables used to store current measurements */
    self->p_rawCurrentMeasurements[0][0]     = self->p_rawCurrentMeasurements[0][1] =
        self->p_rawCurrentMeasurements[1][0] = self->p_rawCurrentMeasurements[1][1] = 0U;
    self->p_rawCurrentMeasurementsReadIndex  = 1;
    self->p_rawCurrentMeasurementsWriteIndex = 0;

    /* Reset outputs */
    self->p_output.dcLinkVoltageQ15    = 0;
    self->p_output.shuntCurrentsQ15[0] = self->p_output.shuntCurrentsQ15[1] = 0;

    /* Reset module parameters */
    self->p_currentAccumulator = 0UL;
    self->p_cycleCounter       = 0U;
    self->p_offset             = 0U;
    self->p_enable             = false;
    self->p_calibrateCSA       = false;

    /* Initialize state */
    self->p_status.state = Ifx_MHA_MeasurementADC_F16_TLE987_State_init;

    /* Initialize peripherals and current gains according to static configuration */
    Ifx_MHA_MeasurementADC_F16_TLE987_setCsaGain(self,
        (Ifx_MHA_MeasurementADC_F16_TLE987_optionCsaGain)IFX_MHA_MEASUREMENTADC_F16_TLE987_CFG_CSA_GAIN);
    Ifx_MHA_MeasurementADC_F16_TLE987_setVDHRange(IFX_MHA_MEASUREMENTADC_F16_TLE987_CFG_VMON_SEN_SEL_INRANGE);
}


/* Period match interrupt */
void Ifx_MHA_MeasurementADC_F16_TLE987_periodMatch(Ifx_MHA_MeasurementADC_F16_TLE987* self)
{
    /* Read ADC EIM result register */
    self->p_rawCurrentMeasurements[self->p_rawCurrentMeasurementsWriteIndex][0] = ADC1_EIM_Result_Get();
}


/* One match interrupt */
void Ifx_MHA_MeasurementADC_F16_TLE987_oneMatch(Ifx_MHA_MeasurementADC_F16_TLE987* self)
{
    /* Read ADC CSA result register */
    self->p_rawCurrentMeasurements[self->p_rawCurrentMeasurementsWriteIndex][1] = ADC1_CSA_Result_Get();

    /* Swap write index of raw current measurment buffer */
    Ifx_MHA_MeasurementADC_F16_TLE987_swapCurrentMeasurementWriteIndex(self);
}


/* Execute function in state on */
static inline Ifx_MHA_MeasurementADC_F16_TLE987_State Ifx_MHA_MeasurementADC_F16_TLE987_stateOn(
    Ifx_MHA_MeasurementADC_F16_TLE987* self)
{
    Ifx_MHA_MeasurementADC_F16_TLE987_State nextState = self->p_status.state;

    /* Check if module is disabled */
    if (self->p_enable == false)
    {
        /* Disable the current sense amplifier */
        CSA_Power_Off();

        /* Set the state to OFF */
        nextState = Ifx_MHA_MeasurementADC_F16_TLE987_State_off;
    }
    else if (self->p_calibrateCSA == true)
    {
        nextState            = Ifx_MHA_MeasurementADC_F16_TLE987_State_calibration;
        self->p_calibrateCSA = false;
    }
    else
    {
        /* Do nothing */
    }

    /* DC link voltage and shunt currents calculation */
    Ifx_MHA_MeasurementADC_F16_TLE987_calc(self);

    return nextState;
}


/* Execute CSA calibration in state calibration */
static inline Ifx_MHA_MeasurementADC_F16_TLE987_State Ifx_MHA_MeasurementADC_F16_TLE987_executeCSACalibration(
    Ifx_MHA_MeasurementADC_F16_TLE987* self)
{
    Ifx_MHA_MeasurementADC_F16_TLE987_State nextState = self->p_status.state;

    /* Increment the p_currentAccumulator after the first cycle, the first cycle (p_cycleCounter =0) is used to
     * wait for the availability of the current measurement */
    if (self->p_cycleCounter > 0)
    {
        /* Accumulate the input current */
        self->p_currentAccumulator += self->p_rawCurrentMeasurements[self->p_rawCurrentMeasurementsReadIndex][0];
    }

    /* Calibration is still ongoing */
    if (self->p_cycleCounter < IFX_MHA_MEASUREMENTADC_F16_TLE987_CFG_CALIBRATION_CYCLES)
    {
        self->p_cycleCounter++;
    }

    /* Calibration is done, get the average current */
    else
    {
        /* Get the average value */
        self->p_offset = (uint16_t)(self->p_currentAccumulator / self->p_cycleCounter);

        /* Reset cycle counter and accumulator variable */
        self->p_cycleCounter       = 0U;
        self->p_currentAccumulator = 0U;

        /* Automatic transition to ON */
        nextState = Ifx_MHA_MeasurementADC_F16_TLE987_State_on;
    }

    return nextState;
}


__USED void Ifx_MHA_MeasurementADC_F16_TLE987_execute(Ifx_MHA_MeasurementADC_F16_TLE987* self)
{
    /* Variable to store the state */
    Ifx_MHA_MeasurementADC_F16_TLE987_State previousState = self->p_status.state;
    Ifx_MHA_MeasurementADC_F16_TLE987_State nextState     = previousState;

    switch (previousState)
    {
        /* Initialize ADC and CSA */
        case Ifx_MHA_MeasurementADC_F16_TLE987_State_init:
            CSA_Power_Off();

            /* Change ADC to sequencer mode */
            ADC1_SetMode(SEQ_MODE);

            /* Set the state to OFF */
            nextState = Ifx_MHA_MeasurementADC_F16_TLE987_State_off;
            break;

        /* Current sense amplifier powered on and ADC measuring outputs */
        case Ifx_MHA_MeasurementADC_F16_TLE987_State_on:

            /* Execute function in state on */
            nextState = Ifx_MHA_MeasurementADC_F16_TLE987_stateOn(self);
            break;

        /* Current sense amplifier powered off */
        case Ifx_MHA_MeasurementADC_F16_TLE987_State_off:

            /* Check if module enabled */
            if (self->p_enable == true)
            {
                /* Enable the current sense amplifier */
                CSA_Power_On();
                nextState = Ifx_MHA_MeasurementADC_F16_TLE987_State_calibration;
            }

            break;

        /* Calibration of the current sense amplifier */
        case Ifx_MHA_MeasurementADC_F16_TLE987_State_calibration:

            /* DC link voltage and shunt currents calculation */
            Ifx_MHA_MeasurementADC_F16_TLE987_calc(self);

            /* CSA calibration */
            nextState = Ifx_MHA_MeasurementADC_F16_TLE987_executeCSACalibration(self);
            break;

        default:

            /* do default transition to INIT */
            nextState = Ifx_MHA_MeasurementADC_F16_TLE987_State_init;
            break;
    }

    self->p_status.state = nextState;
}


static inline Ifx_Math_Fract16 Ifx_MHA_MeasurementADC_F16_TLE987_scaleCurrent(Ifx_MHA_MeasurementADC_F16_TLE987* self,
                                                                              uint16_t
                                                                              rawCurrentMeasurement)
{
    /* Stores the result of the ADC without the CSA offset */
    int16_t          adcWithoutOffset;

    /* Stores the value of the scaled current */
    Ifx_Math_Fract32 scaledCurrent;

    /* Scale shunt current */
    adcWithoutOffset = (int16_t)rawCurrentMeasurement - (int16_t)self->p_offset;
    scaledCurrent    = (Ifx_Math_Fract32)adcWithoutOffset * self->p_currentGain.value;

    /* Convert to 16-bit Q15 value */
    return Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(scaledCurrent, (uint8_t)self->p_currentGain.qFormat));
}


static inline void Ifx_MHA_MeasurementADC_F16_TLE987_calc(Ifx_MHA_MeasurementADC_F16_TLE987* self)
{
    /* Read DC-Link voltage and scale it */
    Ifx_Math_Fract32 scaledDcLink = (Ifx_Math_Fract32)ADC1_VDH_Result_Get() *
                                    IFX_MHA_MEASUREMENTADC_F16_TLE987_CFG_CONVERT_VDC_TO_Q15;
    self->p_output.dcLinkVoltageQ15 = Ifx_Math_Sat_F16_F32(Ifx_Math_ShR_F32(scaledDcLink,
        IFX_MHA_MEASUREMENTADC_F16_TLE987_CFG_VDC_BITS));

    /* Read and scale first shunt current */
    self->p_output.shuntCurrentsQ15[0] = Ifx_MHA_MeasurementADC_F16_TLE987_scaleCurrent(self,
        self->p_rawCurrentMeasurements[self->p_rawCurrentMeasurementsReadIndex][0]);

    /* Read and scale second shunt current */
    self->p_output.shuntCurrentsQ15[1] = Ifx_MHA_MeasurementADC_F16_TLE987_scaleCurrent(self,
        self->p_rawCurrentMeasurements[self->p_rawCurrentMeasurementsReadIndex][1]);
}


static inline void Ifx_MHA_MeasurementADC_F16_TLE987_swapCurrentMeasurementWriteIndex(
    Ifx_MHA_MeasurementADC_F16_TLE987* self)
{
    /* Store actual write index */
    uint8_t writeIndex = self->p_rawCurrentMeasurementsWriteIndex;

    /* Swap actual write index with actual read index */
    self->p_rawCurrentMeasurementsWriteIndex = self->p_rawCurrentMeasurementsReadIndex;

    /* Swap actual read index with previously stored write index */
    self->p_rawCurrentMeasurementsReadIndex = writeIndex;
}


static inline void Ifx_MHA_MeasurementADC_F16_TLE987_setVDHRange(uint32_t attenuatorRange)
{
    if (attenuatorRange == ADC1_VDH_Attenuator_Range_0_20V)
    {
        ADC1_VDH_Attenuator_Range_0_20V_Set();
    }
    else
    {
        ADC1_VDH_Attenuator_Range_0_30V_Set();
    }
}
