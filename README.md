# QR123

Minimal fast QR encoder (up to version 3) for 32-bit MCU.

## Features and limitations

There are small QR encoders out there which are intended for small MCU
applications, but their features still exceed my needs by a wide margin
therefore not quite small nor quite fast.  I was wondering if I focus on the
functions and capacities that I actually need, can I bypass much of the QR
encoding complexity and make a super light weight QR encoder? Turns out, it's
entirely possible and here is the result.

NB: although I'm pleased with the result and like to share it, please check out
the limitations to make sure it's useful for *YOUR* application.

1. It only supports QR version 1, 2, and 3 (max 53 bytes content).
2. ECC level is fixed to LOW.
3. It supports binary mode only.
4. The data masking is fixed to #0.

All these limitations are to be explained; let's see the numbers first (using
arm-gcc 7-2018-q2, Cortex-M4 target with -Os).

| QR_SPEED | Size (bytes) | Stack (bytes) | V3 encode() cycles
| --- | --- | --- | ---
| 1 | 820 | 120 | 128k
| 2 | 980 | 120 | 85k
| 3 | 1462 | 144 | 63k

QR123 only uses the stack for working buffer and a caller supplied 128B context
to hold some parameters and the result QR bitmap.

QR_SPEED is a configurable macro that chooses the implementation of the ECC
calculation, specifically the very busy Galois Field multiplier routine:

- QR_SPEED = 1 selects the basic iterative multiplier.
- QR_SPEED = 2 enables the unrolled assembly version.  This setting is for
Thumb-2 only.
- QR_SPEED = 3 uses the fast GF multiplier based on log/exp look-up tables (cost
512 bytes).

The API has only two functions, one is `qr_eval()` that takes the inputs and
setup the context; the other is `qr_encode()` which does the actual encoding.
The result bitmap is one word per line with dots placed from MSB to LSB, and an
inline helper `qr_getdot()` is provided to show how to iterate through it.
That's all.

## Limitations explained

The limitations are what make QR123 so small and fast.  The chosen parameters
work well with each other to ensure usability of such a simple and small
implementation.

### 1. Version limited to V3

QR123 is for MCU devices with small displays, typically having no more than 128
pixels on each side and their physical sizes typically less than 2".  For
example the popular 128x64 LCD and 128x128 TFT modules.  For these small
displays, version 3 is about the right size.  Yes it's possible to show larger
versions using 1 pixel per dot but the small physical size makes the dots harder
to focus on.

This also limits the memory usage, avoids drawing multiple alignment patterns,
and avoids the version info bits.

Version 3 is 29x29, which means we can put an entire line in a 32-bit word.
This is quite convenient and efficient.  If someday we need the auto-masking
feature, it can be made several times faster than a dot-by-dot approach.

### 2. ECC level LOW only

This is an easy decision.  Screens are easy to clean and they don't curl or wear
like paper or fabric, and the code itself is small.  There won't be many error
to correct, so why not give more bits to the payload? -- especially when the
size is limited.

Besides, ECC LOW up to V4 also allows straightforward ECC calculation without
having to divide the buffer into blocks then interleave the results.  Another
complexity is avoided.

### 3. Binary mode only

QR code has 4 data encoding modes:

1. Kanji mode is for Shift-JIS encoded Kanji characters but is omitted by most
encoders, because the world has moved on to Unicode.
2. Numeric mode is just for numbers, might be useful in some cases.
3. Alphanumeric mode supports numbers, upper case A-Z and a limited selection of
punctuation marks, making it good for basic URLs.  Had this mode used Base64
encoding (the URL-safe variant) it would be much more useful.
4. Binary mode: totally transparent, byte for byte without any internal
encoding.

All the encodings are simple; this choice is based on their usefulness.  Because
any lower case letter in the data means we'll in binary mode anyway, I decide to
leave other modes to the future when they are needed.

### 4. Fixed data mask

Auto data masking is quite expensive.  It comprises the majority of cycles in an
auto-masking encoder, check out [Nayuki's benchmark and
observations](https://www.nayuki.io/page/fast-qr-code-generator-library).

The high cost is by design: all 8 predefined masks must be applied and evaluated
by a long, multi-step scoring procedure, and the mask producing the lowest score
wins.  A low score means the dark and light dots (modules) are more randomly
mixed without large/long/confusing features, which makes the decoding easier.

Although I wish they had adopted a sophisticated but fixed bit permutation
scheme to achieve the same goal, I suppose this exhaustive process was a good
choice because usually QR codes are generated on PCs and the readers are on
resource limited scanners (think warehouse scenario).  It makes sense to favor
the reader and shift more processing to the encoder.

If we can't change it can we skip it?  One can understand that auto-masking is
necessary for big codes printed on rough, crumpled paper, what about small codes
displayed on a clean, perfectly flat, high contrast surface like the LCD?  Every
QR code has timing patterns to mark the fine dimensions of each row and column,
and they should function perfectly in this case.  In other words: if a reader
does its job properly by utilizing the timing patterns, it shouldn't fail on
codes with high scores, at least not on a perfect surface.

I experimented and my conclusion so far is that QR123 is perfectly OK with the
fixed data mask #0.  You may try for yourself the following test samples with
different QR readers.

1. The ["f-train"](f-train.png): a train of 53 lower case 'f', an easy to
produce bad case.  The code has long vertical stripes to score poorly, but none
of the readers hesitates.  BTW this code does resemble a bird's eye view of a
railway platform.

2. [All-dark](all-dark.png): worst case with all data bits shown as dark dots.
Reading takes more time but none needs a retry.  Data to produce this code:

        0x66, 0x66, 0x66, 0x69, 0x99, 0x99, 0x99, 0x99, 0x96, 0x69, 0x99, 0x99,
        0xa6, 0x66, 0x66, 0x99, 0x66, 0xac, 0xcc, 0xcc, 0xcd, 0x33, 0x2c, 0xcd,
        0x33, 0x33, 0x33, 0x33, 0x33, 0x2c, 0xcc, 0xcc, 0xcc, 0xcc, 0xcd, 0x33,
        0x2c, 0xcd, 0x33, 0x33, 0x33, 0x33, 0x33, 0x2c, 0xcc, 0xcc, 0xcc, 0xcc,
        0xcd, 0x33, 0x2c, 0xcd, 0x33


3. [All-light](all-light.png): worst case with all data bits as light dots.
This one is a bit more challenging than the all-dark case.  Reading is slower
and 2 readers need retry but none fails.  It seems the retries are for better
alignment -- if I carefully fill the scan area with the code the read will be 
successful on the first try.  Data for this code:

        0x99, 0x99, 0x99, 0x96, 0x66, 0x66, 0x66, 0x66, 0x69, 0x96, 0x66, 0x66,
        0x59, 0x99, 0x99, 0x66, 0x99, 0x53, 0x33, 0x33, 0x32, 0xcc, 0xd3, 0x32,
        0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xd3, 0x33, 0x33, 0x33, 0x33, 0x32, 0xcc,
        0xd3, 0x32, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xd3, 0x33, 0x33, 0x33, 0x33,
        0x32, 0xcc, 0xd3, 0x32, 0xcc

It's possible that some old readers may perform poorer on the all-light and
all-dark samples, since these are contrived extreme cases.

## References

- [QR Code Tutorial by
Thonky](https://www.thonky.com/qr-code-tutorial/introduction): step by step
explanation of the encoding process.
- [QR Code Step by Step,
Nayuki](https://www.nayuki.io/page/creating-a-qr-code-step-by-step): this makes
my coding so much easier.
- [Reed-Solomon Codes for Coders,
Wikiversity](https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders)
for the ECC.


vim: set ai et ts=4 tw=80 syn=markdown spell spl=en_us fo=ta:
