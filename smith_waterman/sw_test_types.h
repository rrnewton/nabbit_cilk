// Code for the Nabbit task graph library
//
// Header file defining the various test types for dynamic
// program example.
//
// Copyright (c) 2010 Jim Sukha
//
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

#ifndef _SW_TEST_TYPES_H_
#define _SW_TEST_TYPES_H_


typedef enum {
  SW_GENERIC = 0,
  SW_DC_K2 = 1,
  SW_DC_GENERIC_K=2,
  SW_PURE_WAVEFRONT=3,
  SW_STATIC_NABBIT=4,
  SW_STATIC_SERIAL=5,
  SW_MAX_TYPE,
} SWComputeType;

static const char* SWTestTypeNames[] = {
  "Generic",
  "DC_K2",
  "DC_GenericK",
  "Wavefront",
  "StaticNabbit",
  "StaticSerial",
};


// Create a name for this test run.
inline static void SWFillTestName(int n, int m, int test_type,
				  char* test_name, int name_length) {
  assert(test_type >= 0);
  assert(test_type < SW_MAX_TYPE);
  snprintf(test_name, name_length,
	   "sw%d_%d_Typ%s",
	   n, m,
	   SWTestTypeNames[test_type]);
}


#endif
