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

#ifndef __ARRAY2D_BASE__
#define __ARRAY2D_BASE__


/******************************************/
// Base class for a simple 2d array.
//
// This class contains only data and a simple initialization method.

#include "array_layouts.h"


template <class T>
class NabbitArray2DBase {
  
 protected:

  ArrayLargeDim total_size;
  ArrayDim width;
  ArrayDim height;
  NabbitArray2DLayout layout;  
  T* data;

 public:

  NabbitArray2DBase(ArrayDim width,
		    ArrayDim height,
		    NabbitArray2DLayout layout);

  ~NabbitArray2DBase();

  inline T* get_data() { return data; }
};


template <class T>
NabbitArray2DBase<T>::NabbitArray2DBase(ArrayDim width,
					ArrayDim height,
					NabbitArray2DLayout the_layout) :
  total_size(0), width(width), height(height), layout(the_layout), data(NULL)
{

}

template <class T>
NabbitArray2DBase<T>::~NabbitArray2DBase() {
/*   printf("Destroying 2D array with width = %d, height = %d, ", */
/* 	 this->width, this->height); */
/*   printf("layout = %d, total_size = %lu\n", */
/* 	 this->layout, (size_t)this->total_size); */
  assert(data != NULL);
  delete [] this->data;
}
  


#endif

