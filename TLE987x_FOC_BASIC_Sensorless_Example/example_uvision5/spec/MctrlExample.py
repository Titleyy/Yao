"""MctrlExample module."""  # noqa: INP001

import math
from enum import IntEnum

from component_config.cxml import cxml_dataclass
from pydantic import Field, computed_field


class ControlMode(IntEnum):
    """Control mode."""

    VtoF = 0
    ItoFFoc = 1


@cxml_dataclass
class MctrlExample:
    """MctrlExample component configuration."""

    # This control input enables or disables the speed
    # control based on the state machine specification
    enable_control: bool = Field(
        alias="enableControl",
    )

    @computed_field(alias="enableControl_DU")  # type: ignore[misc]
    @property
    def enable_control_DU(self) -> bool:
        return self.enable_control

    # This control input enables or disables the power
    # stage based on the state machine specification
    enable_power_stage: bool = Field(
        alias="enablePowerStage",
    )

    @computed_field(alias="enablePowerStage_DU")  # type: ignore[misc]
    @property
    def enable_power_stage_DU(self) -> bool:
        return self.enable_power_stage

    # This configures the number of time steps needed where the reference speed
    # is zero. Time step length is timebase mid period time.
    rotor_alignment_time: float = Field(
        alias="rotorAlignmentTime",
        ge=0,
        le=65.53,
    )  # in s

    @computed_field(alias="rotorAlignmentTime_DU")  # type: ignore[misc]
    @property
    def rotor_alignment_time_DU(self) -> int:
        return math.floor(
            self.rotor_alignment_time
            * 1e3
            * self.frequency
            / (self.current_loop_factor * self.speed_loop_factor)
        )

    # Sets control model between VtoF or Foc
    control_mode: ControlMode = Field(
        alias="controlMode",
    )

    @computed_field(alias="controlMode_DU")  # type: ignore[misc]
    @property
    def control_mode_DU(self) -> int:
        return self.control_mode.value

    # Reference speed in rpm
    reference_speed: int = Field(
        alias="referenceSpeed",
        ge_dyn=lambda p: -p["base_mech_speed"],
        le_dyn=lambda p: p["base_mech_speed"],
    )  # in rpm

    @computed_field(alias="referenceSpeed_DU")  # type: ignore[misc]
    @property
    def reference_speed_DU(self) -> int:
        return self.reference_speed

    # This control input clears the fault bits in the status
    # interface of underlying modules, e.g. Modulator,
    # Bridge Driver, Pattern Generator and Start Angle
    # Identification and requests the FocSolution to
    # leave the fault state based on the state machine
    # specification
    clr_fault_foc: bool = Field(
        alias="clrFaultFoc",
    )

    @computed_field(alias="clrFaultFoc_DU")  # type: ignore[misc]
    @property
    def clr_fault_foc_DU(self) -> bool:
        return self.clr_fault_foc

    # Base mechanical speed for Q15 normalization of mechanical speed related
    # parameters
    base_mech_speed: int = Field(
        alias="baseMechSpeed",
        ge=0,
        le=65535,
    )  # in rpm

    @computed_field(alias="baseMechSpeed_DU")  # type: ignore[misc]
    @property
    def base_mech_speed_DU(self) -> int:
        return self.base_mech_speed

    # PWM frequency in kHz
    frequency: int = Field(
        alias="frequency",
        ge=1,
        le=20,
    )  # in kHz

    # Factor between PWM timebase and fast timebase. Current control loop is
    # executed in fast timebase.
    current_loop_factor: int = Field(
        alias="currentLoopFactor",
        ge=1,
        le=25,
    )

    @computed_field(alias="currentLoopFactor_DU")  # type: ignore[misc]
    @property
    def current_loop_factor_DU(self) -> int:
        return self.current_loop_factor

    # Factor between fast timebase and mid timebase. Speed control loop is
    # executed in mid timebase.
    speed_loop_factor: int = Field(
        alias="speedLoopFactor",
        ge=1,
        le=25,
    )

    # Use default parameter structs for all components which are filled by
    # ConfigWizard generated defines
    use_default_param_struct: int = Field(
        alias="useDefaultParamStruct",
        ge=0,
        le=1,
    )

    @computed_field(alias="useDefaultParamStruct_DU")  # type: ignore[misc]
    @property
    def use_default_param_struct_DU(self) -> int:
        return self.use_default_param_struct
