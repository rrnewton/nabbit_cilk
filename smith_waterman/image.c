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

//#pragma lang +c

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "image.h"

// For binary images
//const static char PBM_MAGIC[3] = "P4";


// For color images
const static char PPM_MAGIC[3] = "P6";


// Using these functions is going to be REALLY
//  slow, I think...

inline int getPixelIndex(image* x, int u, int v) {
  return (u*(x->rowLength) + v/BYTE_SIZE);
}

inline BYTE getPixel(image* x, int u, int v) {
  BYTE correctByte = x->raw[getPixelIndex(x, u, v)];
  BYTE mask = 1 << (BYTE_SIZE - 1 - (v % BYTE_SIZE));

  return mask & correctByte;
}

inline void setPixel(image* x, int u, int v, int val) {
  int ind = getPixelIndex(x, u, v);
  BYTE mask = 1 << (BYTE_SIZE - 1 - (v% BYTE_SIZE));

  //  printf("For (%d, %d), pixel index is %d\n", u, v, ind);

  if (val) {
    x->raw[ind] = x->raw[ind] | mask;    
  }
  else {
    x->raw[ind] = x->raw[ind] & (~mask);
  }
}

image* createImage(int width, int height) {

  int rowLength;
  image* theImage;

  if ((width <= 0) || (height <= 0)) {
    return NULL;
  }

  rowLength = (width / BYTE_SIZE + ((width % BYTE_SIZE != 0))) ;
  // Number of bytes in the row.

  theImage = (image*)malloc(sizeof(image));
  assert(theImage != NULL);

  theImage->width = width;
  theImage->height = height;
  theImage->rowLength = rowLength;

  theImage->imageSize = (theImage->rowLength * theImage->height);

  // I want a zero image to start with. :)
  //  theImage->raw = (BYTE*)malloc(theImage->imageSize);

  theImage->raw = (BYTE*)calloc(theImage->rowLength * theImage->height, 1);
  assert(theImage->raw != NULL);

  return theImage;    
}


void destroy_image(image* b) {
  free(b->raw);
  free(b);
}

// Creates a copy of the existing image
image* copyImage(image* x) {
  
  image* y;
  assert(x != NULL);

  y = createImage(x->width, x->height);

  y->width = x->width;
  y->rowLength = x->rowLength;
  y->height = x->height;
  y->imageSize = x->imageSize;

  memcpy(y->raw, x->raw, y->imageSize);

  return y;
}

image* createTranspose(image* x) {
  image* y;
  int i, j;

  assert(x != NULL);

  y = createImage(x->height, x->width);
 
  // This is the really slow way to do transpose...
  for (i = 0; i < x->height; i++) {
    for (j = 0; j < x->width; j++) {
      setPixel(y, j, i, getPixel(x, i, j));
    }
  }
  return y;
}

// This is still not that smart...
image* smartTranspose(image* x) {
  image* y;
  int i, j;
  unsigned int currentMask = 0;
  assert(x != NULL);

  y = createImage(x->height, x->width);


  // i loops through rows
  for (i = 0; i < x->height; i++) {

    // j loops through the representation in that row.
    for (j = 0; j < x->rowLength - 1; j++) {
      int z = 0;
      
      currentMask = x->raw[i*x->rowLength + j];

      // scan through x, set the pixel in y.
      while (currentMask != 0) {
	z++;
	setPixel(y, j*BYTE_SIZE +(BYTE_SIZE-z), i, currentMask & 1);
	currentMask  = (currentMask >> 1);
      }      
    }

    {
      // This is how many bits we shoudl ignore.
      int z = x->rowLength * BYTE_SIZE - x->width;
      //      printf("We are ignoring %d bits at the end.\n", z);

      j = x->rowLength - 1;
      currentMask = x->raw[i*x->rowLength + j];
      // Shift away the bits we are ignoring:
      currentMask = currentMask >> z;
      while (currentMask != 0) {
	z++;	
	//	printf("Special processing for pixel at row %d, pixel %d\n", i, z);
	setPixel(y, j*BYTE_SIZE +(BYTE_SIZE-z), i, currentMask & 1);
	currentMask  = (currentMask >> 1);
      }
    }
  }

  return y;
}


int equalImages(image* x, image* y) {

  rtimeStruct t1, t2;
  int equal = TRUE;

  imageCycleCounter(&t1);
  equal = (equal  &&
	   (x->height == y->height) &&
	   (x->width == y->width) &&
	   (x->rowLength == y->rowLength) &&
	   (x->imageSize == y->imageSize));

  equal = equal && (memcmp(x->raw, y->raw, x->imageSize) == 0);

  imageCycleCounter(&t2);


  if (!equal) {
    int i, j;

    for (i = 0; i < x->height; i++) {
      for (j = 0; j < x->width; j++) {
	if (getPixel(x, i, j) != getPixel(y, i, j)) {
	  printf("Images x and y differ at (%d, %d)\n", i, j);
	}
      }
    }
  }
  printf("Time for equality test: %llu \n", t2-t1);
  
  return equal;	     
}


void save_image_to_file(const char* fileName, image* b) {

  size_t bytesWritten;
  char imageHeader[200];
  int imageSize;
  int fd;

  assert(fileName != NULL);
  fd = creat(fileName, 0666);
  assert(fd != -1);

  imageSize = (b->rowLength * b->height);
  
  snprintf(imageHeader, 200, "%s\n%d\n%d\n", PBM_MAGIC, b->width, b->height);
  //  printf("How long is the image header? %d\n", strlen(imageHeader));

  bytesWritten = 0;
  while (bytesWritten != strlen(imageHeader)) {
    bytesWritten =  write(fd, imageHeader, strlen(imageHeader));
  }


  bytesWritten = 0;
  while (bytesWritten != (size_t)imageSize) {
    bytesWritten = write(fd, b->raw, imageSize);
  }


  close(fd);   
}




static int get_int(FILE* f ) { /* scan int from ppm, passing over comments */
  int k;
  unsigned char ch;
  do {
    while( isspace(ch=getc(f)) ) ;
    if( ch=='#' ) while( (ch=getc(f))!='\n' ) ;
  } while( !isdigit(ch) );
  for( k=(ch-'0'); isdigit(ch=getc(f)); k = 10*k+(ch-'0') );
  return k;
}


image* load_image_from_file(const char* fileName) {
  FILE* f;
  image* theImage;


  char magic[3]="ZZ";


  assert(fileName != NULL);
  f = fopen(fileName, "r");
  

  fread(magic,2,1,f);
  if( strcmp(magic,PBM_MAGIC)==0 ) {

    //    printf("WE have a pbm magic here. \n");

    theImage = (image*)malloc(sizeof(image));
    theImage->width = get_int(f);
    theImage->height = get_int(f);

    if ( (theImage->width < 0) || (theImage->height < 0)) {
      fprintf(stderr, "Error! width is %d, height = %d is invalid.\n", theImage->width, theImage->height);
      assert(FALSE);
    }
    
    theImage->rowLength = (theImage->width / BYTE_SIZE + ((theImage->width % BYTE_SIZE) != 0));
    theImage->imageSize = theImage->rowLength * theImage->height;
    theImage->raw = (BYTE*)malloc(theImage->imageSize);


    {
      int numRows = 0;
      while (numRows == 0) {
	//	printf("What is the width? %d row length? %d.  Height? %d\n",
	//	       theImage->width,
	//	       theImage->rowLength,
	//	       theImage->height);
	numRows = fread(theImage->raw, theImage->rowLength, theImage->height,  f);
	//	printf("numrows we got is %d\n",numRows);
      }
    }
    
    //    printf("done with all this stuff.\n");

    //    printf("What size image did we get? %d by %d\n", theImage->width, theImage->height);

    //    x->width=get_int(); x->height=get_int(); x->max=get_int();
    //    x->bpp = (x->max<256) ? 1 : 2;
    //    x->raster = x->width * x->bpp * 3;
    //    if( flag=alloc_ppm(x) )
    //      fread( x->raw, x->height, x->raster, stdin );
  }
  else {
    fprintf(stderr, "This is not a valid binary PPM (magic # is not %s)\n", PBM_MAGIC);
    theImage = NULL;
  }


  fclose(f);
  return theImage;
}





/*************************************************/
// Color image stuff.  It looks almost the same...

color_image* create_color_image(int width, int height, int max) {

  int bytesPerColor;
  color_image* theImage;

  if ((width <= 0) || (height <= 0)) {
    return NULL;
  }

  if (max < 256) {
    bytesPerColor = 1;
  }
  else {
    if (max > 256*256 - 1) {
      max = 256*256 - 1;
    }
    bytesPerColor = 2;
  }


  theImage = (color_image*)malloc(sizeof(color_image));

  assert(theImage != NULL);
  theImage->width = width;
  theImage->height = height;
  theImage->max = max;

  theImage->rowLength = width * 3 * bytesPerColor;
  theImage->imageSize = theImage->rowLength * theImage->height;


  theImage->raw = (BYTE*)calloc(theImage->height, theImage->rowLength);
  assert(theImage->raw != NULL);

  return theImage;    
}


void destroy_color_image(color_image* b) {
  free(b->raw);
  free(b);
}



void save_color_image_to_file(const char* fileName, color_image* b) {

  size_t bytesWritten;
  char imageHeader[200];
  int imageSize;
  int fd;

  assert(fileName != NULL);
  fd = creat(fileName, 0666);
  assert(fd != -1);

  imageSize = (b->rowLength * b->height);
  
  snprintf(imageHeader, 200, "%s\n%d\n%d\n%d\n",
	   PPM_MAGIC,
	   b->width,
	   b->height,
	   b->max);
  //  printf("How long is the image header? %zd\n", strlen(imageHeader));

  bytesWritten = 0;
  while (bytesWritten != strlen(imageHeader)) {
    bytesWritten =  write(fd, imageHeader, strlen(imageHeader));
  }


  bytesWritten = 0;
  while (bytesWritten != (size_t)imageSize) {
    bytesWritten = write(fd, b->raw, imageSize);
  }


  close(fd);   
}



color_image* load_color_image_from_file(const char* fileName) {
  FILE* f;
  color_image* theImage;


  char magic[3]="ZZ";


  assert(fileName != NULL);
  f = fopen(fileName, "r");
  

  fread(magic,2,1,f);
  if( strcmp(magic,PPM_MAGIC)==0 ) {
    int numBytesPerColor = 1;

    //    printf("WE have a ppm magic here. \n");

    theImage = (color_image*)malloc(sizeof(color_image));
    assert(theImage != NULL);
    theImage->width = get_int(f);
    theImage->height = get_int(f);
    theImage->max = get_int(f);

    if ( (theImage->width < 0) || (theImage->height < 0)) {
      fprintf(stderr, "Error! width is %d, height = %d is invalid.\n", theImage->width, theImage->height);
      assert(FALSE);
    }


    if (theImage->max > 256) {
      assert(theImage->max <= 65536);
      numBytesPerColor = 2;      
    }

    

    theImage->rowLength = theImage->width * 3 * numBytesPerColor;
    theImage->imageSize = theImage->rowLength * theImage->height;

    theImage->raw = (BYTE*)malloc(theImage->imageSize);


    {
      int numRows = 0;
      while (numRows == 0) {
	//	printf("What is the width? %d row length? %d.  Height? %d\n",
	//	       theImage->width,
	//	       theImage->rowLength,
	//	       theImage->height);
	numRows = fread(theImage->raw, theImage->rowLength, theImage->height,  f);
	//	printf("numrows we got is %d\n",numRows);
      }
    }
    
    //    printf("done with all this stuff.\n");

    //    printf("What size image did we get? %d by %d\n", theImage->width, theImage->height);

    //    x->width=get_int(); x->height=get_int(); x->max=get_int();
    //    x->bpp = (x->max<256) ? 1 : 2;
    //    x->raster = x->width * x->bpp * 3;
    //    if( flag=alloc_ppm(x) )
    //      fread( x->raw, x->height, x->raster, stdin );
  }
  else {
    fprintf(stderr, "This is not a valid binary PPM (magic # is not %s)\n", PPM_MAGIC);
    theImage = NULL;
  }

  fclose(f);
  return theImage;
}



color_image* transpose_color_image(color_image* x) {
  
  int yIndex, xIndex;
  int numBytesPerColor;
  int i, j;
  color_image* y;
  
  y = create_color_image(x->height, x->width, x->max);

  if (x->max > 256) {
    numBytesPerColor = 2;
  }
  else {
    numBytesPerColor = 1;
  }

  //  printf("what is the width and height? %d %d\n", x->width, x->height);
  // This is the really slow way to do transpose...
  for (i = 0; i < x->height; i++) {
    for (j = 0; j < x->width; j++) {

      xIndex = (i * x->rowLength) + 3*numBytesPerColor * j;
      yIndex = (j * y->rowLength) + 3*numBytesPerColor * i;

      //      printf("(%d, %d): xIndex = %d, yIndex = %d\n", i, j, xIndex, yIndex);
      memcpy(&y->raw[yIndex],
	     &x->raw[xIndex],
	     3*numBytesPerColor);	           
    }
  }
  return y;
}

void get_color_pixel(color_image* x, int u, int v, int* rgb_array, int max) {
  int numBytesPerColor;
  int ind;

  assert(max == 255);
  if (u >= x->height) {
    printf("u = %d. x->height = %d\n",
	   u, x->height);
  }
  assert(u < x->height);
  if (v >= x->width) {
    printf("v = %d. x->width = %d\n",
	   v, x->width);
  }
  assert(v < x->width);
  
  numBytesPerColor = (x->max > 256) ? 2 : 1;
  ind = u * x->rowLength  + 3*numBytesPerColor*v;  

  rgb_array[0] = x->raw[ind];
  rgb_array[1] = x->raw[ind+1];
  rgb_array[2] = x->raw[ind+2];
}



void set_color_pixel(color_image* x, int u, int v, int R, int G, int B) {

  int numBytesPerColor;
  int ind;


  if (u >= x->height) {
    printf("u = %d. x->height = %d\n",
	   u, x->height);
  }
  assert(u < x->height);
  if (v >= x->width) {
    printf("v = %d. x->width = %d\n",
	   v, x->width);
  }
  assert(v < x->width);
  
  numBytesPerColor = (x->max > 256) ? 2 : 1;
  ind = u * x->rowLength  + 3*numBytesPerColor*v;
  

  if (numBytesPerColor == 2) {
    x->max = 256*256 - 1;
    
    x->raw[ind] = (R/256)%256;
    x->raw[ind+1] = R % 256;

    x->raw[ind+2] = (G/256)%256;
    x->raw[ind+3] = G % 256;

    x->raw[ind+4] = (B/256)%256;
    x->raw[ind+5] = B % 256;
  }
  else {
    x->max = 256 - 1;
    x->raw[ind] = R % 256;
    x->raw[ind+1] = G % 256;
    x->raw[ind+2] = B % 256;
  }
}


void set_image_color_for_proc(color_image* img,
			      int i, int j,
			      int p) {
  int R, G, B;
  assert(p >= -1);
  assert((p-1) < maxP);
  R = color_map[3*(p+1)];
  G = color_map[3*(p+1)+1];
  B = color_map[3*(p+1)+2];
  set_color_pixel(img,
		  i, j,
		  R, G, B);
}


int equal_color_images(color_image* x, color_image* y) {

  rtimeStruct t1, t2;
  int equal = TRUE;

  imageCycleCounter(&t1);
  equal = (equal  &&
	   (x->height == y->height) &&
	   (x->width == y->width) &&
	   (x->max == y->max) &&
	   (x->rowLength == y->rowLength) &&
	   (x->imageSize == y->imageSize));

  equal = equal && (memcmp(x->raw, y->raw, x->imageSize) == 0);

  imageCycleCounter(&t2);
  printf("Time for color equality test: %llu \n", t2-t1);
  
  return equal;	     
}
