"""Ifx_Math_MotorControlMath_F16 module."""  # noqa: INP001

from enum import IntEnum

from component_config.cxml import cxml_dataclass
from pydantic import Field, computed_field


class CartesianToPolarTableSize(IntEnum):
    """Cartesian to Polar table size."""

    EightBits = 8
    TenBits = 10
    TwelveBits = 12


@cxml_dataclass
class Ifx_Math_MotorControlMath_F16:  # noqa: N801
    """Ifx_Math_MotorControlMath_F16 class."""

    # Cartesian to Polar table size
    cartesian_to_polar_table_size: CartesianToPolarTableSize = Field(
        alias="cartToPolarLutSize",
    )

    @computed_field(alias="cartToPolarLutSize_DU")  # type: ignore[misc]
    @property
    def cartesian_to_polar_table_size_DU(self) -> CartesianToPolarTableSize:
        return self.cartesian_to_polar_table_size
