"""Ifx_MDA_VToFController_F16 module."""  # noqa: INP001

import math

from component_config.cxml import PDO, PDOField, cxml_dataclass
from component_config_util.qformat import Q15
from pydantic import Field, computed_field


@cxml_dataclass
class Ifx_MDA_VToFController_F16:
    @computed_field(alias="angleIncrement_DU")  # type: ignore[misc]
    @property
    def angle_increment_DU(self) -> int:
        return math.floor(self.sampling_time * 1e-6 * self.base_elec_speed * 2**14 / (2 * math.pi))

    # This configures the rated electrical speed in % of base_elec_speed
    look_up_table_vtof_rated_speed: float = Field(
        alias="ratedSpeed",
        ge=0.0,
        # ge_dyn=lambda p: p["look_up_table_vtof_corner_speed"],
        le=100.0,
    )  # in %

    @computed_field(alias="ratedSpeed_DU")  # type: ignore[misc]
    @property
    def look_up_table_vtof_rated_speed_DU(self) -> int:
        return Q15.from_percent_symmetric(self.look_up_table_vtof_rated_speed).stored_integer

    # This configures the electrical speed at the corner point of the LUT in %
    # of base_elec_speed
    look_up_table_vtof_corner_speed: float = Field(
        alias=("cornerSpeed"),
        ge=0.0,
        le=100.0,
        # le_dyn=lambda p: p["look_up_table_vtof_rated_speed"],
    )  # in %

    @computed_field(alias="cornerSpeed_DU")  # type: ignore[misc]
    @property
    def look_up_table_vtof_corner_speed_DU(self) -> int:
        return Q15.from_percent_symmetric(self.look_up_table_vtof_corner_speed).stored_integer

    # This configures the voltage of the motor, reached at motor rated speed in
    # % of base voltage
    look_up_table_vtof_rated_volt: float = Field(
        alias="ratedVolt",
        # ge_dyn=lambda p: p["look_up_table_vtof_corner_volt"],
        ge=0.0,
        le=100.0,
    )  # in %

    @computed_field(alias="ratedVolt_DU")  # type: ignore[misc]
    @property
    def look_up_table_vtof_rated_volt_DU(self) -> int:
        return Q15.from_percent_symmetric(self.look_up_table_vtof_rated_volt).stored_integer

    # This configures the voltage at the corner point of the LUT in % of base
    # voltage
    look_up_table_vtof_corner_volt: float = Field(
        alias="cornerVolt",
        ge=0.0,
        le=100.0,
        # ge_dyn=lambda p: p["look_up_table_vtof_min_volt"],
        # le_dyn=lambda p: p["look_up_table_vtof_rated_volt"],
    )  # in %

    @computed_field(alias="cornerVolt_DU")  # type: ignore[misc]
    @property
    def look_up_table_vtof_corner_volt_DU(self) -> int:
        return Q15.from_percent_symmetric(self.look_up_table_vtof_corner_volt).stored_integer

    # This configures the voltage at the minimum motor speed in % of base voltage
    look_up_table_vtof_min_volt: float = Field(
        alias="minVolt",
        ge=0.0,
        # le_dyn=lambda p: p["look_up_table_vtof_rated_volt"],
        le=100.0,
    )  # in %

    @computed_field(alias="minVolt_DU")  # type: ignore[misc]
    @property
    def look_up_table_vtof_min_volt_DU(self) -> int:
        return Q15.from_percent_symmetric(self.look_up_table_vtof_min_volt).stored_integer

    # Base electrical speed  for Q15 normalization of electrical speed related
    # parameters.
    base_elec_speed: int = Field(
        alias="baseElecSpeed",
        ge=0,
        le=65535,
    )  # in rad/s

    @computed_field(alias="baseElecSpeed_DU")  # type: ignore[misc]
    @property
    def base_elec_speed_DU(self) -> int:
        return self.base_elec_speed

    # This configures the period of time in which the VToF controller is
    # executed.
    sampling_time: int = Field(
        alias="samplingTime",
        ge=1,
        le=65535,
    )  # in us

    # Amplitude of the output voltage vector in % of base voltage
    output_voltage_vector_amplitude: PDO[float] = PDOField(
        alias="voltageVectorAmplitude",
        ge=-100.0,
        le=100.0,
    )  # in %

    # Angle of the output voltage vector. 0xFFFFFFFF = 2*pi = 360degree
    output_voltage_vector_angle: PDO[float] = PDOField(
        alias="voltageVectorAngle",
        ge=0.0,
        le=6.2832,
    )  # in rad
