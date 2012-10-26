// Nabbit, Array Indexing Library
//
// Copyright (c) 2010 Jim Sukha
//
/*
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __NABBIT_ARRAY_DEFS_H_
#define __NABBIT_ARRAY_DEFS_H_

#include <assert.h>
#include <stdint.h>

typedef int8_t ArrayLgDim;
typedef int32_t ArrayDim;
typedef uint64_t ArrayLargeDim;

/******************************************************************/
// Define some simple math functions used when calculating indexing.

#define NABBIT_MAX(a, b) ((a) > (b) ? (a) : (b))
#define NABBIT_MIN(a, b) ((a) > (b) ? (a) : (b))



// Returns the smallest power of 2 which is >= n.
static inline ArrayLargeDim hyperceil(ArrayLargeDim n) {
  ArrayLargeDim x = n-1;
  x = x | (x >> 1);
  x = x | (x >> 2);
  x = x | (x >> 4);
  x = x | (x >> 8);
  x = x | (x >> 16);
  x = x | (x >> 32);
  return x+1;

  /*   uint64_t ans = 1; */
  /*   while ((ans < n) && (ans > 0)) { */
  /*     ans <<=1; */
  /*   } */
  /*   return (ArrayLargeDim)ans; */
}

// Returns the largest power of 2 which is <= n.
static inline ArrayLargeDim hyperfloor(ArrayLargeDim n) {
  int pos = 0;
  if (n >= 1L<<32) { n >>= 32; pos += 32; }
  if (n >= 1L<<16) { n >>= 16; pos += 16; }
  if (n >= 1L<< 8) { n >>=  8; pos +=  8; }
  if (n >= 1L<< 4) { n >>=  4; pos +=  4; }
  if (n >= 1<< 2) { n >>=  2; pos +=  2; }
  if (n >= 1<< 1) {           pos +=  1; }
  return ((n == 0) ? 0 : (1 << pos));
}

// Returns lg(hyperfloor(n))
static inline int floor_lg(uint64_t n) {
  int pos = 0;
  if (n >= 1L<<32) { n >>= 32; pos += 32; }
  if (n >= 1L<<16) { n >>= 16; pos += 16; }
  if (n >= 1L<< 8) { n >>=  8; pos +=  8; }
  if (n >= 1L<< 4) { n >>=  4; pos +=  4; }
  if (n >= 1<< 2) { n >>=  2; pos +=  2; }
  if (n >= 1<< 1) {           pos +=  1; }
  return ((n == 0) ? (-1) : pos);
}


// Returns ceil(x/block_size)
static inline ArrayLargeDim block_count(ArrayLargeDim x,
					ArrayLargeDim block_size) {
  return (x / block_size) + ((x % block_size != 0) ? 1 : 0);
}



#endif // __NABBIT_ARRAY_DEFS_H_
