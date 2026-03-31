"""Module defining application base values."""  # noqa: INP001

import math

from component_config.cxml import cxml_dataclass
from pydantic import Field, computed_field


@cxml_dataclass
class BaseValues:
    """Application base values."""

    # System base voltage
    base_voltage: float = Field(
        alias="baseVoltage",
        ge=0.001,
        le=10000.0,
    )  # in V

    # System base current
    base_current: float = Field(
        alias="baseCurrent",
        ge=0.001,
        le=10000.0,
    )  # in A

    # System base mechanical rotational speed unit for normalization of
    # mechanical speed signals.
    base_mech_speed: int = Field(
        alias="baseMechSpeed",
        ge=1,
        le=65535,
    )  # in rpm

    # This configures the pole pairs.
    pole_pairs: int = Field(
        alias="polePairs",
        ge=1,
        le=65535,
    )

    # This configures the motor phase resistance value. Calculated:
    @computed_field(alias="baseResistance")  # type: ignore[misc]
    @property
    def base_resistance(self) -> float:
        """System base resistance in Ohm.

        Returns
        -------
            float: baseresistance in Ohm

        """
        return self.base_power / (self.base_current**2)

    # Calculated:
    @computed_field(alias="baseInductance")  # type: ignore[misc]
    @property
    def base_inductance(self) -> float:
        """Base inductance in mH.

        Returns
        -------
            float: base inductance in mH

        """
        return self.base_power * self.base_time / (self.base_current**2)

    @computed_field(alias="baseFlux")  # type: ignore[misc]
    @property
    def base_flux(self) -> float:
        """Base flux in Wb.

        Returns
        -------
            float: base flux in Wb

        """
        return self.base_voltage * (self.base_time / 1000)

    @computed_field(alias="baseMechTorque")  # type: ignore[misc]
    @property
    def base_mech_torque(self) -> float:
        """Base mechanical torque in Nm.

        Returns
        -------
            float: base mechanical torque in Nm

        """
        return self.pole_pairs * self.base_power / self.base_elec_speed

    @computed_field(alias="baseElecSpeed")  # type: ignore[misc]
    @property
    def base_elec_speed(self) -> int:
        """Base electrical speed in rad/s.

        Returns
        -------
            int: base electrical speed in rad/s

        """
        return math.floor(self.base_mech_speed * self.pole_pairs * math.pi / 30)

    @computed_field(alias="baseTime")  # type: ignore[misc]
    @property
    def base_time(self) -> float:
        """Base time in ms.

        Returns
        -------
            float: base time in ms

        """
        return 1.0 / self.base_elec_speed * 1000.0

    @computed_field(alias="basePower")  # type: ignore[misc]
    @property
    def base_power(self) -> float:
        """Base power in W.

        Returns
        -------
            float: base power in W

        """
        return self.base_current * self.base_voltage

    @computed_field(alias="baseInertia")  # type: ignore[misc]
    @property
    def base_inertia(self) -> float:
        """Base inertia in mgm^2.

        Returns
        -------
            float: base inertia in mgm^2

        """
        return self.pole_pairs**2 * self.base_power * (self.base_time / 1000) ** 3 * 1000000

    @computed_field(alias="baseTorqueConstant")  # type: ignore[misc]
    @property
    def base_torque_constant(self) -> float:
        """Base torque constant in Nm/A.

        Returns
        -------
            float: base torque constant in Nm/A

        """
        return self.base_mech_torque / self.base_current

    @computed_field(alias="baseFrictionConstant")  # type: ignore[misc]
    @property
    def base_friction_constant(self) -> float:
        """Base friction constant in Nms.

        Returns
        -------
            float: base friction constant in Nms

        """
        return self.base_mech_torque / self.base_elec_speed  # TODO(mbr): why elec speed?
