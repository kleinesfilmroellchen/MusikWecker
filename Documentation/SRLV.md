# Single-bit Run Length Video (SRLV)

|         |     |
| ------- | --- |
| Version | 0.3 |

Single-bit Run Length Video, abbreviated with SRLV, is a simple video format for black-and-white (1 bit per pixel) intended for software decoding on weak hardware. With the mildly optimized SRLV optimization in this repository, an ESP8266 (a single Xtensa core at 160MHz) can decode 833 frames per second (1.2ms per frame) from 80MHz flash. The small amount of time spent on decoding video allows a microcontroller core to perform all the other time-consuming duties of video playback (audio decoding, I2S communication with the audio DAC, SPI communication when reading audio and/or video from an SD card, I2C or SPI communication with the display) without much worry that the video decoding part will slow it down.

The `compression` directory contains the Python SRLV encoder. It's currently not very configurable apart from the input file, and there's some commented-out code that was used in developing the codec. If you are interested, you can play with these to come up with an even better scheme. Please leave a PR in this case and I may update SRLV (which may include a rename, since the name describes the compression scheme precisely).

> [!NOTE]
> Notes like this one are purely informational and do not specify the format beyond the specification text.

- auto-gen TOC;
  {:toc}

## Video Format

SRLV only performs intra-frame compression, and allows reusing identical frames (such as relatively common pure white or black frames). At present, only the single-frame format of SRLV is specified, and it is not containerized, so external knowledge of the video's dimensions are required to decode it correctly. A simple container format will be specified at a later point and added to this document.

## Frame format

This section specifies a single SRLV frame's raw format. Such frames may be stored using the `srlvf` (SRLV Frame) extension, and `image/srlvf` MIME type. Note that since this is a raw uncontainerized frame, there is no information about the frame's dimensions.

Encoding a frame is done with the following steps:

- The frame is converted to 1BPP XBM-compatible bit layout, including padding.
- The frame is optionally encoded as an inter-frame delta, i.e. a difference from the previous frame.
- Alternatively, the frame's scanlines are optionally converted to a back-and-forth (snake) format.
- The binary data is compressed using one of two methods:
  - Alternating run-length encoding followed by packing with nibble/byte entropy coding.
  - Pixel delta coding followed by black-run/raw coding.

There are six ways of encoding a frame. The numeric value of each method is prefixed as a byte in front of the encoded data, and the name given here will be used throughout the specification.

| Method                                                                 | Name          | Encoding ID |
| ---------------------------------------------------------------------- | ------------- | ----------- |
| RLE with nibble/byte entropy coding                                    | Nibble        | 0           |
| RLE with nibble/byte entropy coding and inter-frame delta              | Nibble Delta  | 1           |
| RLE with nibble/byte entropy coding and snaking scanline reshuffling   | Nibble Snake  | 2           |
| Pixel delta with black-run/raw coding                                  | Pokémon       | 3           |
| Pixel delta with black-run/raw coding and inter-frame delta            | Pokémon Delta | 4           |
| Pixel delta with black-run/raw coding and snaking scanline reshuffling | Pokémon Snake | 5           |

Decoding simply performs the encoding steps in reverse order, using the encoding method specifier to decide which decoding steps (including the two optional preprocessing steps) to perform. Many embedded applications can already use images in XBM format, so the first step doesn't usually have to be applied in reverse in the decoder.

### Conversion to XBM

Frames contain one bit per pixel. The conventional interpretation is 1 = white, 0 = black, but depending on the context any two-color mapping is possible. It is recommended that 1 be assigned the brighter color.

Individual pixel's bits are packed into bytes according to XBM format. This means that pixels are first arranged into a left-to-right, top-to-bottom bitstream. Within the bitstream, consecutive scanlines are therefore arranged top-to-bottom, and the pixels in a scanline appear left-to-right.

> [!NOTE]
> This bit order is equivalent, among many others, to the Quite OK Image Format's (QOI) pixel order, the draw order for CRT televisions, as well as most video signals.

The bitstream is split into bytes, with zero padding at the end. Each byte contains the bits in LSB to MSB order, matching the X Bitmap (XBM) encoding format.

> [!NOTE]
> Run-length encoding does not care about the byte layout, just the bitstream with zero padding. The byte format is specified so that the output of the decompressor, which necessarily needs to have a byte-level format, is properly defined.

### Inter-frame delta coding

This preprocessing step encodes the bitstream as a delta from the previous frame. The unencoded bitstream $y_0$ of the previous frame is bit-wise XORed with the unencoded bitstream $y_1$ of the current frame, such that each bit of the output $x_1$ is given as $x_1[i] = y_0[i] + y_1[i]$.

> [!NOTE]
> For implementation, a byte-wise XOR on the XBM-formatted data of each frame has the same effect as the bitwise XOR.

Inter-frame delta coding methods cannot be used on the first frame.

### Scanline snake reshuffling

This preprocessing step reorders bits in the image before compression / after decompression. The final result is a bit order that "snakes" back and forth across the image. This processing step cannot be combined with inter-frame delta coding.

> [!NOTE]
> Combining snake and delta coding rarely yields better compression under either of the two schemes, so it was removed to speed up close-to-optimal encoding and simplify decoding.

When performing snake reshuffling, every second scanline (horizontal row of pixels) of the image is reversed. The first scanline is not reversed. Reversing means full bit reversal, not just byte reversal. This has the effect that the bits do not scan the image scanline-wise left-to-right, but alternatingly left-to-right and then right-to-left. 

It is recommended to use byte-aligned image widths (pixel count divisible by 8) to accelerate implementations. Some implementations can only handle encoding modes 2 and 5 when this is the case.

### "Nibble" compression

The frame bitstream is run-length encoded using alternating runs of black and white pixels. The first run is black, while the second is white, the third is black again and so on. A run may have any length of 0 to 127 (both inclusive). Runs of length 0 are explicitly valid.

> [!NOTE]
> Run length 0 is used to split long runs up without complicating the format with variable-length codes. For instance, a run of 300 black can be encoded as a 127-bit run of black, a 0-bit run of white (meaning no white pixels at all), another pair of those, and finally a 46-bit run of black:
>
> `0x7f 0x00 0x7f 0x00 0x2e`
>
> For arbitrary data, run length 0 is absolutely necessary.

The final run may be shorter than the remaining number of bits. In this case, the run extends to the end of the bitstream.

> [!NOTE]
> This rule allows saving on many split runs like above for large single-color surfaces, especially when they cover most or all of the image. For example, solid black may be encoded with empty data and solid white with `0x00` regardless of the image size. This is one of the reasons that the image size (and therefore bitstream size) needs to be known.

RLE yields a list of numbers, each from 0 to 127 inclusive. Byte/nibble entropy coding packs these numbers into bytes, using one of two byte formats:

- Byte encoding: A single number is encoded in a byte.
- Nibble encoding: Two numbers are encoded in a byte, using the two nibbles. The top nibble is occupied by the first number in the RLE stream, and the bottom nibble is occupied by the second number.

The topmost bit (MSB) of a byte identifies the kind of coding used. `1` means byte encoding, while `0` means nibble encoding.

For byte encoding, the whole range of values is supported. For nibble encoding, the top nibble may occupy values 0-7 inclusive (using three bits), and the bottom nibble may occupy values 0-15 inclusive (using four bits).

> [!NOTE]
> There are many ways of packing the same numbers into this entropy coding scheme. The reference implementation probably does not implement the best one, and in fact for a while it had a significant inefficiency, restricting the bottom nibble to the same 0-7 range.

### "Pokémon" compression

The frame bitstream is first pixel-delta encoded, and then black portions are run-length encoded.

> [!NOTE]
> The name "Pokémon" was chosen for the pixel delta-based format since it was loosely inspired by part of the compression method used for encoding Pokémon battle sprites in Pokémon Red and Blue (1996/1998). This compression method, addressing many of the same issues as this one (highly limited memory and relatively limited processing power), is surprisingly sophisticated and very interesting in its own right (and the cause of the infamous glitch Pokémon MissingNo's appearance), details can be found [here](https://www.youtube.com/watch?v=aF1Yw_wu2cM).
>
> Pokémon usually performs better on images with large amounts of noise, as its overhead for fast-changing pixels is significantly less than with the simple RLE scheme.

To perform pixel-delta encoding, each pixel is encoded as the XOR of itself with the previous pixel in bitstream order. The previous pixel of the first pixel is black.

For the run-length coding of these deltas, each byte may encode either:

- a run of up to 128 pixels of black (0) or
- 7 uncompressed pixels, in LSB-to-MSB-order

The topmost bit (MSB) of a byte identifies the kind of coding used. `1` means black RLE, while `0` means raw encoding. For the RLE, a bias of 1 is used, so a codeword of `0x80` represents not a run of length 0, but length 1.

> [!NOTE]
> Unlike in the simple RLE, since arbitrary codewords can follow one another, it is not necessary to have runs of length 0.

If the data does not specify all pixel values, the rest of the pixels are filled with 0.

### Turtle compression

Data for a frame consists of a frame header (9 byte) followed by any number of objects drawn onto the image.

The frame header starts with an info byte of the form `i0000000`:

- `i` is a flag determining whether to invert the frame colors at the end. By default, white (1) pixels are drawn on top of a black (0) background. If inverted, frames effectively have black pixels drawn on top of a white background.

Following this are 8 bytes for the Huffman coding table. Each byte contains a 3-bit entry bitsize (biased by 1, so 1-8) and an entry. The bitsize occupies the topmost 3 bits while the entry itself is situated at the least significant bits and cut off according to the given bitsize. (Any bitsize above 5 is illegal, as the entry only has 5 bits available.)

The entries for the table are always in this order:

- 135L
- 90L
- 45L
- F1
- FN
- FX
- 45R
- 90R
- 135R

See below for command names. In the following only these command names are used, which are substituted with their respective Huffman code in each frame.

#### Objects

An object starts with two header bytes of the form `0xxxxxxx 0yyyyyyy`:

- `x` is the X offset of the first pixel.
- `y` is the Y offset of the first pixel.

For solid objects, turtle graphics logic is used. The turtle has a current position on the image, as well as a grid direction that is aligned to 45 degrees (i.e. diagonal or horizontal or vertical). The initial facing direction is always down right (positive X and Y). Any command instructs the turtle to optionally change its facing direction, and then take at least one step in that new direction. The commands are:

- `90L`: Rotate 90 degrees left, then take one step.
- `90R`: Rotate 90 degrees right, then take one step.
- `45L`: Rotate 45 degrees left, then take one step.
- `45L`: Rotate 45 degrees right, then take one step.
- `135L`: Rotate 135 degrees left, then take one step.
- `135R`: Rotate 135 degrees right, then take one step.
- `F1`: Move forward one step without rotating.
- `FN`: Move forward at least 2 steps without rotating.
- `FX`: Move forward until starting position or image border is reached.

The `FN` command is the only one with a single parameter, namely the number of steps to move forward. The steps are encoded using Rice-Golomb coding with order `k = 2` and bias 1, such that the step count 2 is encoded as value 1.

The turtle commands automatically end once the starting location is reached again. The starting position is always filled in, but there’s no initial step taken in the starting direction. Any surface enclosed by the edge drawn by the turtle is filled in. The usual even-odd filling algorithm is used: Any pixel is filled in if any line from it to infinity crosses through an odd number of edges. The filling algorithm disregards any existing objects in the image.

The object is combined onto the full image by means of an XOR operation for each pixel, i.e. every pixel filled by the object is inverted in the final image.

#### Single-pixel trailer

Single-pixel trailer objects are a list of pixels to be set.

After the same common header as the normal objects use, each following coordinate is encoded using Rice-Golomb coding with order `k = 5` and zig-zag coding, such that 1 -> 1, -1 -> 2, 2 -> 3, -2 -> 4, 3 -> 5 etc.

All pixel coordinates, each a pair (x, y), are encoded as the difference from the previous one.
