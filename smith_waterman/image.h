// Code for the Nabbit task graph library
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

// Simple code for saving / loading .ppm files


#ifndef __IMAGE_H
#define __IMAGE_H

#define __restrict
#include <assert.h>

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define TRUE 1
#define FALSE 0


const static char PBM_MAGIC[3] = "P4";

enum {BYTE_SIZE = 8};


typedef unsigned char BYTE;

typedef struct binary_image_struct {
  int width;
  int height;


  // These two fields are function of the previous categories, 
  //  but its a pain to keep recalculating them..

  int rowLength;  // Rounded up to next multiple of 8 bits
  size_t imageSize;   // The number of bytes in the image size...

  BYTE* raw;
} image;


typedef struct color_image_struct {
  int width;
  int height;
  int max;
  

  int rowLength;
  size_t imageSize;

  BYTE* raw;
} color_image;

typedef  unsigned long long rtimeStruct;

static inline void imageCycleCounter(rtimeStruct* tv) {
/*   // The 32-bit version: */
/*   unsigned long long rt0; */
/*   __asm__ __volatile__("rdtsc" : "=A" (rt0)); */
/*   *tv = rt0; */

  //  The 64-bit version:  We don't really need it I
  unsigned int low,high;
  __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
  *tv = (((unsigned long long)high)<<32)+low;
}


// Color map which works for up to maxP processor values.
const int maxP = 18;
static const int color_map[3*(maxP+2)] = {
  255, 255, 255,   // -1  White
  0, 0, 0,         // 0   Black
  255, 0, 0,       // 1   Red,
  0, 192, 0,       // 2   Green
  0, 0, 255,       // 3   Blue,
  255, 255, 0,     // 4   Yellow,
  255, 0, 255,     // 5   Magenta
  0, 255, 255,     // 6   Cyan
  0, 0, 139,       // 7   Dark blue, 
  169, 169, 169,   // 8   Dark grey
  139, 0, 0,       // 9   Dark red
  0, 100, 0,       // 10  Dark green
  189, 183, 107,   // 11  Dark Khaki
  0, 139, 139,     // 12  Dark cyan
  139, 0, 139,     // 13  Dark Magenta,
  211, 211, 211,   // 14  light grey
  135, 206, 235,   // 15  sky blue
  102, 205, 170,   // 16  medium aquamarine, 
  259, 235, 215,   // 17  Antique white
  218, 165, 32,    // 18  goldenrod
};



// Using these functions is going to be REALLY
//  slow, I think...

inline int getPixelIndex(image* x, int u, int v);
inline int getRowNum(image* x, int i);
inline int getColNum(image* x, int i);

inline BYTE getPixel(image* x, int u, int v);
inline void setPixel(image* x, int u, int v, int val);




image* createImage(int width, int height);
void destroy_image(image* b);


// Creates a copy of the existing image
image* copyImage(image* x);
image* createTranspose(image* x);

// This is still not that smart...
image* smartTranspose(image* x);


int equalImages(image* x, image* y);



void save_image_to_file(const char* fileName, image* b);
image* load_image_from_file(const char* fileName);


color_image* create_color_image(int width, int height, int max);
color_image* load_color_image_from_file(const char* fileName);
void destroy_color_image(color_image* b);


void save_color_image_to_file(const char* fileName, color_image* b);
color_image* transpose_color_image(color_image* x);
int equal_color_images(color_image* x, color_image* y);


void get_color_pixel(color_image* x, int u, int v, int* rgb_array, int max);
void set_color_pixel(color_image* x, int u, int v, int R, int G, int B);

void set_image_color_for_proc(color_image* img,
			      int i, int j,
			      int p);


#endif
