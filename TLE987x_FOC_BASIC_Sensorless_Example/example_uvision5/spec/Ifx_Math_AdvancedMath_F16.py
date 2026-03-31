"""Ifx_Math_AdvancedMath_F16 module."""  # noqa: INP001

from enum import IntEnum

from component_config.cxml import cxml_dataclass
from pydantic import Field, computed_field


class PiDiscretizationMethod(IntEnum):
    """PI controllers discretization method."""

    Trapezoidal = 0
    BackwardEuler = 1
    ForwardEuler = 2


class PiAntiwindupMethod(IntEnum):
    """PI controllers anti-windup method."""

    NoAntiwindup = 0
    Clamp = 1
    BackCalculation = 2


class PllDelayLength(IntEnum):
    """PLL delay length."""

    TwoCycles = 2
    FourCycles = 4
    EightCycles = 8
    SixteenCycles = 16
    ThirtytwoCycles = 32


@cxml_dataclass
class Ifx_Math_AdvancedMath_F16:  # noqa: N801
    """Ifx_Math_AdvancedMath_F16 class."""

    # PI controllers discretization method
    pi_discretization_method: PiDiscretizationMethod = Field(
        alias="piDisc",
    )

    @computed_field(alias="piDisc_DU")  # type: ignore[misc]
    @property
    def pi_discretization_method_DU(self) -> PiDiscretizationMethod:
        return self.pi_discretization_method

    # PI controllers anti-windup method
    pi_antiwindup_method: PiAntiwindupMethod = Field(
        alias="piAntiWind",
    )

    @computed_field(alias="piAntiWind_DU")  # type: ignore[misc]
    @property
    def pi_antiwindup_method_DU(self) -> PiAntiwindupMethod:
        return self.pi_antiwindup_method

    # PLL delay length
    pll_delay_length: PllDelayLength = Field(
        alias="pllDelayLength",
    )

    @computed_field(alias="pllDelayLength_DU")  # type: ignore[misc]
    @property
    def pll_delay_length_DU(self) -> PllDelayLength:
        return self.pll_delay_length
