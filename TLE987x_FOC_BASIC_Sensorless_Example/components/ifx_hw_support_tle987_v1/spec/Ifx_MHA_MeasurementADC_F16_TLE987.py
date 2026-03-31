"""Ifx_MHA_MeasurementADC_F16_TLE987 module."""  # noqa: INP001

from enum import IntEnum

from component_config.cxml import PDO, PDOField, cxml_dataclass
from component_config_util.qformat import convert_to_q15_and_upshift
from pydantic import Field, computed_field


class CsaGain(IntEnum):
    """Current sense amplifier gain."""

    G10 = 0
    G20 = 1
    G40 = 2
    G60 = 3

    @property
    def gain(self) -> float:
        """Get the gain value of the current sense amplifier."""
        match self:
            case CsaGain.G10:
                return 10
            case CsaGain.G20:
                return 20
            case CsaGain.G40:
                return 40
            case CsaGain.G60:
                return 60


class VmonSenSelInrange(IntEnum):
    """VDH Monitoring Input Attenuator Select Input Range."""

    ZeroToTwentyVolt = 0
    ZeroToThirtyVolt = 1


class StatusState(IntEnum):
    """State machine state of the Measurement ADC."""

    Init = 0
    Off = 1
    On = 2
    Calibration = 3


def _adc_result_shift(up_shift: int) -> int:
    """Calculate remaining shift for ADC results.

    10 bit ADC results are stored in a 12 bit register
    OUT_CH0[11:2] = result[9:0]. OUT_CH0[1:0] are padded with “00”.
    Refer to  user manual TLE987x page 814

    Args:
    ----
        up_shift (int): up_shift from Q15

    Returns:
    -------
        int: final adc_result_shift

    """
    return (15 - up_shift) - (15 - 12)


@cxml_dataclass
class Ifx_MHA_MeasurementADC_F16_TLE987:  # noqa: N801
    """Ifx_MHA_MeasurementADC_F16_TLE987 component configuration."""

    # Current sense amplifier gain
    csa_gain: CsaGain = Field(
        alias="csaGain",
    )

    @computed_field(alias="csaGain_DU")  # type: ignore[misc]
    @property
    def csa_gain_DU(self) -> int:
        return self.csa_gain

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

    # Shunt resistance of current sense single shunt
    shunt_res: float = Field(
        alias="shuntRes",
        ge=0.0001,
        le=0.1,
    )  # in Ohm

    # Current sense amplifier offset calibration cycles
    calibration_cycles: int = Field(
        alias="calibrationCycles",
        ge=1,
        le=256,
    )  # in counts

    @computed_field(alias="calibrationCycles_DU")  # type: ignore[misc]
    @property
    def calibration_cycles_DU(self) -> int:
        return self.calibration_cycles

    @property
    def _attenuator_range(self) -> float:
        return 22.3214 if self.vmon_sen_sel_inrange == VmonSenSelInrange.ZeroToTwentyVolt else 30

    @property
    def _vdc_real(self) -> float:
        return self._attenuator_range / self.base_voltage

    @computed_field(alias="convertVdcToQ15_DU")  # type: ignore[misc]
    @property
    def vdc_to_q15_DU(self) -> int:
        """DC link voltage conversion factor.

         Factor and shift are used to convert the voltage into Q15 value.

        Returns
        -------
            int: dc link voltage conversion factor

        """
        return convert_to_q15_and_upshift(self._vdc_real)[0].stored_integer

    @computed_field(alias="vdcBits_DU")  # type: ignore[misc]
    @property
    def vdc_bits_DU(self) -> int:
        """DC link voltage conversion shift.

        Factor and shift are used to convert the voltage into Q15 value.

        Returns
        -------
            int: vdc bits

        """
        return _adc_result_shift(convert_to_q15_and_upshift(self._vdc_real)[1])

    def _calc_current_gain(self, csa_gain: CsaGain) -> float:
        return 5 / (self.base_current * csa_gain.gain * self.shunt_res)

    @computed_field(alias="currentGainQ10_DU")  # type: ignore[misc]
    @property
    def current_gain_q_10_DU(self) -> int:
        return convert_to_q15_and_upshift(self._calc_current_gain(CsaGain.G10))[0].stored_integer

    @computed_field(alias="currentGainQ20_DU")  # type: ignore[misc]
    @property
    def current_gain_q_20_DU(self) -> int:
        return convert_to_q15_and_upshift(self._calc_current_gain(CsaGain.G20))[0].stored_integer

    @computed_field(alias="currentGainQ40_DU")  # type: ignore[misc]
    @property
    def current_gain_q_40_DU(self) -> int:
        return convert_to_q15_and_upshift(self._calc_current_gain(CsaGain.G40))[0].stored_integer

    @computed_field(alias="currentGainQ60_DU")  # type: ignore[misc]
    @property
    def current_gain_q_60_DU(self) -> int:
        return convert_to_q15_and_upshift(self._calc_current_gain(CsaGain.G60))[0].stored_integer

    @computed_field(alias="currentGainShift10_DU")  # type: ignore[misc]
    @property
    def current_gain_shift_10_DU(self) -> int:
        return _adc_result_shift(
            convert_to_q15_and_upshift(self._calc_current_gain(CsaGain.G10))[1]
        )

    @computed_field(alias="currentGainShift20_DU")  # type: ignore[misc]
    @property
    def current_gain_shift_20_DU(self) -> int:
        return _adc_result_shift(
            convert_to_q15_and_upshift(self._calc_current_gain(CsaGain.G20))[1]
        )

    @computed_field(alias="currentGainShift40_DU")  # type: ignore[misc]
    @property
    def current_gain_shift_40_DU(self) -> int:
        return _adc_result_shift(
            convert_to_q15_and_upshift(self._calc_current_gain(CsaGain.G40))[1]
        )

    @computed_field(alias="currentGainShift60_DU")  # type: ignore[misc]
    @property
    def current_gain_shift_60_DU(self) -> int:
        return _adc_result_shift(
            convert_to_q15_and_upshift(self._calc_current_gain(CsaGain.G60))[1]
        )

    # VDH Monitoring Input Attenuator Select Input Range
    vmon_sen_sel_inrange: VmonSenSelInrange = Field(
        alias="vmonSenSelInrange",
    )

    @computed_field(alias="vmonSenSelInrange_DU")  # type: ignore[misc]
    @property
    def vmon_sen_sel_inrange_DU(self) -> VmonSenSelInrange:
        return self.vmon_sen_sel_inrange

    # Current sense amplifier offset. An offset calibration is performed always
    # between states off and on (When powerstage is enabled). To measure the
    # offset again the parameter calibrateCSA can be set to 1. This will trigger
    # a calibration sequence.
    offset: PDO[int] = PDOField(
        alias="offset",
        ge=0,
        le=16384,
    )

    # Current sense amplifier offset calibration. Setting a 1 will start a
    # calibration sequence. Calibration end is shown on monitoring output state.
    # It will leave state calibration and enter state on.
    calibrate_csa: PDO[int] = PDOField(
        alias="calibrateCSA",
        ge=0,
        le=16384,
    )

    # State machine state of the Measurement ADC.
    state: PDO[StatusState] = PDOField(
        alias="state",
    )

    # This is the first of two measured shunt currents from single shunt
    # topology.
    shunt_currents_0: PDO[float] = PDOField(
        alias="shuntCurrents_0",
        ge_dyn=lambda p: -p["base_current"],
        le_dyn=lambda p: p["base_current"],
    )  # in A

    # This is the second of two measured shunt currents from single shunt
    # topology.
    shunt_currents_1: PDO[float] = PDOField(
        alias="shuntCurrents_1",
        ge_dyn=lambda p: -p["base_current"],
        le_dyn=lambda p: p["base_current"],
    )  # in A

    # The measured voltage at the DC link capacitors.
    dc_link_voltage: PDO[float] = PDOField(
        alias="dcLinkVoltage",
        ge_dyn=lambda p: -p["base_voltage"],
        le_dyn=lambda p: p["base_voltage"],
    )  # in V
