import argparse
from enum import Enum
from itertools import batched, chain, product
from math import ceil, floor, log2
from pathlib import Path
import cv2
from PIL import Image
from io import BytesIO
import struct
from typing import Iterable, Tuple, TypeVar
from dataclasses import dataclass, field


target_height = 64
fps = 8
ms_per_frame = (1 / fps) * 1000

x_block_size = 80
y_block_size = 64


def reverse_mask(x):
    x = ((x & 0x55555555) << 1) | ((x & 0xAAAAAAAA) >> 1)
    x = ((x & 0x33333333) << 2) | ((x & 0xCCCCCCCC) >> 2)
    x = ((x & 0x0F0F0F0F) << 4) | ((x & 0xF0F0F0F0) >> 4)
    # x = ((x & 0x00FF00FF) << 8) | ((x & 0xFF00FF00) >> 8)
    # x = ((x & 0x0000FFFF) << 16) | ((x & 0xFFFF0000) >> 16)
    return x


def bytes_to_c_array(data: bytes, name: str) -> str:
    return f"static unsigned char {name}[] PROGMEM = {{ {', '.join(f'{b:#04x}' for b in data)} }};"


def strip_tiff(data: bytes) -> bytes:
    # All we need to strip the TIFF-specific data and leave us with CCITT Group 4 encoded data:
    # - TIFF header occupies the first 8 bytes.
    # - The IFD pointer lives at bytes 4-7 (little endian).
    # - IFD is the last data in the file; everything between header and IFD is image data.
    ifd_position, *_ = struct.unpack("<I", data[4:8])
    return data[8:ifd_position]


def encode_interframe_delta(current_frame: bytes, previous_frame: bytes) -> bytes:
    """
    Delta encode consecutive frames.
    Really, this is not delta encoding, but XOR encoding, since we have single bits.
    Due to XOR's symmetry, decoding is also possible via XOR again.
    """
    return bytes(a ^ b for a, b in zip(current_frame, previous_frame))


def compress_into_nibbles(data: bytes) -> bytes:
    """
    Compresses the RLE data further by combining small nibbles.
    This stems from the observation that many run lengths are less than 8 in size,
    so two nibbles can be combined into one,
    using the MSBit as a signal as to whether this is one 8-bit (1) or two 4-bit (0) numbers.
    """
    output = bytearray()
    full_byte_marker = 0x80
    top_threshold = 0b0111
    bottom_threshold = 0b1111

    def append_byte(byte: int):
        output.append(byte | full_byte_marker)

    def append_nibbles(a, b):
        output.append((a << 4) | b)

    index = 0
    while index < len(data) - 1:
        a, b = data[index], data[index + 1]
        if a > top_threshold:
            append_byte(a)
            index += 1
        # minor optimization to skip over b in the next loop
        elif b > bottom_threshold:
            append_byte(a)
            append_byte(b)
            index += 2
        else:
            append_nibbles(a, b)
            index += 2

    # last byte wasn't consumed in a two-number byte
    if index < len(data):
        append_byte(data[-1])

    return output


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


def encode_rle(data: bytes) -> bytes:
    """
    Simple Run Length Encoding (RLE) scheme for compressing black and white images.
    We make use of the fact that most images have long stretches of white or black on one line.
    The data consists of single bytes encoding alternating runs.
    The byte's numeric (unsigned) value corresponds with the length of the run.
    The first run is black, the second white, etc.
    Zero-length runs are necessary to encode runs of the same color that go over 255 pixels.
    The last run's color is extended to the end of the image, if necessary.
    For instance, an all-black image can be encoded with 0b01, and an all-white image with 0b00 0b01.
    To decode such "under-filled" images correctly, separate knowledge of image dimensions is necessary.
    """
    output = bytearray()
    current_run_color = 0
    current_run_length = 0
    # when using nibble compression, leave the topmost bit for the byte/nibble signal
    limit = 0x7F

    def terminate_run():
        nonlocal current_run_length, current_run_color
        while current_run_length > limit:
            output.append(limit)
            output.append(0x00)
            current_run_length -= limit
        output.append(current_run_length)
        current_run_length = 0
        current_run_color = 1 - current_run_color

    for byte in data:
        for bit in range(8):
            pixel_value = 1 if (byte & (1 << bit)) >= 1 else 0
            if pixel_value != current_run_color:
                terminate_run()
            current_run_length += 1

    if current_run_length > 0:
        # last run doesn't need to be encoded in multiple bytes due to the extension rule
        current_run_length = min(current_run_length, limit)
        terminate_run()

    return output


def encode_rle_unbounded(data: bytes) -> list[int]:
    """
    Same Run Length Encoding (RLE) scheme for compressing black and white images as above.
    However, runs have unbounded lengths, so that they may be encoded as variable-length integers.
    """
    output = list()
    current_run_color = 0
    current_run_length = 0

    def terminate_run():
        nonlocal current_run_length, current_run_color
        output.append(current_run_length)
        current_run_length = 0
        current_run_color = 1 - current_run_color

    for byte in data:
        for bit in range(8):
            pixel_value = 1 if (byte & (1 << bit)) >= 1 else 0
            if pixel_value != current_run_color:
                terminate_run()
            current_run_length += 1

    if current_run_length > 0:
        # last run doesn't need to be encoded in multiple bytes due to the extension rule
        current_run_length = min(current_run_length, 1)
        terminate_run()

    return output


class PacketType(Enum):
    RLE = 1
    Data = 0


def encode_pokémon(data: bytes) -> tuple[bytes, PacketType]:
    """
    Compression scheme almost identical to Pokémon Red/Blue/Yellow's compression for Pokémon battle graphics.
    First, perform delta coding with the next pixel to yield many black pixels and a few white ones.
    Pixels are split up into two-bit pairs.
    Whenever there's a series of more than two pixels of black, they're encoded as an Exponential Golomb coded number.
    Otherwise, raw data bit pairs are used, until 00 indicates the end of the raw data.
    After that, another RLE packet follows, and so on.
    Whether the first packet is RLE or data will be encoded in the format itself,
    so the returned boolean indicates whether this was RLE (True) or data (False).
    """
    previous_pixel = 0
    # relatively inefficient storage, but we need to operate on the pixels again anyways so it doesn't matter
    # the decoder will do the two steps combined to save memory.
    deltas: list[int] = []
    for byte in data:
        for bit in range(8):
            pixel_value = 1 if (byte & (1 << bit)) >= 1 else 0
            delta = previous_pixel ^ pixel_value
            deltas.append(delta)
            previous_pixel = pixel_value

    output_stream = BitStream()
    first_packet_type: None | PacketType = None
    i = 0
    bit_pairs = list(batched(deltas, 2))
    while i < len(bit_pairs):
        packet = bit_pairs[i]
        i += 1
        match packet:
            # encode as much data as possible using RLE
            case (0, 0):
                if first_packet_type is None:
                    first_packet_type = PacketType.RLE
                else:
                    # terminate data packet
                    output_stream.write_bits(0, 2)

                bit_count = 1
                while i < len(bit_pairs) and bit_pairs[i] == (0, 0):
                    i += 1
                    bit_count += 1
                output_stream.write_exponential_golomb(bit_count)

            case (bit0, bit1):
                if first_packet_type is None:
                    first_packet_type = PacketType.Data
                output_stream.write_bits((bit0 << 1) | bit1, 2)

    output_stream.write_to_byte_boundary()
    # print([bin(x) for x in output_stream.data])
    return (output_stream.data, first_packet_type)


def encode_garbagémon(data: bytes) -> bytes:
    """
    Compression scheme loosely inspired by Pokémon Red/Blue/Yellow's compression for Pokémon battle graphics.
    First, perform delta coding with the next pixel to yield many black pixels and a few white ones.
    Pixels are encoded as either (1) a black run from 1-128 (bias of 1, marker bit 1)
    or (0) seven raw pixels (marker bit 0).
    The "fill with last color" rule applies here as well, except the fill color is always black.
    """
    previous_pixel = 0
    # relatively inefficient storage, but we need to operate on the pixels again anyways so it doesn't matter
    # the decoder will do the two steps combined to save memory.
    deltas: list[int] = []
    for byte in data:
        for bit in range(8):
            pixel_value = 1 if (byte & (1 << bit)) >= 1 else 0
            delta = previous_pixel ^ pixel_value
            deltas.append(delta)
            previous_pixel = pixel_value

    bias = 7
    limit = 127 + bias
    marker = 0x80

    output = bytearray()
    current_run_length = 0
    current_raw_deltas: list[int] = []

    def terminate_run():
        nonlocal output, current_run_length, marker, limit
        if current_run_length > 0:
            while current_run_length > limit:
                output.append((limit - bias) | marker)
                current_run_length -= limit
            if current_run_length >= bias:
                output.append((current_run_length - bias) | marker)
            else:
                current_raw_deltas.extend([0] * current_run_length)
                if current_run_length == 7:
                    terminate_raw_deltas()
            current_run_length = 0

    def terminate_raw_deltas():
        nonlocal current_raw_deltas, output
        if len(current_raw_deltas) > 0:
            byte = 0
            for delta in current_raw_deltas:
                byte = (byte >> 1) | (delta << 6)
            assert byte < limit
            output.append(byte)
            current_raw_deltas = []

    for delta in deltas:
        if delta == 0:
            # if we're in raw pixels, continue them until the end.
            if len(current_raw_deltas) > 0:
                current_raw_deltas.append(delta)
                if len(current_raw_deltas) == 7:
                    terminate_raw_deltas()
            # prefer to continue or start run otherwise.
            else:
                current_run_length += 1
        else:
            # terminate any ongoing run since we can't continue it
            # if the run is less than 7 pixels, it's more efficient to encode it together with this white pixel as a raw byte
            if current_run_length > 0 and current_run_length < bias:
                current_raw_deltas.extend([0] * current_run_length)
                current_run_length = 0
            else:
                terminate_run()
            current_raw_deltas.append(delta)
            if len(current_raw_deltas) == 7:
                terminate_raw_deltas()

    # no need to terminate an ongoing run, fill color is always black
    # only terminate delta list if necessary
    if len(current_raw_deltas) > 0:
        remaining_padding = 7 - len(current_raw_deltas)
        current_raw_deltas.extend([0] * remaining_padding)
        terminate_raw_deltas()
    return output


T = TypeVar("T")


def encode_block(
    block: Image, previous_block: Image, encoder_counts: list[int], k_counts: list[int]
) -> bytes:
    block_data = bytes(reverse_mask(byte) for byte in block.tobytes())
    # do zig-zag or snaking encoding of the block data, which may generate longer stretches of the same color
    block_data_snake = bytes(
        chain.from_iterable(
            [
                *list(normal_row),
                *list(reversed([reverse_mask(x) for x in reverse_row])),
            ]
            for normal_row, reverse_row in batched(
                batched(block_data, x_block_size // 8), 2
            )
        )
    )
    previous_block_data = bytes(reverse_mask(byte) for byte in previous_block.tobytes())
    # previous_block_data_snake = bytes(
    #     reverse_mask(byte)
    #     for byte in previous_block.transpose(Image.Transpose.TRANSPOSE).tobytes()
    # )
    delta = encode_interframe_delta(block_data, previous_block_data)
    # delta_snake = encode_interframe_delta(block_data_snake, previous_block_data_snake)
    # RLE
    compressed_image_data = encode_rle(block_data)
    # RLE + nibble compression
    nibble_compressed = compress_into_nibbles(encode_rle(block_data))
    nibble_compressed_delta = compress_into_nibbles(encode_rle(delta))
    nibble_snake_compressed = compress_into_nibbles(encode_rle(block_data_snake))
    # nibble_snake_compressed_delta = compress_into_nibbles(encode_rle(delta_snake))
    # RLE + LEB128
    # leb_compressed = encode_leb(encode_rle_unbounded(block_data))
    # leb_snake_compressed = encode_leb(
    #     encode_rle_unbounded(block_data_snake)
    # )
    # garbagémon
    garbagémon_compressed = encode_garbagémon(block_data)
    garbagémon_compressed_delta = encode_garbagémon(delta)
    garbagémon_snake_compressed = encode_garbagémon(block_data_snake)
    # garbagémon_snake_compressed_delta = encode_garbagémon(delta_snake)
    # Pokémon
    # FIXME: doesn't properly account for two different start types so far
    # pokémon_compressed, _ = encode_pokémon(block_data)
    # pokémon_compressed_delta, _ = encode_pokémon(delta)
    # pokémon_snake_compressed, _ = encode_pokémon(block_data_snake)
    # RLE + Rice/Exponential-Golomb
    # rice_compressed, k_rice = encode_rice(encode_rle_unbounded(block_data))
    # rice_compressed_delta = encode_rice(encode_rle_unbounded(delta))
    # rice_snake_compressed, k_rice_snake = encode_rice(
    #     encode_rle_unbounded(block_data_snake)
    # )
    # Pick best compressor. In the case that they are equal, pick fast compressor.
    compressed_image_data: bytes
    compressed_size: int = 100000000000000000
    compressed_encoder_number: int
    for i, data in enumerate(
        [
            nibble_compressed,
            nibble_compressed_delta,
            nibble_snake_compressed,
            # nibble_snake_compressed_delta,
            # leb_compressed,
            # leb_snake_compressed,
            garbagémon_compressed,
            garbagémon_compressed_delta,
            garbagémon_snake_compressed,
            # pokémon_compressed,
            # pokémon_compressed_delta,
            # pokémon_snake_compressed,
            # rice_compressed,
            # rice_compressed_delta,
            # rice_snake_compressed,
            # block_data,
        ]
    ):
        if len(data) < compressed_size:
            compressed_encoder_number = i
            compressed_image_data = data
            compressed_size = len(data)

    encoder_counts[compressed_encoder_number] += 1
    # k_counts[k_rice] += 1
    # k_counts[k_rice_snake] += 1
    # assert compressed_size < (x_block_size * y_block_size) / 8
    # assert compressed_size < 256
    # bytes([compressed_encoder_number, compressed_size])
    return bytes([compressed_encoder_number]) + compressed_image_data


def encode(input: Path):
    video = cv2.VideoCapture(str(input))
    success, image = video.read()
    video_fps = video.get(cv2.CAP_PROP_FPS)
    video_frame_count = video.get(cv2.CAP_PROP_FRAME_COUNT)
    video_seconds = video_frame_count / video_fps
    output_frame_count = video_seconds * fps

    frame_references: list[str] = []
    frame_c_arrays = ""
    name_for_frame: dict[bytes, str] = {}

    count = 0
    reuse_count = 0
    encoder_counts = [0] * 20
    k_counts = [0] * 20

    binary_size = 0
    raw_size = 0

    success = True
    last_frame: Image.Image | None = None
    while success:
        video.set(cv2.CAP_PROP_POS_MSEC, int(count * ms_per_frame))
        success, image = video.read()
        if not success:
            break
        image: Image = Image.fromarray(image)
        width, height = image.size
        height_scale = height / target_height
        image = image.resize((int(width / height_scale), int(target_height))).convert(
            mode="1"
        )
        x_blocks = floor(image.size[0] / x_block_size)
        y_blocks = floor(image.size[1] / y_block_size)
        image = image.crop((0, 0, x_blocks * x_block_size, y_blocks * y_block_size))
        image_binary = bytes(reverse_mask(byte) for byte in image.tobytes())

        array_name = f"frame_{count}"
        # two CCITT Group 4 encoding options (stripped and unstripped)
        bin_output = BytesIO()
        image.save(bin_output, format="tiff", compression="group4")
        # compressed_image_data = bin_output.getbuffer()
        # compressed_image_data = strip_tiff(bin_output.getbuffer())

        srlv_frame_name = Path(f"frames/{array_name}.tiff")
        srlv_frame_name.write_bytes(bin_output.getbuffer())

        # split image up into macroblocks
        compressed_image_data = bytearray()
        for x_index, y_index in product(range(x_blocks), range(y_blocks)):
            # print(count, x_index, y_index)
            box = (
                x_index * x_block_size,
                y_index * y_block_size,
                (x_index + 1) * x_block_size,
                (y_index + 1) * y_block_size,
            )
            block = image.crop(box)
            if last_frame is not None:
                previous_block = last_frame.crop(box)
            else:
                previous_block = block
            encoded_block = encode_block(
                block, previous_block, encoder_counts, k_counts
            )
            compressed_image_data += encoded_block

        c_array = bytes_to_c_array(compressed_image_data, array_name)
        # disable frame reuse since interframe compression messes with it.
        # first instance of this image
        if name_for_frame.get(bytes(compressed_image_data)) is None:
            name_for_frame[bytes(compressed_image_data)] = array_name
            frame_c_arrays += c_array + "\n"
            binary_size += len(compressed_image_data)
        else:
            reuse_count += 1
        frame_references.append(name_for_frame[bytes(compressed_image_data)])

        raw_size += len(image_binary)
        count += 1
        if count % (fps * 3) == 0:
            print(f"{count / output_frame_count * 100:.1f}% done")

        last_frame = image

    print(f"{reuse_count / output_frame_count * 100:.2f}% of frames reused")
    print(
        ", ".join(
            f"{count / (x_blocks*y_blocks*output_frame_count) * 100:.2f}% {name}"
            for count, name in zip(
                encoder_counts,
                [
                    "nibble",
                    # "nibble delta",
                    "nibble snake",
                    # "garbagémon",
                    # "garbagémon delta",
                    # "garbagémon snake",
                    # "pokémon",
                    "pokémon delta",
                    "pokémon snake",
                    "rice",
                    # "rice delta",
                    "rice snake",
                    # "raw",
                ],
            )
        )
    )
    print(
        "k:\n",
        ", ".join(
            f"{count / (x_blocks*y_blocks*output_frame_count*7) * 100:.2f}% {k}"
            for k, count in enumerate(k_counts)
        ),
    )
    print(
        f"{binary_size} bytes compressed, {raw_size} bytes raw, {binary_size / raw_size * 100:.1f}%"
    )
    frame_reference_c_array = f"static unsigned char* frames[] PROGMEM = {{ {', '.join(frame_references)} }};\n"
    frame_sizes = f"static size_t frame_sizes[] PROGMEM = {{ {', '.join(f'sizeof({reference})' for reference in frame_references)} }};\n"
    output = Path("bad_apple.h")
    output.write_text(
        frame_c_arrays + "\n" + frame_reference_c_array + "\n" + frame_sizes,
        encoding="utf-8",
    )


def main():
    parser = argparse.ArgumentParser(
        description="Encode video to TIFF CCITT Group 4 images"
    )
    parser.add_argument("input", type=Path, help="Input file")
    args = parser.parse_args()
    encode(args.input)


if __name__ == "__main__":
    main()
