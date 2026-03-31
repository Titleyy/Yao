"""Ifx_MS_FocSolution_F16 module."""  # noqa: INP001

from __future__ import annotations

import math
from enum import IntEnum
from typing import Any

from component_config.cxml import PDO, PDOField, cxml_dataclass
from component_config_util.decorators import decimal_places
from component_config_util.qformat import Q15, Q15_MAX_FLOAT
from pydantic import Field, computed_field

from .base_values import BaseValues

MAX_INT32 = (2**31) - 1
MIN_INT32 = -(2**31)

Q30_MAX_FLOAT = MAX_INT32 / 2**30

SPEED_PI_DEC_PLACES = 10000


def _to_q30(value: float) -> int:
    q_value = math.floor(value * 2**30)

    if not MIN_INT32 <= q_value <= MAX_INT32:
        msg = f"Value {q_value} is not in Q30 range"
        raise ValueError(msg)

    return q_value


class State(IntEnum):
    """State of the FOC solution state machine."""

    Init = 0
    Off = 1
    StandBy = 2
    Fault = 3
    Run = 4
    RampDown = 5
    StartAngleIdent = 6


class TransitionMode(IntEnum):
    """Transition mode."""

    DirectTransition = 0
    SmoothTransition = 1


class SubState(IntEnum):
    """Sub state of the FOC solution state machine."""

    OpenLoop = 0
    TransitionUp = 1
    ClosedLoop = 2
    TransitionDown = 3


class ActualControlMode(IntEnum):
    """Actual control mode."""

    VtoF = 0
    ItoFFoc = 1


def _speed_ramp_rate_max(p: dict[str, Any]) -> float:
    return Q30_MAX_FLOAT * p["base_speed_ramp_rate"]


@cxml_dataclass
class Ifx_MS_FocSolution_F16(BaseValues):  # noqa: N801
    """Ifx_MS_FocSolution_F16 component configuration."""

    # Start-up current in % of base current.
    start_up_current: float = Field(
        alias="startUpCurrent",
        ge=0.0,
        le=100.0,
    )  # in %

    @computed_field(alias="startUpCurrent_DU")  # type: ignore[misc]
    @property
    def start_up_current_DU(self) -> int:
        return Q15.from_percent_symmetric(self.start_up_current).stored_integer

    # This configures the ramp up rate of the start-up current.
    start_up_current_ramp_up_rate: int = Field(
        alias="startUpCurrentRampUpRate",
        ge=0,
        le_dyn=lambda p: Q15_MAX_FLOAT * (p["base_current"] * 1e6) / p["speed_loop_period"],
    )  # in A/s

    @computed_field(alias="startUpCurrentRampUpRate_DU")  # type: ignore[misc]
    @property
    def start_up_current_ramp_up_rate_DU(self) -> int:
        return Q15(
            self.start_up_current_ramp_up_rate
            / ((self.base_current * 1e6) / self.speed_loop_period)
        ).stored_integer

    # Speed pi proportional gain
    speed_pi_prop_gain: float = Field(
        alias="speedPiPropGain",
        ge=0.0,
        le_dyn=lambda p: MAX_INT32 / SPEED_PI_DEC_PLACES * p["base_current"] / p["base_mech_speed"],
    )  # in A/rpm

    @computed_field(alias="speedPiPropGain_DU")  # type: ignore[misc]
    @property
    @decimal_places(4)
    def speed_pi_prop_gain_DU(self) -> float:
        return self.speed_pi_prop_gain / self.base_current * self.base_mech_speed

    # Speed pi integral gain
    speed_pi_integ_gain: float = Field(
        alias="speedPiIntegGainSamplingTime",
        ge=0.0,
        le_dyn=lambda p: MAX_INT32
        / SPEED_PI_DEC_PLACES
        * p["speed_loop_frequency"]
        * p["base_current"]
        / p["base_mech_speed"],
    )  # in A/rpms

    @computed_field(alias="speedPiIntegGainSamplingTime_DU")  # type: ignore[misc]
    @property
    @decimal_places(4)
    def speed_pi_integ_gain_DU(self) -> float:
        return (
            self.speed_pi_integ_gain
            / (self.speed_loop_frequency * self.base_current)
            * self.base_mech_speed
        )

    # Sets the output lower limit of speedPi and the refCurrent limiter in % of
    # base current
    ref_current_lower_limit: float = Field(
        alias="refCurrentLowerLimit",
        ge=-100.0,
        le=100.0,
    )  # in %

    @computed_field(alias="refCurrentLowerLimit_DU")  # type: ignore[misc]
    @property
    def ref_current_lower_limit_DU(self) -> int:
        return Q15.from_percent_symmetric(self.ref_current_lower_limit).stored_integer

    # Sets the output upper limit of speedPi and the refCurrent limiter  in % of
    # base current
    ref_current_upper_limit: float = Field(
        alias="refCurrentUpperLimit",
        ge=-100.0,
        le=100.0,
    )  # in %

    @computed_field(alias="refCurrentUpperLimit_DU")  # type: ignore[misc]
    @property
    def ref_current_upper_limit_DU(self) -> int:
        return Q15.from_percent_symmetric(self.ref_current_upper_limit).stored_integer

    # Maximum speed of accelaration limiter in % of baseMechSpeed
    ref_speed_upper_limit: float = Field(
        alias="refSpeedUpperLimit",
        # ge_dyn=lambda p: p["ref_speed_lower_limit"],
        ge=-100.0,
        le=100.0,
    )  # in %

    @computed_field(alias="refSpeedUpperLimit_DU")  # type: ignore[misc]
    @property
    def ref_speed_upper_limit_DU(self) -> int:
        return Q15.from_percent_symmetric(self.ref_speed_upper_limit).stored_integer

    # Minimum speed of accelaration limiter in % of baseMechSpeed
    ref_speed_lower_limit: float = Field(
        alias="refSpeedLowerLimit",
        ge=-100.0,
        le=100.0,
    )  # in %

    @computed_field(alias="refSpeedLowerLimit_DU")  # type: ignore[misc]
    @property
    def ref_speed_lower_limit_DU(self) -> int:
        return Q15.from_percent_symmetric(self.ref_speed_lower_limit).stored_integer

    # Speed threshold for transition from ramp down to standby
    minimum_speed_threshold: float = Field(
        alias="minimumSpeedThreshold",
        ge=0,
        le=100.0,
    )  # in %

    @computed_field(alias="minimumSpeedThreshold_DU")  # type: ignore[misc]
    @property
    def minimum_speed_threshold_DU(self) -> int:
        return Q15.from_percent_symmetric(self.minimum_speed_threshold).stored_integer

    @computed_field  # type: ignore[misc]
    @property
    def base_speed_ramp_rate(self) -> float:
        return self.base_mech_speed / (self.speed_loop_period * 1e-6)

    # This configures the ramp up rate of the acceleration limiter when the
    # control is in open loop
    speed_ramp_up_rate_open_loop: int = Field(
        alias="speedRampUpRateOpenLoop",
        ge=0,
        le_dyn=_speed_ramp_rate_max,
    )  # in rpm/s

    @computed_field(alias="speedRampUpRateOpenLoop_DU")  # type: ignore[misc]
    @property
    def speed_ramp_up_rate_open_loop_DU(self) -> int:
        return _to_q30(self.speed_ramp_up_rate_open_loop / self.base_speed_ramp_rate)

    # This configures the ramp down rate of the acceleration limiter when the
    # control is in open loop
    speed_ramp_down_rate_open_loop: int = Field(
        alias="speedRampDownRateOpenLoop",
        ge=0,
        le_dyn=_speed_ramp_rate_max,
    )  # in rpm/s

    @computed_field(alias="speedRampDownRateOpenLoop_DU")  # type: ignore[misc]
    @property
    def speed_ramp_down_rate_open_loop_DU(self) -> int:
        return _to_q30(self.speed_ramp_down_rate_open_loop / self.base_speed_ramp_rate)

    # This configures the ramp up rate of the acceleration limiter when the
    # control is in closed loop
    speed_ramp_up_rate_closed_loop: int = Field(
        alias="speedRampUpRateClosedLoop",
        ge=0,
        le_dyn=_speed_ramp_rate_max,
    )  # in rpm/s

    @computed_field(alias="speedRampUpRateClosedLoop_DU")  # type: ignore[misc]
    @property
    def speed_ramp_up_rate_closed_loop_DU(self) -> int:
        return _to_q30(self.speed_ramp_up_rate_closed_loop / self.base_speed_ramp_rate)

    # This configures the ramp down rate of the acceleration limiter when the
    # control is in closed loop
    speed_ramp_down_rate_closed_loop: int = Field(
        alias="speedRampDownRateClosedLoop",
        ge=0,
        le_dyn=_speed_ramp_rate_max,
    )  # in rpm/s

    @computed_field(alias="speedRampDownRateClosedLoop_DU")  # type: ignore[misc]
    @property
    def speed_ramp_down_rate_closed_loop_DU(self) -> int:
        return _to_q30(self.speed_ramp_down_rate_closed_loop / self.base_speed_ramp_rate)

    # This configures the Quadrature current at transition in % of baseCurrent.
    # Used in direct transition mode.
    q_current_at_transition: float = Field(
        alias="qCurrentAtTransition",
        ge=0.0,
        le=100.0,
    )  # in %

    @computed_field(alias="qCurrentAtTransition_DU")  # type: ignore[misc]
    @property
    def q_current_at_transition_DU(self) -> int:
        return Q15.from_percent_symmetric(self.q_current_at_transition).stored_integer

    # This configures the minimum angle error in degree to do the transition
    # from transition up state to close loop state.
    transition_angle_tolerance: float = Field(
        alias="transitionAngleTolerance",
        ge=0.0,
        le=90.0,
    )  # in deg

    @computed_field(alias="transitionAngleTolerance_DU")  # type: ignore[misc]
    @property
    def transition_angle_tolerance_DU(self) -> int:
        return math.floor(self.transition_angle_tolerance * 2**16 / 360)

    # This configures the maximum allowed time for the transition from
    # transition up state to closed loop state or from transition down state to
    # open loop state, transition time limit.
    transition_time_limit: int = Field(
        alias="transitionTimeLimit",
        ge_dyn=lambda p: 1 * p["speed_loop_period"] / 1e3,
        le_dyn=lambda p: 65535 * p["speed_loop_period"] / 1e3,
    )  # in ms

    @computed_field(alias="transitionTimeLimit_DU")  # type: ignore[misc]
    @property
    def transition_time_limit_DU(self) -> int:
        return round(self.transition_time_limit * 1e3 / self.speed_loop_period)

    # This configures the D current scaling factor
    transition_down_d_current_scaling: float = Field(
        alias="transitionDownDCurrentScaling",
        ge=0.0,
        le=Q15_MAX_FLOAT * 2.0,
    )

    @computed_field(alias="transitionDownDCurrentScaling_DU")  # type: ignore[misc]
    @property
    def transition_down_d_current_scaling_DU(self) -> int:
        return math.floor(self.transition_down_d_current_scaling * 2**14)

    # This configures the Transition speed up.
    transition_speed_up: float = Field(
        alias="transitionSpeedUp",
        ge=0.0,
        le=100.0,
    )  # in %

    @computed_field(alias="transitionSpeedUp_DU")  # type: ignore[misc]
    @property
    def transition_speed_up_DU(self) -> int:
        return Q15.from_percent_symmetric(self.transition_speed_up).stored_integer

    # This configures the Transition speed band.
    transition_speed_band: float = Field(
        alias="transitionSpeedBand",
        ge=0.0,
        # le_dyn=lambda p: p["transition_speed_up"],
        le=100.0,
    )  # in %

    @computed_field(alias="transitionSpeedBand_DU")  # type: ignore[misc]
    @property
    def transition_speed_band_DU(self) -> int:
        return Q15.from_percent_symmetric(self.transition_speed_band).stored_integer

    @computed_field(alias="inverseTorqueConstant_DU")  # type: ignore[misc]
    @property
    def inverse_torque_constant_DU(self) -> int:
        return math.floor(
            self.base_torque_constant / self.torque_constant * 2**15
        )  # TODO(mbr): exceeds Q15 value by design
        return Q15(self.base_torque_constant / self.torque_constant).stored_integer

    # Viscous friction constant for pre control
    friction_constant: float = Field(
        alias="frictionConstant",
        ge=0.0,
        le_dyn=lambda p: Q15_MAX_FLOAT * 1000 * p["base_friction_constant"],
    )  # in mNms

    @computed_field(alias="frictionConstant_DU")  # type: ignore[misc]
    @property
    def friction_constant_DU(self) -> int:
        return Q15(self.friction_constant / (1000 * self.base_friction_constant)).stored_integer

    @computed_field(alias="rotorInertiaOverSamplingTime_DU")  # type: ignore[misc]
    @property
    @decimal_places(4)
    def rotor_inertia_over_sampling_time_DU(self) -> float:
        return (self.rotor_mech_inertia / self.speed_loop_period) / (
            self.base_inertia / (self.base_time * 1000)
        )

    # Initial Start-up current in % of base current. Value must be <=
    # startup_current
    init_startup_current: float = Field(
        alias="initStartupCurrent",
        ge=-100.0,
        le=100.0,
    )  # in %

    @computed_field(alias="initStartupCurrent_DU")  # type: ignore[misc]
    @property
    def init_startup_current_DU(self) -> int:
        return Q15.from_percent_symmetric(self.init_startup_current).stored_integer

    # Speed pi Anti-windup gain
    speed_pi_anti_windup_gain: float = Field(
        alias="speedPiAntiWindupGain",
        ge=0.0,
        le_dyn=lambda p: MAX_INT32
        * 1e6  # convert period from us to s
        / p["speed_loop_period"]
        / p["base_mech_speed"]
        / SPEED_PI_DEC_PLACES,
    )  # in 1/rpms

    @computed_field(alias="speedPiAntiWindupGain_DU")  # type: ignore[misc]
    @property
    @decimal_places(4)
    def speed_pi_anti_windup_gain_DU(self) -> float:
        return (
            self.speed_pi_anti_windup_gain * (self.speed_loop_period * 1e-6) * self.base_mech_speed
        )

    # This is the system clock frequency. The value is used from
    # IFX_MHA_DRIVERGROUP_F16_CFG.PWM_CLOCK_FREQ_MHZ
    f_sys: int = Field(
        alias="fSys",
        ge=1,
        le=1000,
    )  # in MHz

    # Phase resistance
    phase_res: float = Field(
        alias="phaseRes",
        ge=0.0,
        le_dyn=lambda p: Q15_MAX_FLOAT * p["base_resistance"],
    )  # in Ohm

    # Motor D-Axis Inductance / Direct phase inductance
    direct_phase_ind: float = Field(
        alias="directPhaseInd",
        ge=0.0,
        le_dyn=lambda p: Q15_MAX_FLOAT * p["base_inductance"],
    )  # in mH

    # Motor Q-Axis inductance / Quadrature phase inductance
    quadrature_phase_ind: float = Field(
        alias="quadraturePhaseInd",
        ge=0.0,
        le_dyn=lambda p: Q15_MAX_FLOAT * p["base_inductance"],
    )  # in mH

    @computed_field(alias="averageInductance")  # type: ignore[misc]
    @property
    def average_inductance(self) -> float:
        """Average inductance in mH."""
        return (self.direct_phase_ind + self.quadrature_phase_ind) / 2

    # Rotor inertia
    rotor_mech_inertia: float = Field(
        alias="rotorMechInertia",
        ge=0.0,
        le=100.0,
    )  # in mgm^2

    torque_constant: float = Field(
        alias="torqueConstant",
        ge=0.0,
        le=100.0,
    )  # in Nm/A

    frequency: int = Field(
        alias="frequency",
        ge=0,
        le=400,
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

    @computed_field(alias="currentLoopFrequency")  # type: ignore[misc]
    @property
    def current_loop_frequency(self) -> int:
        """Current loop frequency in Hz."""
        return math.floor(self.frequency * 1e3 / self.current_loop_factor)

    @computed_field(alias="currentLoopPeriod")  # type: ignore[misc]
    @property
    def current_loop_period(self) -> int:
        """Current loop period in us."""
        return math.floor(1e6 / self.current_loop_frequency)

    # Factor between fast timebase and mid timebase. Speed control loop is
    # executed in mid timebase.
    speed_loop_factor: int = Field(
        alias="speedLoopFactor",
        ge=1,
        le=25,
    )

    @computed_field(alias="speedLoopFrequency")  # type: ignore[misc]
    @property
    def speed_loop_frequency(self) -> int:
        """Speed loop frequency in Hz."""
        return math.floor(self.current_loop_frequency / self.speed_loop_factor)

    @computed_field(alias="speedLoopPeriod")  # type: ignore[misc]
    @property
    def speed_loop_period(self) -> int:
        """Speed loop period in us."""
        return math.floor(1e6 / self.speed_loop_frequency)

    @computed_field(alias="speedLoopPeriod_DU")  # type: ignore[misc]
    @property
    def speed_loop_period_DU(self) -> int:
        return self.speed_loop_period

    @computed_field(alias="samplingTime")  # type: ignore[misc]
    @property
    def sampling_time(self) -> int:
        """Sampling time in us."""
        return math.floor(1e6 / self.current_loop_frequency)

    # HighSide-LowSide switching dead-time in ticks.1 tick = 1 / f_sys =
    # 1/40Mhz = 0.025 us
    dead_time: float = Field(
        alias="deadTime",
        ge=0.05,
        le=2.0,
    )  # in us

    currctrl_adjustm_factor: float = Field(
        alias="currctrlAdjustmFactor",
        ge=0.01,
        le=1.0,
    )

    @computed_field(alias="currPiPropGain")  # type: ignore[misc]
    @property
    def curr_pi_prop_gain(self) -> float:
        """Current PI proportional gain."""
        return (
            self.currctrl_adjustm_factor
            * (self.average_inductance / 1000)
            * self.current_loop_frequency
            / 4
        )

    @computed_field(alias="currPiIntegGain")  # type: ignore[misc]
    @property
    def curr_pi_integ_gain(self) -> float:
        """Current PI integral gain."""
        return self.currctrl_adjustm_factor * self.phase_res * self.current_loop_frequency / 4

    # Transition mode
    transition_mode: TransitionMode = Field(
        alias="transitionMode",
    )

    @computed_field(alias="transitionMode_DU")  # type: ignore[misc]
    @property
    def transition_mode_DU(self) -> int:
        return self.transition_mode

    # Enable feature start angle identification. Config define
    include_start_angle_ident: int = Field(
        alias="includeStartangleIdent",
        ge=0,
        le=1,
    )

    @computed_field(alias="includeStartangleIdent_DU")  # type: ignore[misc]
    @property
    def include_start_angle_ident_DU(self) -> int:
        return self.include_start_angle_ident

    # This monitoring output represents the state of the FOC solution state
    # machine
    state: PDO[State] = PDOField(
        alias="state",
    )

    # This monitoring output represents the substate of the FOC solution state
    # machine
    sub_state: PDO[SubState] = PDOField(
        alias="subState",
    )

    # This monitoring output represents control mode
    actual_control_mode: PDO[ActualControlMode] = PDOField(
        alias="actualControlMode",
    )

    # This output shows the current estimated speed
    # from the flux estimator.
    estimated_speed: PDO[int] = PDOField(
        alias="estimatedSpeed",
        ge_dyn=lambda p: -p["base_mech_speed"],
        le_dyn=lambda p: p["base_mech_speed"],
    )  # in rpm
