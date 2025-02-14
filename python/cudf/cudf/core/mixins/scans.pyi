# Copyright (c) 2022, NVIDIA CORPORATION.

from typing import Set

class Scannable:
    _SUPPORTED_SCANS: Set

    def cumsum(self):
        ...

    def cumprod(self):
        ...

    def cummin(self):
        ...

    def cummax(self):
        ...
