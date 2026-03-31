"""Ifx_MDA_FluxEstimator_F16 module."""  # noqa: INP001

import math
from enum import IntEnum
from functools import cached_property

from component_config.cxml import PDO, PDOField, cxml_dataclass
from component_config_util.decorators import decimal_places
from component_config_util.qformat import Q15, Q15_MAX_FLOAT
from pydantic import Field, computed_field


class Mode(IntEnum):
    """Flux estimator mode."""

    Disable = 0
    Enable = 1


@cxml_dataclass
class Ifx_MDA_FluxEstimator_F16:  # noqa: N801
    """Ifx_MDA_FluxEstimator_F16 component configuration."""

    # Period in which the flux estimator is executed
    sampling_time: int = Field(
        alias="samplingTime",
        ge=1,
        le=65535,
    )  # in us

    @computed_field(alias="samplingTime_DU")  # type: ignore[misc]
    @property
    def sampling_time_DU(self) -> int:
        return self.sampling_time

    @computed_field(alias="alphaFilterTimeConstant")  # type: ignore[misc]
    @property
    def alpha_tc(self) -> int:
        return math.floor(1e6 / (2 * math.pi * self.alpha_cutoff))

    @computed_field(alias="alphaFilterTimeConstant_DU")  # type: ignore[misc]
    @property
    def alpha_tc_DU(self) -> int:
        return self.alpha_tc

    @computed_field(alias="betaFilterTimeConstant")  # type: ignore[misc]
    @property
    def beta_tc(self) -> int:
        return math.floor(1e6 / (2 * math.pi * self.beta_cutoff))

    @computed_field(alias="betaFilterTimeConstant_DU")  # type: ignore[misc]
    @property
    def beta_tc_DU(self) -> int:
        return self.beta_tc

    @computed_field(alias="speedFilterTimeConstant")  # type: ignore[misc]
    @property
    def speed_tc(self) -> int:
        return math.floor(1e6 / (2 * math.pi * self.speed_cutoff))

    @computed_field(alias="speedFilterTimeConstant_DU")  # type: ignore[misc]
    @property
    def speed_tc_DU(self) -> int:
        return self.speed_tc

    # PLL filter gain.
    pll_gain: float = Field(
        alias="pllFilterPropGain",
        ge=0.0,
        le=214748.3647,
    )

    @computed_field(alias="pllFilterPropGain_DU")  # type: ignore[misc]
    @property
    @decimal_places(4)
    def pll_gain_DU(self) -> float:
        return self.pll_gain

    @cached_property
    def _adjustment_factor(self) -> float:
        min_base_time = self.phase_inductance * 1e-3 * self.base_current**2 / self.base_power
        max_alpha_beta = max(self.alpha_tc, self.beta_tc) * 1e-6
        new_base_time = max(min_base_time, max_alpha_beta)

        return (self.base_time * 1e-3) / new_base_time

    # Phase resistance
    phase_res: float = Field(
        alias="phaseRes",
        ge=0.0,
        le_dyn=lambda p: Q15_MAX_FLOAT * p["base_resistance"],
    )  # in Ohm

    @computed_field(alias="phaseRes_DU")  # type: ignore[misc]
    @property
    def phase_res_DU(self) -> int:
        return Q15(self.phase_res / self.base_resistance).stored_integer

    @computed_field(alias="phaseIndAdjusted")  # type: ignore[misc]
    @property
    def phase_ind_adjusted(self) -> int:
        """Phase inductance adjusted.

        Same adjustment factor is used for alpha and beta gains and inductance.
        Adjustment is needed because usually the filter time constants are relatively
        large compared to the other time values on the system.
        Due to this fact, the system base time might not be enough to
        normalize the filter gains (which are the same as the time constants) and
        cause overflows.

        Returns
        -------
            int: adjusted phase inductance

        """
        return math.floor(self._average_inductance_Q15 * self._adjustment_factor)

    @computed_field(alias="phaseIndAdjusted_DU")  # type: ignore[misc]
    @property
    def phase_ind_adjusted_DU(self) -> int:
        return self.phase_ind_adjusted

    @property
    def _alpha_gain_q14(self) -> int:
        return math.floor(2**14 * self.alpha_tc_DU * 1e-6 / (self.base_time * 1e-3))

    @computed_field(alias="alphaGainAdjusted")  # type: ignore[misc]
    @property
    def alpha_gain_adjusted(self) -> int:
        """Alpha gain adjusted.

        Same adjustment factor is used for alpha and beta gains and inductance.
        Adjustment is needed because usually the filter time constants are relatively
        large compared to the other time values on the system.
        Due to this fact, the system base time might not be enough to
        normalize the filter gains (which are the same as the time constants) and
        cause overflows.

        Returns
        -------
            int: adjusted alpha gain

        """
        return math.floor(self._alpha_gain_q14 * self._adjustment_factor)

    @computed_field(alias="alphaGainAdjusted_DU")  # type: ignore[misc]
    @property
    def alpha_gain_adjusted_DU(self) -> int:
        return self.alpha_gain_adjusted

    @property
    def _beta_gain_q14(self) -> int:
        return math.floor(2**14 * self.beta_tc_DU * 1e-6 / (self.base_time * 1e-3))

    @computed_field(alias="betaGainAdjusted")  # type: ignore[misc]
    @property
    def beta_gain_adjusted(self) -> int:
        """Beta gain adjusted.

        Same adjustment factor is used for alpha and beta gains and inductance.
        Adjustment is needed because usually the filter time constants are relatively
        large compared to the other time values on the system.
        Due to this fact, the system base time might not be enough to
        normalize the filter gains (which are the same as the time constants) and
        cause overflows.

        Returns
        -------
            int: adjusted beta gain

        """
        return math.floor(self._beta_gain_q14 * self._adjustment_factor)

    @computed_field(alias="betaGainAdjusted_DU")  # type: ignore[misc]
    @property
    def beta_gain_adjusted_DU(self) -> int:
        return self.beta_gain_adjusted

    # System base current
    base_current: float = Field(
        alias="baseCurrent",
        ge=0.001,
        le=10000.0,
    )  # in A

    # Base time for Q15 normalization of time related parameters.
    base_time: float = Field(
        alias="systemBaseTime",
        ge=0.0001,
        le=318.3,
    )  # in ms

    @computed_field(alias="systemBaseTime_DU")  # type: ignore[misc]
    @property
    def base_time_DU(self) -> int:
        return math.floor((2**30 * 2 * math.pi) * (self.base_time * 1e-3))

    # Base power for Q15 normalization of power related parameters.
    base_power: float = Field(
        alias="basePower",
        ge=0,
        le=10000.0,
    )  # in W

    # Base inductance for Q15 normalization of inductance related parameters. Is
    # calculated = base_power*base_time*base_current^-2
    base_inductance: float = Field(
        alias="baseInductance",
        ge=0.0,
        le=100.0,
    )  # in mH

    # Base resistance for Q15 normalization of resistance related parameters. Is
    # calculated = base_power*base_current^-2
    base_resistance: float = Field(
        alias="baseResistance",
        ge=0.0,
        le=100.0,
    )  # in Ohm

    # Phase inductance
    phase_inductance: float = Field(
        alias="phaseInd", ge=0.0, le_dyn=lambda p: Q15_MAX_FLOAT * p["base_inductance"]
    )  # in mH

    @property
    def _average_inductance_Q15(self) -> int:
        return Q15(self.phase_inductance / self.base_inductance).stored_integer

    # Alpha lowpass cut-off frequency
    alpha_cutoff: float = Field(
        alias="alphaCutoff",
        ge=0.0,
        le=214748.0,
    )  # in Hz

    # Beta lowpass cut-off frequency
    beta_cutoff: float = Field(
        alias="betaCutoff",
        ge=0.0,
        le=214748.0,
    )  # in Hz

    # Base current  is used for normalization
    speed_cutoff: float = Field(
        alias="speedCutoff",
        ge=0.0,
        le=214748.0,
    )  # in Hz

    # Estimated rotor flux speed in % of Base electrical speed.
    speed: PDO[float] = PDOField(
        ge=-100.0,
        le=100.0,
    )  # in %

    # Estimated rotor flux angle. 0xFFFFFFFF = 2*pi = 360degree
    angle_pll: PDO[float] = PDOField(
        alias="anglePll",
        ge=0.0,
        le=6.2832,
    )  # in rad

    # Configured operation mode of the Flux Estimator
    mode: PDO[Mode] = PDOField()

    # Scaling factor for conversion from PU to TU. TODO: To be generated
    # implicitly!
    scaling_factor_tu: int = Field(
        alias="scalingFactorTu",
        ge=1,
        le=1000000,
    )

    @computed_field(alias="scalingFactorTu_DU")  # type: ignore[misc]
    @property
    def scaling_factor_tu_DU(self) -> int:
        return self.scaling_factor_tu
