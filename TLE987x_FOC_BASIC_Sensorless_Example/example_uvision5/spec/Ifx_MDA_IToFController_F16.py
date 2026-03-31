"""Ifx_MDA_IToFController_F16 module."""  # noqa: INP001

import math
import warnings

from component_config.cxml import PDO, PDOField, cxml_dataclass
from pydantic import Field, computed_field

MAX_INT16 = 2**15 - 1


@cxml_dataclass
class Ifx_MDA_IToFController_F16:
    @computed_field(alias="angleIncrement_DU")  # type: ignore[misc]
    @property
    def angle_increment_DU(self) -> int:
        """Angle increment factor as Q14.

        Returns
        -------
            int: angle increment factor as Q14

        """
        result = math.floor(
            self.sampling_time * 1e-6 * self.base_elec_speed * 2**14 / (2 * math.pi)
        )

        if result > MAX_INT16:
            warnings.warn(
                "angle_increment_DU exceeds MAX_INT16. angle_increment_DU = sampling_time * 1e-6 * base_elec_speed * 2**14 / (2 * pi) ",
                stacklevel=2,
            )
            return MAX_INT16

        return result

    # Sampling time
    sampling_time: int = Field(
        alias="samplingTime",
        ge=1,
        le=65535,
    )  # in us

    # Base electrical speed for Q15 normalization of electrical speed related
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

    # Angle of the output current vector. 0xFFFFFFFF = 2*pi = 360degree
    current_vec_angle: PDO[float] = PDOField(
        alias="currentVecAngle",
        ge=0.0,
        le=6.2832,
    )  # in rad
