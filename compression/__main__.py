import argparse
from pathlib import Path
import cv2
from PIL import Image
from io import BytesIO

target_height = 64
fps = 3
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

    success = True
    while success:
        video.set(cv2.CAP_PROP_POS_MSEC, int(count * ms_per_frame))
        success, image = video.read()
        if not success:
            break
        image: Image = Image.fromarray(image)
        width, height = image.size
        height_scale = height / target_height
        bin_output = BytesIO()
        image = image.resize((int(width / height_scale), int(target_height))).convert(
            mode="1"
        )
        image.save("test.tiff", format="tiff", compression="group4")
        bin_output = BytesIO(bytes(reverse_mask(x) for x in image.tobytes()))
        array_name = f"frame_{count}"
        c_array = bytes_to_c_array(bin_output.getbuffer(), array_name)
        # first instance of this image
        raw_image_data = image.tobytes()
        if name_for_frame.get(raw_image_data) is None:
            name_for_frame[raw_image_data] = array_name
            frame_c_arrays += c_array + "\n"
        else:
            reuse_count += 1
        frame_references.append(name_for_frame[raw_image_data])
        binary_size += len(bin_output.getbuffer())

        count += 1
        if count % fps == 0:
            print(f"{count / output_frame_count * 100:.1f}% done")

    print(f"{reuse_count / output_frame_count * 100:.2f}% of frames reused")
    print(f"{binary_size} bytes")
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
