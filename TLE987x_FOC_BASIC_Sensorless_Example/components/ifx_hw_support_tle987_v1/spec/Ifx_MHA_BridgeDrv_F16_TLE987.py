"""Ifx_MHA_BridgeDrv_F16_TLE987 module."""  # noqa: INP001

from enum import IntEnum

from component_config.cxml import PDO, PDOField, cxml_dataclass
from pydantic import Field, computed_field


class FaultReaction(IntEnum):
    """Fault reaction of the Bridge Driver."""

    Disable = 0
    Enable = 1
    ReportOnly = 2
    ReportAndReact = 3


class StatusState(IntEnum):
    """State machine state of the Bridge Driver."""

    Init = 0
    Off = 1
    On = 2
    Fault = 3


@cxml_dataclass
class Ifx_MHA_BridgeDrv_F16_TLE987:  # noqa: N801
    """Ifx_MHA_BridgeDrv_F16_TLE987 component configuration."""

    # Time in which the component is executed. This is the speed loop period /
    # mid timebase.
    execution_period: int = Field(
        alias="executionPeriod",
        ge=0,
        le=65535,
    )  # in us

    # Maximum transition time from init to off state. If charge pump voltage is
    # stable transition is done earlier.
    transition_time_cycles: float = Field(
        alias="transitionTimeCycles",
        ge=0.0,
        le=32000.0,
    )  # in ms

    @computed_field(alias="transitionTimeCycles_DU")  # type: ignore[misc]
    @property
    def transition_time_cycles_DU(self) -> int:
        """Maximum transition time from init to off state.

        If charge pump voltage is stable transition is done earlier.

        Returns
        -------
            int: maximum transition time

        """
        return max(
            1,
            round(self.transition_time_cycles * 1e-3 / (self.execution_period * 1e-6)),
        )

    # Enable fault user callback
    enable_fault_out: bool = Field(
        alias="enableFaultOut",
    )

    @computed_field(alias="enableFaultOut_DU")  # type: ignore[misc]
    @property
    def enable_fault_out_DU(self) -> bool:
        return self.enable_fault_out

    # Fault reaction on overvoltage condition. When fault reaction is set to
    # Report and react fault output behavior is uncontrolled freewheeling OFF.
    fault_reaction_overvolt: FaultReaction = Field(
        alias="faultReactionOvervolt",
    )

    @computed_field(alias="faultReactionOvervolt_DU")  # type: ignore[misc]
    @property
    def fault_reaction_overvolt_DU(self) -> FaultReaction:
        return self.fault_reaction_overvolt

    # Fault reaction on undervoltage condition. When fault reaction is set to
    # Report and react fault output behavior is uncontrolled freewheeling OFF.
    fault_reaction_undervolt: FaultReaction = Field(
        alias="faultReactionUndervolt",
    )

    @computed_field(alias="faultReactionUndervolt_DU")  # type: ignore[misc]
    @property
    def fault_reaction_undervolt_DU(self) -> FaultReaction:
        return self.fault_reaction_undervolt

    # Fault out callback string
    fault_out: str = Field(
        alias="faultOut",
        default="usrFaultCallback",
        component_config_meta={  # remove if fixed
            "generate_pattern": "configDefine",
            "symbol_name": "CFG_FAULT_OUT",
        },
    )  # fault out user callback

    @computed_field(alias="faultOut_DU")  # type: ignore[misc]
    @property
    def fault_out_DU(self) -> str:
        return self.fault_out

    # TODO(MBR): clarify use of callback strings in CXML
    # https://jirard.intra.infineon.com/browse/ATVMCES-3090

    # State machine state of the Bridge Driver
    state: PDO[StatusState] = PDOField(
        alias="state",
    )

    # Overcurrent flag of the Bridge Driver
    error_overcurrent: PDO[bool] = PDOField(
        alias="error_overcurrent",
    )

    # Overvoltage flag of the Bridge Driver
    error_overvoltage: PDO[bool] = PDOField(
        alias="error_overvoltage",
    )

    # Undervoltage flag of the Bridge Driver
    error_undervoltage: PDO[bool] = PDOField(
        alias="error_undervoltage",
    )
