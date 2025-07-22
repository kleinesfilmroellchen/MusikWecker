from math import floor, log2
from dataclasses import dataclass, field


@dataclass
class BitStream:
    data: bytearray = field(default_factory=bytearray)
    next_index: int = field(default=0, init=False)
    current_bit: int = field(default=0, init=False)
    current_byte: int = field(default=0, init=False)

    # def read_bit(self) -> int:
    #     if self.current_bit == 0:
    #         self.current_byte = self.data[self.next_index]
    #         self.next_index += 1
    #     return 1 if self.current_byte & (1 << self.current_bit) > 0 else 0

    # def read_bits(self, count: int) -> int:
    #     output = 0
    #     for _ in range(count):
    #         bit = self.read_bit()
    #         output = (output << 1) | bit
    #     return output

    def write_bits(self, value: int, count: int):
        bit_count = count
        while bit_count > 0:
            next_bit = (value >> (bit_count - 1)) & 1
            bit_count -= 1

            self.current_byte <<= 1
            self.current_byte |= next_bit
            self.current_bit += 1

            if self.current_bit > 7:
                self.data.append(self.current_byte)
                self.current_bit = 0
                self.current_byte = 0

    def write_to_byte_boundary(self):
        if self.current_bit == 0:
            return
        self.write_bits(0, 8 - self.current_bit)

    def write_rice(self, value: int, k: int) -> int:
        """
        Barring the zig-zag encoding, this is basically:
        https://github.com/xiph/flac/blob/28e4f0528c76b296c561e922ba67d43751990599/src/libFLAC/bitwriter.c#L727
        """
        msbs = value >> k
        pattern = 1 << k
        pattern |= value & (pattern - 1)
        self.write_bits(0, msbs)
        self.write_bits(pattern, k + 1)
        return msbs

    def write_exponential_golomb(self, value: int):
        """Rice order 0"""
        leading_bits = int(floor(log2(value)) + 1)
        self.write_bits(0, leading_bits - 1)
        self.write_bits(value, leading_bits)


def encode_rice(data: list[int]) -> tuple[bytes, int]:
    """
    Rice / Exponential Golomb coding for variable length integers.
    Adaptive k algorithm based on https://ieeexplore.ieee.org/document/4362102,
    a variant of the Melcode algorithm: https://ieeexplore.ieee.org/document/855427
    """
    M = 7

    shortest_data = bytes()
    shortest_len = 100000000000
    shortest_k = 0

    # for k in range(1, 10):
    k = 4
    stream = BitStream()
    counter = 0
    for value in data:
        u = stream.write_rice(value, k)
        # if u > 1:
        #     counter += 1
        # if u > 2:
        #     counter += 1
        # if u > 3:
        #     counter += 1
        # if u > 4:
        #     counter += 1
        # if u == 0 and (counter & (1 << k)) == 0:
        #     counter -= 1
        # if counter >= M:
        #     k += 1
        #     counter = 0
        # if counter <= -M:
        #     k -= 1
        #     counter = 0
    stream.write_to_byte_boundary()
    # if len(stream.data) < shortest_len:
    #     shortest_data = stream.data
    #     shortest_len = len(shortest_data)
    #     shortest_k = k

    return (stream.data, shortest_k)


def encode_leb(data: list[int]) -> bytes:
    """LEB128 encoding for variable length integers."""
    output = bytearray()
    for value in data:
        if value == 0:
            output.append(0)
        while value != 0:
            byte = value & 0x7F
            value >>= 7
            if value != 0:
                byte &= 0x80
            output.append(byte)
    return output
