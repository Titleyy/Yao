"""Ifx_MAS_Modulator_F16 module."""  # noqa: INP001

import math
from enum import IntEnum

from component_config.cxml import PDO, PDOField, cxml_dataclass
from component_config_util.qformat import Q15
from pydantic import Field, computed_field


class MeasurementPoint(IntEnum):
    """Measurement point."""

    MeasurementPointBeginning = 0
    MeasurementPointEnd = 1


class LutTableSize(IntEnum):
    """Lookup table size."""

    TenBits = 10
    TwelveBits = 12


class CurrentMeasurementTopology(IntEnum):
    """Current measurement topology."""

    SingleDCLinkShunt = 0
    ThreeLegShunt = 1


class OutBehavior(IntEnum):
    """Output behavior."""

    ActiveShortLow = 0
    ActiveShortHigh = 1
    ActiveShortHighLow = 2


class FaultReaction(IntEnum):
    """Fault reaction."""

    Disable = 0
    Enable = 1
    ReportOnly = 2
    ReportAndReact = 3


class StatusState(IntEnum):
    """Status state."""

    Init = 0
    Off = 1
    On = 2
    Fault = 3
    Brake = 4


class StatusSubState(IntEnum):
    """Status sub state."""

    BidirectionalTwoPhase = 0
    BidirectionalThreePhase = 1


@cxml_dataclass
class Ifx_MAS_Modulator_F16:  # noqa: N801
    """Ifx_MAS_Modulator_F16 component configuration."""

    def _ns_to_ticks(self, value_ns: int) -> int:
        return math.floor((self.f_sys * value_ns) / 1000)

    # Configuration of Measurement time in us. 1 tick = 1 / f_sys =
    # 1/40Mhz = 0.025us
    measurement_time: float = Field(
        alias="measurementTime",
        ge=0,
        le=5.0,
    )  # in us

    @computed_field(alias="measurementTime_DU")  # type:ignore[misc]
    @property
    def measurement_time_DU(self) -> int:
        return round(self.f_sys * self.measurement_time)

    # HighSide-LowSide switching Dead time in us. 1 tick = 1 / f_sys =
    # 1/40Mhz = 0.025us
    dead_time: float = Field(
        alias="deadTime",
        ge=0.05,
        le=2.0,
    )  # in us

    @computed_field(alias="deadTime_DU")  # type:ignore[misc]
    @property
    def dead_time_DU(self) -> int:
        return round(self.f_sys * self.dead_time)

    # Configuration of Timedelay of Powerstage Driver. 1 tick = 1 / f_sys =
    # 1/40Mhz = 0.025us
    driver_delay: float = Field(
        alias="driverDelay",
        ge=0,
        le=2.0,
    )  # in us

    @computed_field(alias="driverDelay_DU")  # type:ignore[misc]
    @property
    def driver_delay_DU(self) -> int:
        return round(self.f_sys * self.driver_delay)

    # Configuration of Current sense Ringing time in us. 1 tick = 1 / f_sys =
    # 1/40Mhz = 0.025us
    ringing_time: float = Field(
        alias="ringingTime",
        ge=0.05,
        le=2.0,
    )  # in us

    @computed_field(alias="ringingTime_DU")  # type:ignore[misc]
    @property
    def ringing_time_DU(self) -> int:
        return round(self.f_sys * self.ringing_time)

    @computed_field(alias="period")  # type:ignore[misc]
    @property
    def period(self) -> int:
        return math.floor(self.f_sys * 1000 / self.frequency)

    @computed_field(alias="period_DU")  # type:ignore[misc]
    @property
    def period_DU(self) -> int:
        return self.period

    # Current measurement point
    measurement_point: MeasurementPoint = Field(
        alias="measurementPoint",
    )

    @computed_field(alias="measurementPoint_DU")  # type:ignore[misc]
    @property
    def measurement_point_DU(self) -> MeasurementPoint:
        return self.measurement_point

    # This configures the max. output voltage of the modulator. Unit is in
    # percent of baseVoltage
    max_amplitude: float = Field(
        alias="maxAmplitude",
        ge=0.0,
        le=100.0,
    )  # in %

    @computed_field(alias="maxAmplitude_DU")  # type: ignore[misc]
    @property
    def max_amplitude_DU(self) -> int:
        return Q15.from_percent_symmetric(self.max_amplitude).stored_integer

    # High transition from bidirectional two phase to bidirectional three phase
    # shifting
    bi_directional_shifting_threshold_high: float = Field(
        alias="biDirectionalShiftingThresholdHigh",
        ge=0.0,
        le=100.0,
    )  # in %

    @computed_field(alias="biDirectionalShiftingThresholdHigh_DU")  # type: ignore[misc]
    @property
    def bi_directional_shifting_threshold_high_DU(self) -> int:
        return Q15.from_percent_symmetric(
            self.bi_directional_shifting_threshold_high
        ).stored_integer

    # Low transition from bidirectional two phase to bidirectional three phase
    # shifting
    bi_directional_shifting_threshold_low: float = Field(
        alias="biDirectionalShiftingThresholdLow",
        ge=0.0,
        # le_dyn=lambda p: p["bi_directional_shifting_threshold_high"],
        le=100.0,
    )  # in %

    @computed_field(alias="biDirectionalShiftingThresholdLow_DU")  # type: ignore[misc]
    @property
    def bi_directional_shifting_threshold_low_DU(self) -> int:
        return Q15.from_percent_symmetric(self.bi_directional_shifting_threshold_low).stored_integer

    # This configures the system clock frequency.
    f_sys: int = Field(
        alias="fSys",
        ge=1,
        le=1000,
    )  # in MHz

    # PWM frequency in kHz
    frequency: int = Field(
        alias="frequency",
        ge=1,
        le=400,
    )  # in kHz

    # Base voltage for Q15 normalization of voltage related parameters.
    base_voltage: float = Field(
        alias="baseVoltage",
        ge=0.0,
        le=10000.0,
    )  # in V

    # Min on time of PWM-pulse in ticks
    min_on_time: int = Field(
        alias="minOnTime",
        ge=0,
        le=65535,
    )  # in ticks

    @computed_field(alias="minOnTime_DU")  # type:ignore[misc]
    @property
    def min_on_time_DU(self) -> int:
        return self.min_on_time

    # lookup table size
    lut_table_size: LutTableSize = Field(
        alias="tableSin60Sqrt3LUTSize",
    )

    @computed_field(alias="tableSin60Sqrt3LUTSize_DU")  # type:ignore[misc]
    @property
    def lut_table_size_DU(self) -> LutTableSize:
        return self.lut_table_size

    # Configures whether current measurement topology is single shunt or three
    # shunt
    current_measurement_topology: CurrentMeasurementTopology = Field(
        alias="currentMeasurementTopology",
    )

    @computed_field(alias="currentMeasurementTopology_DU")  # type:ignore[misc]
    @property
    def current_measurement_topology_DU(self) -> CurrentMeasurementTopology:
        return self.current_measurement_topology

    # Stat Config for brake mode output behavior
    brake_out_behavior: OutBehavior = Field(
        alias="brakeOutBehavior",
    )

    @computed_field(alias="brakeOutBehavior_DU")  # type:ignore[misc]
    @property
    def brake_out_behavior_DU(self) -> OutBehavior:
        return self.brake_out_behavior

    # When the modulator reacts on a fault one of three fault behaviors can be
    # selected. So this fault behavior is only used in case a fault reaction is
    # set to Report and react.
    fault_out_behavior: OutBehavior = Field(
        alias="faultOutBehavior",
    )

    @computed_field(alias="faultOutBehavior_DU")  # type:ignore[misc]
    @property
    def fault_out_behavior_DU(self) -> OutBehavior:
        return self.fault_out_behavior

    # Fault reaction on max amplitude reached condition.
    fault_reaction_max_amplitude: FaultReaction = Field(
        alias="faultReactionMaxAmplitude",
    )

    @computed_field(alias="faultReactionMaxAmplitude_DU")  # type:ignore[misc]
    @property
    def fault_reaction_max_amplitude_DU(self) -> FaultReaction:
        return self.fault_reaction_max_amplitude

    # Fault reaction on overmodulation reached condition.
    fault_reaction_overmodulation: FaultReaction = Field(
        alias="faultReactionOvermodulation",
    )

    @computed_field(alias="faultReactionOvermodulation_DU")  # type:ignore[misc]
    @property
    def fault_reaction_overmodulation_DU(self) -> FaultReaction:
        return self.fault_reaction_overmodulation

    # This enables or disables a fault user callback
    enable_fault_out: bool = Field(
        alias="enableFaultOut",
    )

    @computed_field(alias="enableFaultOut_DU")  # type:ignore[misc]
    @property
    def enable_fault_out_DU(self) -> bool:
        return self.enable_fault_out

    # Fault out callback string
    fault_out: str = Field(
        default="usrCallback",
        alias="faultOut",
        component_config_meta={  # remove if fixed
            "generate_pattern": "configDefine",
            "symbol_name": "CFG_FAULT_OUT",
        },
    )  # fault out user callback

    @computed_field(alias="faultOut_DU")  # type:ignore[misc]
    @property
    def fault_out_DU(self) -> str:
        return self.fault_out

    # TODO(MBR): clarify use of callback strings in CXML
    # https://jirard.intra.infineon.com/browse/ATVMCES-3090

    # State machine state of the Modulator
    state: PDO[StatusState] = PDOField(
        alias="state",
    )

    # Sub state of the Modulator
    sub_state: PDO[StatusSubState] = PDOField(
        alias="subState",
    )

    # Error status flag. Is set when max amplitude is reached.
    error_max_amplitude_flag: PDO[bool] = PDOField(
        alias="error_maxAmplitudeFlag",
    )

    # Error status flag. Is set when over modulation is reached.
    error_overmodulation_flag: PDO[bool] = PDOField(
        alias="error_overmodulationFlag",
    )

    # The timer compare values for the CCU phase switches (HS), separated for
    # up-counting and for down-counting on each phase, total of six signals, in
    # timer ticks:
    # index 0: phase U, up-counting
    # index 1: phase V, up-counting
    # index 2: phase W, up-counting
    # index 3: phase U, down-counting
    # index 4: phase V, down-counting
    # index 5: phase W, down-counting
    compare_values: PDO[int] = PDOField(
        alias="compareValues",
        ge=0,
        le=65535,
    )  # in ticks

    # This output contains the timer values for current measurement events. For
    # single-shunt scheme, the modulator calculates the values for trigger time
    # 1 and trigger timer 2 to meet one of the following conditions:
    # - case A:
    # - trigger timer 1 is within a pulse where a phase X is active. X is any
    # phase of U, V or W.
    # - trigger timer 2 is within the pulse where two phases are active. One of
    # the two phases is phase X.
    # - case B:
    # - trigger timer 1 is within a pulse where a phase X is active. X is any
    # phase of U, V or W.
    # - trigger timer 2 is within the pulse where another phase is active.
    # The trigger timer 1 and trigger timer 2 are the tick count starting from
    # zero match.
    trigger_time: PDO[int] = PDOField(
        alias="triggerTime",
        ge=0,
        le=65535,
    )  # in ticks

    # This output represents the amplitude of the resulting voltage space vector
    # in polar coordinates. Range: [0; upper limit], where the "upper limit" is
    # the minimum of the following:
    # - Maximum amplitude
    # - The value of the reference voltage at which overmodulation occurs which
    # depends on the input dcLinkVoltage_V as follows: Uref_overmod = 0.907 * 2
    # / pi * dcLinkVoltage_V
    actual_voltage_amplitude: PDO[float] = PDOField(
        alias="actualVoltageAmplitude",
        ge_dyn=lambda p: -p["base_voltage"],
        le_dyn=lambda p: p["base_voltage"],
    )  # in V

    # This output represents the angle of the resulting voltage space vector in
    # polar coordinates. This is equal to the input reference voltage angle.
    # 0xFFFFFFFF = 2*pi = 360degree
    actual_voltage_angle: PDO[float] = PDOField(
        alias="actualVoltageAngle",
        ge=0.0,
        le=6.2832,
    )  # in rad

    # This output contains the sector number which is needed for current
    # reconstruction, with range between 0 and 5
    current_reconstruction_info_sector: PDO[int] = PDOField(
        alias="currentReconstructionInfoSector",
        ge=0,
        le=5,
    )

    # This output contains the flag to identify whether the second current
    # trigger is during the period where two phases are active (case A), during
    # the period where only one phase is active (case B)
    current_reconstruction_info_seconnd_trigger_is_sum: PDO[bool] = PDOField(
        alias="currentReconstructionInfoSecondTriggerIsSum",
    )
