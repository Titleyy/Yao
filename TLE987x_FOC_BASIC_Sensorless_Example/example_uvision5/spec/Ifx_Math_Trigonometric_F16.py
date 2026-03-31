"""Ifx_Math_Trigonometric_F16 module."""  # noqa: INP001

from enum import IntEnum

from component_config.cxml import cxml_dataclass
from pydantic import Field, computed_field


class TableSize(IntEnum):
    """Table size."""

    EightBits = 8
    TenBits = 10
    TwelveBits = 12


@cxml_dataclass
class Ifx_Math_Trigonometric_F16:  # noqa: N801
    """Ifx_Math_Trigonometric_F16 class."""

    # Sine table size
    sine_table_size: TableSize = Field(
        alias="sinLutSize",
    )

    @computed_field(alias="sinLutSize_DU")  # type: ignore[misc]
    @property
    def sine_table_size_DU(self) -> TableSize:
        return self.sine_table_size

    # Atan table size
    atan_table_size: TableSize = Field(
        alias="atanLutSize",
    )

    @computed_field(alias="atanLutSize_DU")  # type: ignore[misc]
    @property
    def atan_table_size_DU(self) -> TableSize:
        return self.atan_table_size
