/*
 * QR123: minimal fast QR encoder for version 1, 2, 3.
 * 
 * Copyright (c) 2019 Ling LI <lix2ng@gmail.com>, shared under the MIT license.
 *
 * Verify results here:
 *    https://www.nayuki.io/page/creating-a-qr-code-step-by-step
 *
 * Size and performance numbers for Cortex-M4 using gcc-arm 7-2018-q2:
 * eval() cycles are about 0.5k; footprint and encode() cycles in various
 * QR_SPEED settings are:
 *
 *    1. code 820, stack 120, v3 cycles 138k (100%)
 *    2. code 980, stack 120, v3 cycles 85k (62%)
 *    3. code 1462, stack 144, v3 cycles 63k (46%)
 */
#ifndef QR123_H_
#define QR123_H_

#include <stdint.h>
#include <stdbool.h>
typedef unsigned uint;

#define QR_LINES 29

typedef struct qr_ctx {
   uint8_t size;            // 21, 25 or 29 (ver*4+17)
   uint8_t _mode;           // unused, for future.
   uint8_t _pad;            // unused, padding.
   uint8_t len;             // length of input data.
   const uint8_t *data;     // input data.
   void *params;            // data and ECC parameters.
   uint32_t bmp[QR_LINES];  // QR code bitmap, 1 word per line.
} qr_ctx;

/*
 * Return false if input exceeds the capacity of chosen version.
 *  - Parameters are written to the context.
 *  - Must evaluate before encoding.
 *  - Must fail if evaluation fails.
 *  Capacity: V1 17B, V2 32B, V3 53B.
 */
extern bool qr_eval(qr_ctx *ctx, uint ver, const uint8_t *data, uint len);

/*
 * The actual encoding.
 */
extern void qr_encode(qr_ctx *ctx);

/*
 * Get dots for display.
 */
static inline bool qr_getdot(qr_ctx *ctx, uint x, uint y)
{
   return ctx->bmp[y] << x >> 31;
}

#endif /* QR123_H_ */

/* vim: set syn=c.doxygen cin et sw=3 ts=3 tw=80 fo=croqm: */
