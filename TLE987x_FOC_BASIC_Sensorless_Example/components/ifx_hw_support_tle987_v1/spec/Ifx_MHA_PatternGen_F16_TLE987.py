"""Ifx_MHA_PatternGen_F16_TLE987 module."""  # noqa: INP001

from __future__ import annotations

from enum import IntEnum

from component_config.cxml import PDO, PDOField, cxml_dataclass
from pydantic import Field, computed_field


class FaultReactionTrap(IntEnum):
    """Fault reaction of the Pattern Generator."""

    Disable = 0
    Enable = 1
    ReportOnly = 2
    ReportAndReact = 3


class StatusState(IntEnum):
    """State machine state of the Pattern Generator."""

    Init = 0
    Off = 1
    On = 2
    Fault = 3
    Brake = 4


@cxml_dataclass
class Ifx_MHA_PatternGen_F16_TLE987:  # noqa: N801
    """Ifx_MHA_PatternGen_F16_TLE987 component configuration."""

    # This configures the value of the HighSide-LowSide switching dead-time
    dead_time: float = Field(
        alias="deadTime",
        ge=0.05,
        le=2.0,
    )  # in us

    # Changing ge or le dynamically in this case leads to wrong TU->DU conversion cause
    # DU is also in physical unit ns and therefore must be changed at the same time.
    # So better not adjust the limits at all. Just use fixed safe limits
    # that should work for all supported frequencies.
    # TODO: Can we also change min DU and max DU to keep scaling stable?

    # dead_time: float = Field(
    #    ge_dyn=lambda p: p["min_dead_time"],
    #    le_dyn=lambda p: 255 / (2 * p["frequency"]),
    # )  # in us

    @computed_field(alias="deadTime_DU")  # type: ignore[misc]
    @property
    def dead_time_DU(self) -> int:
        """Dead time.

        Returns
        -------
            int: Dead time in ns

        """
        return round(self.dead_time * 1000)

    # PWM frequency in kHz
    frequency: int = Field(
        serialization_alias="frequency_DU",
        ge=0,
        le=100,
    )  # in kHz

    # Minimum dead time in us
    min_dead_time: float = Field(
        alias="minDeadTime",
        ge=0,
        le=2.0,
    )  # in us

    @computed_field(alias="minDeadTime_DU")  # type: ignore[misc]
    @property
    def min_dead_time_DU(self) -> int:
        """Min dead time.

        Returns
        -------
            int: Min dead time in ns

        """
        return round(self.min_dead_time * 1000)

    # This defines the current loop factor in counts of multiples of the PWM-
    # cycle to define the cyclic call of current control loop.
    current_loop_factor: int = Field(
        alias="currentLoopFactor",
        ge=1,
        le=20,
    )

    @computed_field(alias="currentLoopFactor_DU")  # type: ignore[misc]
    @property
    def current_loop_factor_DU(self) -> int:
        return self.current_loop_factor

    # Fault reaction on trap condition
    fault_reaction_trap: FaultReactionTrap = Field(
        alias="faultReactionTrap",
    )

    @computed_field(alias="faultReactionTrap_DU")  # type: ignore[misc]
    @property
    def fault_reaction_trap_DU(self) -> FaultReactionTrap:
        return self.fault_reaction_trap

    # Enable fault user callback
    enable_fault_out: bool = Field(
        alias="enableFaultOut",
    )

    @computed_field(alias="enableFaultOut_DU")  # type: ignore[misc]
    @property
    def enable_fault_out_DU(self) -> bool:
        return self.enable_fault_out

    # Inverse rotation direction
    invert_rotation: bool = Field(
        alias="invertRotation",
    )

    @computed_field(alias="invertRotation_DU")  # type: ignore[misc]
    @property
    def invert_rotation_DU(self) -> bool:
        return self.invert_rotation

    # Status of the Pattern Generator, containing the bit
    # coded errors and the state machine state
    state: PDO[StatusState] = PDOField(
        alias="state",
    )
