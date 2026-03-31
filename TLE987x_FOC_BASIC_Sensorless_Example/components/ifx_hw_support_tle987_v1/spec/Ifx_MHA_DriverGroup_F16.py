"""Ifx_MHA_DriverGroup_F16 module."""  # noqa: INP001

from component_config.cxml import cxml_dataclass
from pydantic import Field, computed_field


@cxml_dataclass
class Ifx_MHA_DriverGroup_F16:  # noqa: N801
    """Ifx_MHA_DriverGroup_F16 component configuration."""

    # Base voltage for Q15 normalization of voltage related parameters.
    base_voltage: float = Field(
        alias="baseVoltage",
        ge=0.0,
        le=10000.0,
    )  # in V

    # Base current for Q15 normalization of current related parameters.
    base_current: float = Field(
        alias="baseCurrent",
        ge=0.0,
        le=10000.0,
    )  # in A

    # PWM frequency
    frequency: int = Field(
        ge=0,
        le=100,
    )  # in kHz

    # This defines the current loop factor in counts of multiples of the PWM-
    # cycle to define the cyclic call of current control loop.
    current_loop_factor: int = Field(
        alias="currentLoopFactor",
        ge=1,
        le=20,
    )  # in counts

    @computed_field(alias="currentLoopFactor_DU")  # type: ignore[misc]
    @property
    def current_loop_factor_DU(self) -> int:
        return self.current_loop_factor

    # Time in which the component is executed. This is the speed loop period /
    # mid timebase.
    speed_loop_period: int = Field(
        alias="speedLoopPeriod",
        ge=0,
        le=65535,
    )  # in us

    @computed_field(alias="speedLoopPeriod_DU")  # type: ignore[misc]
    @property
    def speed_loop_period_DU(self) -> int:
        return self.speed_loop_period

    # This configures the value of the HighSide-LowSide switching dead-time
    dead_time: float = Field(
        alias="deadTime",
        ge=0.05,
        le=2.0,
    )  # in us
