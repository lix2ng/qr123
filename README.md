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

1. It only supports QR version 1, 2, and 3 (53B content max).

2. ECC level is fixed to LOW.

3. It's binary mode only.

4. The data masking is fixed to #0.

All these limitations are to be explained, but now let's see the numbers first
(using arm-gcc 7-2018-q2, Cortex-M4 target with -Os).

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

The limitations are what make QR123 so small and fast.  The chosen parameters,
or subset of features and capacities, work well with each other to ensure
usability of such a simple and small implementation.

### 1. Version limited to V3

QR123 is for MCU devices with small displays, typically having no more than 128
pixels on each side and their physical sizes typically less than 2".  For
example the popular 128x64 LCD and 128x128 TFT modules.  For these small
displays, version 3 is about the right choice.  Yes it's possible to show larger
versions using 1 pixel per dot but the small physical size makes the dots harder
to focus on.

This also limits the memory usage, avoids drawing multiple alignment patterns,
and avoids the version info bits.

Version 3 is 29x29, which means we can put an entire line in a 32-bit word.
This is quite convenient and efficient.

### 2. ECC level LOW only

This is an easy choice.  Screens are easy to clean and they don't curl or wear
like paper or fabric.  If there aren't many error to correct why not dedicate
more bits to payload especially when the size is limited?

Besides, ECC LOW up to V4 also allows straightforward ECC calculation without
having to divide the buffer into blocks then interleave the results.

### 3. Binary mode only

QR code defines 4 data encoding modes:

1. Kanji mode is for Shift-JIS encoded Kanji characters and omitted by most
encoders, because the world has moved on to Unicode.

2. Numeric mode is just for numbers, might be useful in some cases.

3. Alphanumeric mode supports numbers, upper case A-Z and a limited selection of
punctuation marks, making it good for basic URLs.  Had this mode adopted BASE64
encoding (the URL-safe variant) it would be much more useful.

4. Binary mode: totally transparent, byte for byte without any internal
encoding.

This choice is based on their usefulness.  Since any lower case letter in the
data means we have to use binary mode anyway, I leave other modes for now until
they are needed.

### 4. Fixed data mask

Auto data masking is quite expensive.  It comprises the majority of cycles in an
auto-masking encoder, check out [Nayuki's benchmark and
observations](https://www.nayuki.io/page/fast-qr-code-generator-library).

The high cost is by design: all 8 predefined masks must be applied and evaluated
by a multi-step scoring procedure, and the mask producing the lowest score wins.
A low score means the dark and light dots (modules) are more randomly mixed
without large/long/confusing dark or light features, which makes the decoding
easier.

Although I wish they had adopted a capable but fixed bit permutation algorithm
to achieve the same goal, I suppose this exhaustive process was the right choice
because usually QR codes are generated on PCs and the readers are on resource
limited scanners (think warehouse scenario).  It makes sense to favor the reader
and shift more processing to the encoder.

One can understand that auto-masking is necessary for poorly rendered big codes,
but what about small codes displayed on a clean, perfectly flat, high contrast
surface like the LCD? Every QR code has timing patterns to give the fine
dimensions of each row and column, and they should function perfectly in this
case.

I experimented and my conclusion so far is that QR123 is perfectly OK with the
fixed data mask #0.  You may try for yourself the following test samples with
different QR readers.

1. The ["f-train"](f-train.png): a train of 53 lower case 'f', an easy to
produce bad case.  The code has long vertical stripes to score poorly, but none
of the readers hesitates.  BTW this code does resemble a bird's eye view of a
train platform.

2. [All-dark](all-dark.png): worst case with all data bits shown as dark dots.
Reading becomes slower but none needs a retry.

3. [All-light](all-light.png): worst case with all data bits as light dots.
This one is more challenging than the all-dark case.  Reading is even slower and
2 readers need retries but none fails.

Some old readers may experience more retries on all-light and all-dark cases,
because these are contrived extreme cases, not real data.

## References

- [QR Code Tutorial by
Thonky](https://www.thonky.com/qr-code-tutorial/introduction): step by step
explanation of the encoding process.

- [QR Code Step by Step,
Nayuki](https://www.nayuki.io/page/creating-a-qr-code-step-by-step): makes my
coding so much easier.

- [Reed-Solomon Codes for Coders,
Wikiversity](https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders)
for the ECC.

[](
vim: set ai et ts=4 tw=80 syn=markdown spell spl=en_us fo=ta:
)

