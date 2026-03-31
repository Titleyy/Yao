"""Ifx_MDA_FocController_F16 module."""  # noqa: INP001

from __future__ import annotations

import warnings
from enum import IntEnum
from typing import Any

from component_config.cxml import PDO, PDOField, cxml_dataclass
from component_config_util.qformat import Q15, Q15_MAX_FLOAT
from pydantic import Field, computed_field

MAX_UINT32 = 2**32 - 1
IFX_MATH_FRACT16_MAX = 32767
# Scaling factor used to up shift 16bits of Ifx_Math_Fract16Q(variable Qformat) value to store it in a uint32 variable for transfer
# e.g. Ifx_Math_Fract16Q of 0.00778 (value=255, qFormat=q15) becomes uint32 of 512
VARIABLEQ_TO_U32_SHIFT = 2**16


class AntiwindupCtrl(IntEnum):
    """AntiwindupCtrl Enum."""

    NoSaturation = 0
    AntiWindupPositiv = 1
    AntiWindupNegativ = 2


def _current_dq_pi_integ_gain_max(p: dict[str, Any]) -> float:
    # Calculate the maxmium valuePU (maxPU) for current_d_pi_integ,_gain current_q_pi_integ_gain.
    # The max ranges needs to be adjusted so that maxDU/TU/PU fullfill the 2 constraints:
    # 1) maxTU < maxDU
    # Otherwise TU->DU transformation in ParamTable looses information and value will drift in write read cycles.
    # Note1: maxPU * 10**decimal_places = maxTU
    # Note2: maxDU = IFX_MATH_FRACT16_MAX*VARIABLEQ_TO_U32_SHIFT This is the saturation limit of Ifx_Math_ConvToQForm_F16().
    # with a reserve for adjustment of base values
    # maxDU = IFX_MATH_FRACT16_MAX*VARIABLEQ_TO_U32_SHIFT
    # 2) defaultPU needs to result in defaultDU with an acceptable precision of 3..5 digits or 8..16 bits
    #    to have a tuning reserve in both directions.
    #
    # The max DU/TU/PU ranges can be adjusted by choosing three factors
    # a) VARIABLEQ_TO_U32_SHIFT (can be 2**0 .. 2**16)
    # b) physicalUnit (can be mV/As, V/As , kV/As, MV/As)
    # c) decimalPlaces (is limited to values 0..4)
    #
    # VARIABLEQ_TO_U32_SHIFT can be chosen always as high as possible 2**16.
    #    Higher exponents would break int32 max. Lower would just reduce precision.
    #    maxDU = 32767 * 2**16 / 1000 = 2,147,418,112
    # physicalUnit = "V/As" and decimalPlaces = "0" full fill the constraints. So we choose this solution.
    # defaultPU = 250 V/As
    # defaultDU = 512 uses 9 bits of 32bits
    # Note: physicalUnit = "kV/As" and decimalPlaces = "3" would full fill the constraints, too.
    #
    #
    # maxPU = IFX_MATH_FRACT16_MAX * base_voltage / (sampling_time * 1e-6 * base_current )
    #                      = 32767 * 24V          / (150us         * 1e-6 * 5A           )
    #                      = 32767 * 32000 V/As = 1,048,544,000 V/As
    max_pu = (
        IFX_MATH_FRACT16_MAX * p["base_voltage"] / (p["sampling_time"] * 1e-6 * p["base_current"])
    )

    max_tu = max_pu * 10**0  # decimalPlaces = 0
    # maxDU = IFX_MATH_FRACT16_MAX * VARIABLEQ_TO_U32_SHIFT

    # TODO: Throwing this exception is not acceptable for a customer release, still root cause needs to be fixed.
    # When base_current is reduced to 2A and base voltage is increased to 48V we already have a case
    # were maxTU would exceed maxDU and we see a voltage drift on the UI.
    # Note1: This drift is not necessarily an exception. This depends on UI implementation.
    # Note2: Transfer of normalized variable Q parameters over DTI would solve this
    # Or we have to dynamically adjust not only maxPU but also physicalValue and decimalPlaces.
    # if maxTU > maxDU:
    #    raise OverflowError(
    #        "pi_integ_gain maxTU > maxDU. Adjust physicalUnit and decimalPlaces in cxml. "
    #    )

    # Note: Base value changes can lead to exceeding uint32 range.
    if max_tu > MAX_UINT32:
        warnings.warn("pi_integ_gain maxTU exceeds uint32", stacklevel=2)
        return MAX_UINT32 / 10**0  # decimalPlaces = 0

    return max_pu


def _current_dq_pi_prop_gain_max(p: dict[str, Any]) -> float:
    # Calculate the maxmium valuePU (maxPU) for current_d_pi_prop_gain and current_q_pi_prop_gain.
    #
    # Solution of max range adjustment:
    # VARIABLEQ_TO_U32_SHIFT = 2**16
    # physicalUnit = "V/A"
    # decimalPlaces = 3
    # maxPU = IFX_MATH_FRACT16_MAX * base_voltage / base_current
    #                      = 32767 * 24V          / 5A
    #                      = 32767 * 4.8 V/A = 157,281.6 V/A
    # maxTU = 157281600  < uint32 and < maxDU
    # defaultPU = 0.35 V/A
    # defaultDU = 4779 uses 13bits of 32bits
    #
    # See _current_dq_pi_integ_gain_max comment for details
    #
    max_pu = IFX_MATH_FRACT16_MAX / p["base_current"] * p["base_voltage"]

    max_tu = max_pu * 10**3  # decimalPlaces = 3

    if max_tu > MAX_UINT32:
        warnings.warn("pi_integ_gain maxTU exceeds uint32", stacklevel=2)
        return MAX_UINT32 / 10**3  # decimalPlaces = 3

    return max_pu


def _current_dq_pi_anti_windup_gain_max(p: dict[str, Any]) -> float:
    # Calculate the maxmium valuePU for current_d_pi_anti_windup_gain and current_q_pi_anti_windup_gain.
    # Solution of max range adjustment:
    # VARIABLEQ_TO_U32_SHIFT = 2**16
    # physicalUnit = "1/As"
    # decimalPlaces = 1
    # maxPU = IFX_MATH_FRACT16_MAX / (sampling_time * 1e-6 * base_current )
    #                      = 32767 / (150us         * 1e-6 * 5A)
    #                      = 32767 / 0.00075 As  = 43,689,333 1/As
    # maxTU = 436,893,330  < uint32 and < maxDU
    # defaultPU = 1.0 1/As
    # defaultDU = 49 uses 6bits of 32bits
    #
    # See _current_dq_pi_integ_gain_max comment for details
    #

    max_pu = IFX_MATH_FRACT16_MAX / (p["sampling_time"] * 1e-6 * p["base_current"])

    max_tu = max_pu * 10**1  # decimalPlaces = 1

    if max_tu > MAX_UINT32:
        warnings.warn("pi_integ_gain maxTU exceeds uint32", stacklevel=2)
        return MAX_UINT32 / 10**1  # decimalPlaces = 1

    return max_pu


@cxml_dataclass
class Ifx_MDA_FocController_F16:  # noqa: N801
    """Ifx_MDA_FocController_F16 component configuration."""

    # Id PI controller Integral gain
    current_d_pi_integ_gain: float = Field(
        alias="currentDPiIntegGainSamplingTime",
        ge=0.0,
        le_dyn=_current_dq_pi_integ_gain_max,
    )  # in V/As

    @computed_field(alias="currentDPiIntegGainSamplingTime_DU")  # type: ignore[misc]
    @property
    def current_d_pi_integ_gain_DU(self) -> int:
        return int(
            self.current_d_pi_integ_gain
            * VARIABLEQ_TO_U32_SHIFT
            * (self.sampling_time * 1e-6 * self.base_current)
            / self.base_voltage
        )

    # Id PI controller Output lower limit  in % of base voltage
    current_d_pi_lower_limit: float = Field(
        alias="currentDPiLowerLimit",
        ge=-100.0,
        le=0.0,
    )  # in %

    @computed_field(alias="currentDPiLowerLimit_DU")  # type: ignore[misc]
    @property
    def current_d_pi_lower_limit_DU(self) -> int:
        return Q15.from_percent_symmetric(self.current_d_pi_lower_limit).stored_integer

    # Id PI controller Output upper limit  in % of base voltage
    current_d_pi_upper_limit: float = Field(
        alias="currentDPiUpperLimit",
        ge=0.0,
        le=100.0,
    )  # in %

    @computed_field(alias="currentDPiUpperLimit_DU")  # type: ignore[misc]
    @property
    def current_d_pi_upper_limit_DU(self) -> int:
        return Q15.from_percent_symmetric(self.current_d_pi_upper_limit).stored_integer

    # Id PI controller Proportional gain
    current_d_pi_prop_gain: float = Field(
        alias="currentDPiPropGain",
        ge=0.0,
        le_dyn=_current_dq_pi_prop_gain_max,
    )  # in V/A

    @computed_field(alias="currentDPiPropGain_DU")  # type: ignore[misc]
    @property
    def current_d_pi_prop_gain_DU(self) -> int:
        return int(
            self.current_d_pi_prop_gain
            * VARIABLEQ_TO_U32_SHIFT
            * self.base_current
            / self.base_voltage
        )

    # Id PI controller Anti windup gain
    current_d_pi_anti_windup_gain: float = Field(
        alias="currentDPiAntiWindupGainSamplingTime",
        ge=0.0,
        le_dyn=_current_dq_pi_anti_windup_gain_max,
    )  # in 1/As

    @computed_field(alias="currentDPiAntiWindupGainSamplingTime_DU")  # type: ignore[misc]
    @property
    def current_d_pi_anti_windup_gain_DU(self) -> int:
        return int(
            self.current_d_pi_anti_windup_gain
            * VARIABLEQ_TO_U32_SHIFT
            * (self.sampling_time * 1e-6 * self.base_current)
        )

    # Iq PI controller Integral gain
    current_q_pi_integ_gain: float = Field(
        alias="currentQPiIntegGainSamplingTime",
        ge=0.0,
        le_dyn=_current_dq_pi_integ_gain_max,
    )  # in V/As

    @computed_field(alias="currentQPiIntegGainSamplingTime_DU")  # type: ignore[misc]
    @property
    def current_q_pi_integ_gain_DU(self) -> int:
        return int(
            self.current_q_pi_integ_gain
            * VARIABLEQ_TO_U32_SHIFT
            * (self.sampling_time * 1e-6 * self.base_current)
            / (self.base_voltage)
        )

    # Iq PI controller Output lower limit in % of base voltage
    current_q_pi_lower_limit: float = Field(
        alias="currentQPiLowerLimit",
        ge=-100.0,
        le=0.0,
    )  # in %

    @computed_field(alias="currentQPiLowerLimit_DU")  # type: ignore[misc]
    @property
    def current_q_pi_lower_limit_DU(self) -> int:
        return Q15.from_percent_symmetric(self.current_q_pi_lower_limit).stored_integer

    # Iq PI controller Output upper limit  in % of base voltage
    current_q_pi_upper_limit: float = Field(
        alias="currentQPiUpperLimit",
        ge=0.0,
        le=100.0,
    )  # in %

    @computed_field(alias="currentQPiUpperLimit_DU")  # type: ignore[misc]
    @property
    def current_q_pi_upper_limit_DU(self) -> int:
        return Q15.from_percent_symmetric(self.current_q_pi_upper_limit).stored_integer

    # Iq PI controller Proportional gain
    current_q_pi_prop_gain: float = Field(
        alias="currentQPiPropGain",
        ge=0.0,
        le_dyn=_current_dq_pi_prop_gain_max,
    )  # in V/A

    @computed_field(alias="currentQPiPropGain_DU")  # type: ignore[misc]
    @property
    def current_q_pi_prop_gain_DU(self) -> int:
        return int(
            self.current_q_pi_prop_gain
            * VARIABLEQ_TO_U32_SHIFT
            * self.base_current
            / self.base_voltage
        )

    # Iq PI controller Anti windup gain
    current_q_pi_anti_windup_gain: float = Field(
        alias="currentQPiAntiWindupGainSamplingTime",
        ge=0.0,
        le_dyn=_current_dq_pi_anti_windup_gain_max,
    )  # in 1/As

    @computed_field(alias="currentQPiAntiWindupGainSamplingTime_DU")  # type: ignore[misc]
    @property
    def current_q_pi_anti_windup_gain_DU(self) -> int:
        return int(
            self.current_q_pi_anti_windup_gain
            * VARIABLEQ_TO_U32_SHIFT
            * (self.sampling_time * 1e-6 * self.base_current)
        )

    # Base voltage for Q15 normalization of voltage related parameters.
    base_voltage: float = Field(
        alias="baseVoltage",
        ge=0.001,
        le=10000.0,
    )  # in V

    # Base current for Q15 normalization of current related parameters.
    base_current: float = Field(
        alias="baseCurrent",
        ge=0.001,
        le=10000.0,
    )  # in A

    # Base inductance for Q15 normalization of inductance related parameters. Is
    # calculated = base_power*base_time*base_current^-2
    base_inductance: float = Field(
        alias="baseInductance",
        ge=0.0,
        le=100.0,
    )  # in mH

    # Foc Controller Task Cycle Time
    sampling_time: float = Field(
        alias="samplingTime",
        ge=1.0,
        le=65535.0,
    )  # in us

    # Motor D-Axis Inductance
    direct_inductance: float = Field(
        alias="directInductance",
        ge=0.0,
        le_dyn=lambda p: Q15_MAX_FLOAT * p["base_inductance"],
    )  # in mH

    @computed_field(alias="directInductance_DU")  # type: ignore[misc]
    @property
    def direct_inductance_DU(self) -> int:
        return Q15(self.direct_inductance / self.base_inductance).stored_integer

    # Motor Q-Axis Inductance
    quadrature_inductance: float = Field(
        alias="quadratureInductance",
        ge=0.0,
        le_dyn=lambda p: Q15_MAX_FLOAT * p["base_inductance"],
    )  # in mH

    @computed_field(alias="quadratureInductance_DU")  # type: ignore[misc]
    @property
    def quadrature_inductance_DU(self) -> int:
        return Q15(self.quadrature_inductance / self.base_inductance).stored_integer

    # Enable DQ-Axis decoupling
    dq_decoupling_enable: bool = Field(
        alias="dqDecouplingEnable",
    )

    @computed_field(alias="dqDecouplingEnable_DU")  # type: ignore[misc]
    @property
    def dq_decoupling_enable_DU(self) -> bool:
        return self.dq_decoupling_enable

    # Voltage Limit D-AxisPriorization ON-OFF
    limit_volt_vector_d_prio: bool = Field(
        alias="limitVoltVectorDPrio",
    )

    @computed_field(alias="limitVoltVectorDPrio_DU")  # type: ignore[misc]
    @property
    def limit_volt_vector_d_prio_DU(self) -> bool:
        return self.limit_volt_vector_d_prio

    # This monitoring output represents the saturation status of the voltage on
    # the D-Axis. Status of the anti windup control in FocController.
    d_antiwindup_ctrl: PDO[AntiwindupCtrl] = PDOField(
        alias="dAntiwindupCtrl",
    )

    # This monitoring output represents the saturation status of the voltage on
    # the Q-Axis. Status of the anti windup control in FocController.
    q_antiwindup_ctrl: PDO[AntiwindupCtrl] = PDOField(
        alias="qAntiwindupCtrl",
    )

    # This output represents the direct current of the FocController.
    current_dq_real: PDO[float] = PDOField(
        alias="currentDqReal",
        ge_dyn=lambda p: -p["base_current"],
        le_dyn=lambda p: p["base_current"],
    )  # in A

    # This output represents the quadarature current of the FocController.
    current_dq_imag: PDO[float] = PDOField(
        alias="currentDqImag",
        ge_dyn=lambda p: -p["base_current"],
        le_dyn=lambda p: p["base_current"],
    )  # in A

    # This output represents the amplitude of the requested voltage space
    # vector.
    voltage_command_polar_amplitude: PDO[int] = PDOField(
        alias="voltageCommandPolarAmplitude",
        ge=-32767,
        le=32767,
    )

    # This output represents the angle of the requested voltage space vector.
    # This is equal to the input reference voltage angle. 0xFFFFFFFF = 2*pi =
    # 360degree
    voltage_command_polar_angle: PDO[float] = PDOField(
        alias="voltageCommandPolarAngle",
        ge=0.0,
        le=6.2832,
    )  # in rad
