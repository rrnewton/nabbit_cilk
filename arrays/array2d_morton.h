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

#ifndef __ARRAY_2D_
#define __ARRAY_2D_


/**
 * File containing definitions for a Morton-order layout of a 2d
 * array.
 *
 */

// #define DEBUG_PRINT
#include "morton.h"


/**
 * Defines an array whose elements are stored in a Morton-indexed
 * order.
 *
 * This array class allows blocks of size 2^PAD_LEVEL
 * to be padded by adding M_PADDING extra elements.
 *
 * For example, if PAD_LEVEL = 3 and M_PADDING = 6, then every block
 * of size 8 by 8 is padded by 6 elements.
 *
 * One can index into the array by using
 *   (1) row / column indices:  get(i, j)
 *   (2) morton indices:  idx_get(x), where x is the morton index into the array.
 *
 * The implementation actually indexes into the storage array via
 * array indices.
 *  ArrayMortonIndex(x) converts a Morton index x into the array
 *  index.
 * (When PAD_LEVEL = 0 and MPADDING = 0, the two are the same,
 *  otherwise, ArrayMortonIndex(x) > x because of padding).
 *   
 * TODO: I don't know whether the padding is worth the effort to
 * support, or if this approach is a reasonable way to do it.
 * Is the level of indirection expensive?
 * 
 * For now, mostly, I've used only PAD_LEVEL = 0 and MPADDING = 0.
 */
template <class T, uint8_t PAD_LEVEL, int M_PADDING=0>
class NabbitArray2DMorton: public NabbitArray2DBase<T> {

 protected:
  ArrayDim padded_dim;


  // TODO: Currently we don't support any dynamic padding
  // of small blocks in the array.
  // If one changes the source, we support static 
  // padding (through the use of the MPADDING template parameter).
  //  int padding;

  
 public:
 
  NabbitArray2DMorton(ArrayDim width,
		      ArrayDim height);
  
  void fill_with_constant_element(T element);
  void print();
  void print_layout();

  inline ArrayDim get_width() { return this->width; }
  inline ArrayDim get_height() { return this->height; }

  // Index into the array via row and column indices.
  inline T get(ArrayDim i,
	       ArrayDim j);
  
  inline void set(ArrayDim i, ArrayDim j, T val);



  // Access the array via iterators.  Iterators are Morton-order
  // indices (i.e., the bit-interleaving of row and column indices).
 
  inline ArrayLargeDim row_iterator() {
    return MortonIndexing::get_idx(0, 0);
  }
  inline ArrayLargeDim row_iterator(ArrayDim row_num) {
    return MortonIndexing::get_idx(row_num, 0);
  }  
  inline void increment_row(ArrayLargeDim* idx) {
    *idx = MortonIndexing::next_row(*idx);
  }
  inline bool has_next_row(ArrayLargeDim* idx) {
    return ((*idx) < MortonIndexing::get_idx(this->height, 0));
  }
 
  // Converts a row iterator to the row index.
  inline ArrayDim row_idx(ArrayLargeDim* idx) {
    return MortonIndexing::get_row(*idx);
  }

 
  inline ArrayLargeDim col_iterator() {
    return MortonIndexing::get_idx(0, 0);
  }
  inline ArrayLargeDim col_iterator(ArrayDim col_num) {
    return MortonIndexing::get_idx(0, col_num);
  }
  inline void increment_col(ArrayLargeDim* idx) {
    *idx = MortonIndexing::next_col(*idx);
  }
  inline bool has_next_col(ArrayLargeDim* idx) {
    return ((*idx) < MortonIndexing::get_idx(0, this->width));
  }
  
  // Converts the col iterator to the column index.
  inline ArrayDim col_idx(ArrayLargeDim* idx) {
    return MortonIndexing::get_col(*idx);
  }


  // Access via row and column iterator).
  inline T idx_get(ArrayLargeDim* row_idx,
 		   ArrayLargeDim* col_idx) {
    return this->data[ArrayMortonIndex(*row_idx + *col_idx)];    
  }

  inline void idx_set(ArrayLargeDim* row_idx,
		      ArrayLargeDim* col_idx,
		      T val) {
    this->data[ArrayMortonIndex(*row_idx + *col_idx)] = val;
  }


  // Access via a combined iterator (row + column iterator).
  inline T idx_get(ArrayLargeDim* total_idx) {
    return this->data[ArrayMortonIndex(*total_idx)];
  }
  inline void idx_set(ArrayLargeDim* total_idx,
		      T val) {
    this->data[ArrayMortonIndex(*total_idx)] = val;
  }

 private:



  // Constants needed to deal with possible padding of blocks.
  // We are only allowing padding in powers of 2 so we can
  // mask off bits easily.
  static const uint32_t LOW_PAD_MASK = ((1 << PAD_LEVEL)-1);
  static const uint32_t HIGH_PAD_MASK = ((0xFFFFFFFF ^ LOW_PAD_MASK));
  static const uint64_t HIGH_PAD_L_MASK = (((uint64_t)(-1)) ^ LOW_PAD_MASK);

  static inline ArrayLargeDim ArrayMortonIndex(ArrayLargeDim x);

 
  // Returns the size of the array we need to allocate to store all
  // the elements, taking into account the padding we need.
  static inline ArrayLargeDim MortonSize(ArrayDim n);

  
};



template <class T, uint8_t PAD_LEVEL, int M_PADDING>
T NabbitArray2DMorton<T, PAD_LEVEL, M_PADDING>::get(ArrayDim i,
						    ArrayDim j) {
  return this->data[ArrayMortonIndex(MortonIndexing::get_idx(i, j))];
}

template <class T, uint8_t PAD_LEVEL, int M_PADDING>
void NabbitArray2DMorton<T, PAD_LEVEL, M_PADDING>::set(ArrayDim i,
						       ArrayDim j,
						       T val) {
  this->data[ArrayMortonIndex(MortonIndexing::get_idx(i, j))] = val;
}


template <class T, uint8_t PAD_LEVEL, int M_PADDING>
NabbitArray2DMorton<T, PAD_LEVEL, M_PADDING>::NabbitArray2DMorton(ArrayDim test_width,
								  ArrayDim test_height) :
  NabbitArray2DBase<T>(test_width, test_height, MORTON_LAYOUT)
{  
  assert(this->layout == MORTON_LAYOUT);
  assert(test_width == test_height);
  
  ArrayLargeDim num_elements = MortonSize(test_width);
  this->padded_dim = num_elements;
  
  this->total_size = sizeof(T) * num_elements;
  assert(this->total_size > 0);

  this->data = new T[this->total_size];
  assert(this->data != NULL);

#ifdef DEBUG_PRINT
  printf("Creating Morton 2D array with width = %d, height = %d, ",
	 this->width, this->height);
  printf("layout = %d, total_size = %lu\n",
	 this->layout,
	 (size_t)this->total_size);  
#endif  
}



template <class T, uint8_t PAD_LEVEL, int M_PADDING>
void NabbitArray2DMorton<T, PAD_LEVEL, M_PADDING>::fill_with_constant_element(T element) {
  assert(this->data);
  for (ArrayDim i = 0; i < this->height; i++) {
    for (ArrayDim j = 0; j < this->width; j++) {
      set(i, j, element);
    }  
  }
}

template <class T, uint8_t PAD_LEVEL, int M_PADDING>
void NabbitArray2DMorton<T, PAD_LEVEL, M_PADDING>::print() {
  printf("2D Array %p (width = %d, height = %d, ",
	 this, this->width, this->height);
  printf("layout = %d, padded_dim = %d, total_size = %lu)\n",
	 this->layout, this->padded_dim, this->total_size);

  if (this->total_size < 1000) {
    for (ArrayDim i = 0; i < this->width; i++) {
      printf("i = %3d: ", i);
      for (ArrayDim j = 0; j < this->width; j++) {
	printf("%4d ", get(i, j));
      }
      printf("\n");
    }
  }
}


template <class T, uint8_t PAD_LEVEL, int M_PADDING>
void NabbitArray2DMorton<T, PAD_LEVEL, M_PADDING>::print_layout() {
  printf("2D Array %p LAYOUT (width = %d, height = %d, ",
	 this, this->width, this->height);
  printf("layout = %d, padded_dim = %d, total_size = %lu)\n",
	 this->layout, this->padded_dim, this->total_size);
  printf("PAD_LEVEL = %d, M_PADDING = %d\n",
	 PAD_LEVEL, M_PADDING);
   
  if (this->total_size < 1000) {
    for (ArrayLargeDim i = 0; i < this->total_size; i++) {
      printf("(%lu, %d) ", i, this->data[i]);
    }
    printf("\n");
  }
}
  
template <class T, uint8_t PAD_LEVEL, int M_PADDING>
ArrayLargeDim NabbitArray2DMorton<T, PAD_LEVEL, M_PADDING>::ArrayMortonIndex(ArrayLargeDim x) {
  return (x) + (((x & HIGH_PAD_L_MASK)>> PAD_LEVEL) * M_PADDING);  
}

template <class T, uint8_t PAD_LEVEL, int M_PADDING>
ArrayLargeDim NabbitArray2DMorton<T, PAD_LEVEL, M_PADDING>::MortonSize(ArrayDim n) {
  ArrayLargeDim test_size = 1;
  while (test_size < (size_t)n) {
    test_size <<=1;
  }

  return ArrayMortonIndex(MortonIndexing::get_idx(test_size-1,
						  test_size-1) + 1);
}


#endif
