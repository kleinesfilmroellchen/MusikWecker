from pathlib import Path
from PIL import Image, ImageOps, UnidentifiedImageError
import regex

# regex to find the insertion point of the PROGMEM macro, which will be at the end of the regex.
progmem_insertion_finder = regex.compile(r" char \p{ID_Start}\p{ID_Continue}*\s*\[\]")
im_identifier_finder = regex.compile(r"\bimage_")


def main():
    directory = Path("./graphics")
    total_code = ""
    for image_path in directory.iterdir():
        print(f"converting {image_path}...")
        try:
            image = Image.open(image_path)
        except UnidentifiedImageError:
            continue
        image = ImageOps.invert(
            image.convert(mode="RGB").convert(mode="1", dither=None)
        )
        image_code = image.tobitmap().decode(encoding="utf-8")

        # do some transpilation so the xbm code is usable
        image_identifier_name = f"{image_path.stem}_symbol_"
        print(progmem_insertion_finder.search(image_code))
        insertion_match = progmem_insertion_finder.search(image_code)
        progmem_point = insertion_match.end()
        unsigned_point = insertion_match.start()
        image_code = (
            image_code[:unsigned_point]
            + " unsigned"
            + image_code[unsigned_point:progmem_point]
            + " PROGMEM"
            + image_code[progmem_point:]
        )
        image_code = im_identifier_finder.sub(
            string=image_code, repl=image_identifier_name
        )

        total_code += "\n" + image_code

    with open("graphics.h", "w", encoding="utf-8") as output:
        output.write(total_code)


if __name__ == "__main__":
    main()
