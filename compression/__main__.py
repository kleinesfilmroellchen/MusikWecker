import argparse
from pathlib import Path
import cv2
from PIL import Image
from io import BytesIO
import struct

target_height = 64
fps = 9
ms_per_frame = (1 / fps) * 1000


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
    threshold = 7

    def append_byte(byte: int):
        output.append(byte | full_byte_marker)

    def append_nibbles(a, b):
        output.append((a << 4) | b)

    index = 0
    while index < len(data) - 1:
        a, b = data[index], data[index + 1]
        if a > threshold:
            append_byte(a)
            index += 1
        # minor optimization to skip over b in the next loop
        elif b > threshold:
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

    binary_size = 0
    raw_size = 0

    success = True
    last_frame: bytes | None = None
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
        image_binary = bytes(reverse_mask(byte) for byte in image.tobytes())

        array_name = f"frame_{count}"
        # two CCITT Group 4 encoding options (stripped and unstripped)
        # bin_output = BytesIO()
        # image.save(bin_output, format="tiff", compression="group4")
        # compressed_image_data = bin_output.getbuffer()
        # compressed_image_data = strip_tiff(bin_output.getbuffer())

        # inter-frame deltas
        # if last_frame is not None:
        #     source_frame = encode_interframe_delta(image_binary, last_frame)
        # else:
        source_frame = image_binary
        # RLE
        # compressed_image_data = encode_rle(source_frame)
        # RLE + nibble compression
        compressed_image_data = compress_into_nibbles(encode_rle(source_frame))

        srlv_frame_name = Path(f"frames/{array_name}.srlvf")
        srlv_frame_name.write_bytes(compressed_image_data)

        c_array = bytes_to_c_array(compressed_image_data, array_name)
        # first instance of this image
        if name_for_frame.get(image_binary) is None:
            name_for_frame[image_binary] = array_name
            frame_c_arrays += c_array + "\n"
            binary_size += len(compressed_image_data)
        else:
            reuse_count += 1
        frame_references.append(name_for_frame[image_binary])

        raw_size += len(image_binary)
        count += 1
        if count % fps == 0:
            print(f"{count / output_frame_count * 100:.1f}% done")

        last_frame = image.tobytes()

    print(f"{reuse_count / output_frame_count * 100:.2f}% of frames reused")
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
