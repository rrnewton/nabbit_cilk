// Nabbit, Array Indexing Library
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

#ifndef __ARRAY_LAYOUTS_H_
#define __ARRAY_LAYOUTS_H_

#include <assert.h>
#include <stdint.h>
#include "nabbit_array_defs.h"
#include "morton.h"


/******************************************************************
 * This file defines the BlockLayout struct, which describes possible
 * memory layouts for an n by m array.  It also contains the
 * definitions of functions for indexing these arrays.
 * 
 * We consider combinations of the following layouts
 * 
 * ROW_MAJOR_LAYOUT:  rows are contiguous
 * COL_MAJOR_LAYOUT:  columns are contiguous
 *
 * MORTON_LAYOUT: a bit-interleaved layout: the bits of the 
 *                row and column index are interleaved.
 *                This layout may consumes extra memory 
 *                (at least virtual memory) because 
 *                array dimensions must be padded out to the
 *                largest power of 2 bigger than the largest dimension. 
 *
 *                Mostly good for square matrices only, since for
 *                rectangular matrices, this would square the space
 *                used.  (e.g., n by m, but m is small).
 *
 *                In fact, one could define a "packed morton-order"
 *                layout which only rounds each dimension up to its
 *                nearest power of 2 (only multiples the space by up
 *                to 4).  But the indexing becomes more complicated
 *                because you also need to keep 
 * 
 *           

 *
 * Suppose we have a 2d matrix, n = 3, m = 3:
 *       0    1   2
 *       10  11  12
 *       20  21  22
 *
 * The arrangement of numbers in memory are:
 * 
 * ROW_MAJOR_LAYOUT:
 *       0  1  2 10 11 12 20 21 22
 *
 * COL_MAJOR_LAYOUT:
 *       0 10  20 1 11 21  2 12 22
 *
 * MORTON_LAYOUT: (requires 4 by 4 block)
 *  (a-g are padding elements)
 * 
 *       0    1   2  c
 *       10  11  12  d
 *       20  21  22  f
 *        a   b   e  g
 *      
 *       0 10 1 11   20 a 21 b   2 12 c d   22 e f g
 */

 
typedef enum { ROW_MAJOR_LAYOUT=0,
	       COL_MAJOR_LAYOUT=1,
	       MORTON_LAYOUT=2,
	       OTHER_LAYOUT=3
	       // The last layout is here so we can iterate over all layouts more easily.
} NabbitArray2DLayout;

// The strings for the names of the layouts.
const char* Nabbit2DLayoutStrings[4] =
  { "ROW_MAJOR_LAYOUT",
    "COL_MAJOR_LAYOUT",
    "MORTON_LAYOUT",
    "OTHER_LAYOUT" };



// A class for translating between a row/column
// index (i, j) and an array index.
// 
// layout_type should be one of the types defined in
// the enum above.
//
template <int layout_type>
class NabbitLayoutIndexing {

 public:

  // Translates (i, j) into an array index.
  // For now, we assume that if i and j take up W bits, then
  // ArrayLargeDim is 2W bits.
  //
  // For long or tall matrices, we might have to do something
  // different.
  //
  static inline ArrayLargeDim get_idx(ArrayDim i, ArrayDim j,
				      ArrayLargeDim Aparam);

  // Extracts the row from the index.
  static inline ArrayDim get_row(ArrayLargeDim idx,
				 ArrayLargeDim Aparam);

  // Extracts the column from the index.
  static inline ArrayDim get_col(ArrayLargeDim idx,
				 ArrayLargeDim Aparam);

  // Returns the increment we need to add to
  // move down by "bsize" rows from the current position (i, j).
  // In other words, it computes index(i+bsize, j) - index(i, j).
  //
  // Aparam is the layout-specific information which encodes both the
  // current position (i, j) and also allows us to shift by row and
  // column.
  //
  // For a row-major layout, Aparam is the separation between rows.
  // For a col-major layout, Aparam is the separation between columns.
  // 
  // For a Morton layout, Aparam is the Morton index of (i, j).
  // 
  static inline ArrayLargeDim block_row_inc(ArrayDim bsize,
					    ArrayLargeDim Aparam);

  // Moves "bsize" columns over from current position (i, j).
  static inline ArrayLargeDim block_col_inc(ArrayDim bsize,
					    ArrayLargeDim Aparam);
  
  // Returns the increment we need to add to
  // move down by "bsize" rows from (0, 0).
  //
  // Aparam is the layout-specific information needed to
  // compute this value.
  //
  // For a row-major layout, Aparam is the separation between rows.
  // For a col-major layout, Aparam is the separation between columns.
  //
  // For a morton layout, Aparam is 0
  // (the value is actually ignored).
  //
  // This method may be more efficient to use than
  // a generic block_row_inc when the programmer knows enough about
  // "bsize" to exploit the symmetries of the layout.
  //
  static inline ArrayLargeDim aligned_block_row_inc(ArrayDim bsize,
						    ArrayLargeDim Aparam);

  // Similar method for moving "bsize" columns over from (0, 0).
  static inline ArrayLargeDim aligned_block_col_inc(ArrayDim k,
						    ArrayLargeDim Aparam);

  // TODO: For Morton layouts we really want a method which
  // moves over by blocks whose sizes are powers of 2, and it could be slightly more 
  // efficient to pass around lg(bsize)...
};


class NabbitIndexingWrapper {

 public:
  // The same methods as above, except that they don't require a template argument.
  // Thus, we can call these when the layout is determined at runtime.
  // These functions end up wrappying the calls to templated 
  // methods described above within a switch statement.

  static inline ArrayLargeDim aligned_block_row_inc(ArrayDim bsize,
						    ArrayLargeDim Aparam,
						    NabbitArray2DLayout ltype);
  static inline ArrayLargeDim aligned_block_col_inc(ArrayDim bsize,
						    ArrayLargeDim Aparam,
						    NabbitArray2DLayout ltype);
  
  static inline ArrayLargeDim get_idx(ArrayDim si,
				      ArrayDim sj,
				      ArrayDim sub_n,
				      ArrayDim sub_m,
				      NabbitArray2DLayout inner_layout);

};




#define RowMajorRowInc(k, rowsep) ((rowsep)*(k))
#define RowMajorColInc(k, rowsep) (k)

#define ColMajorRowInc(k, colsep) (k)
#define ColMajorColInc(k, colsep) ((colsep)*(k))


// Compute index from row and column.
template <>
inline ArrayLargeDim
NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_idx(ArrayDim i,
						ArrayDim j,
						ArrayLargeDim Aparam) {
  return (Aparam*i + j);
}

template <>
inline ArrayLargeDim
NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_idx(ArrayDim i,
						ArrayDim j,
						ArrayLargeDim Aparam) {
  return (i + Aparam*j);
}

template <>
inline ArrayLargeDim
NabbitLayoutIndexing<MORTON_LAYOUT>::get_idx(ArrayDim i,
					     ArrayDim j,
					     ArrayLargeDim Aparam) {
  return MortonIndexing::get_idx(i, j);
}


// Compute row from index.
template <>
inline ArrayDim
NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_row(ArrayLargeDim idx,
						ArrayLargeDim Aparam) {
  return idx / Aparam;
}

template <>
inline ArrayDim
NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_row(ArrayLargeDim idx,
						ArrayLargeDim Aparam) {
  return idx % Aparam;
}

template <>
inline ArrayDim
NabbitLayoutIndexing<MORTON_LAYOUT>::get_row(ArrayLargeDim idx,
					     ArrayLargeDim Aparam __attribute__((__unused__))) {
  return MortonIndexing::get_row(idx);
}



// Compute column from index.
template <>
inline ArrayDim
NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_col(ArrayLargeDim idx,
						ArrayLargeDim Aparam) {
  return idx % Aparam;
}

template <>
inline ArrayDim
NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_col(ArrayLargeDim idx,
						ArrayLargeDim Aparam) {
  return idx / Aparam;
}

template <>
inline ArrayDim
NabbitLayoutIndexing<MORTON_LAYOUT>::get_col(ArrayLargeDim idx,
					     ArrayLargeDim Aparam __attribute__((__unused__))) {
  return MortonIndexing::get_col(idx);
}



template <>
inline ArrayLargeDim
NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::aligned_block_row_inc(ArrayDim bsize,
							      ArrayLargeDim Aparam) {
  return RowMajorRowInc(bsize, Aparam);
}

template <>
inline ArrayLargeDim
NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::aligned_block_col_inc(ArrayDim bsize,
							      ArrayLargeDim Aparam) {
  return RowMajorColInc(bsize, Aparam);
}

template <>
ArrayLargeDim
NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::aligned_block_row_inc(ArrayDim bsize,
							      ArrayLargeDim Aparam) {
  return ColMajorRowInc(bsize, Aparam);
}

template <>
ArrayLargeDim
NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::aligned_block_col_inc(ArrayDim bsize,
							      ArrayLargeDim Aparam) {
  return ColMajorColInc(bsize, Aparam);
}

template <>
ArrayLargeDim
NabbitLayoutIndexing<MORTON_LAYOUT>::aligned_block_row_inc(ArrayDim bsize,
							   ArrayLargeDim Aparam __attribute__((__unused__))) {
  return MortonIndexing::get_idx(bsize, 0);
}

template <>
ArrayLargeDim
NabbitLayoutIndexing<MORTON_LAYOUT>::aligned_block_col_inc(ArrayDim bsize,
							   ArrayLargeDim Aparam __attribute__((__unused__))) {
  return MortonIndexing::get_idx(0, bsize);
}



ArrayLargeDim NabbitIndexingWrapper::aligned_block_row_inc(int k,
							   ArrayLargeDim Aparam,
							   NabbitArray2DLayout ltype) {
  switch (ltype) {
  case ROW_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::aligned_block_row_inc(k, Aparam);
    break;
  case COL_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::aligned_block_row_inc(k, Aparam);
    break;
  case MORTON_LAYOUT:
    return NabbitLayoutIndexing<MORTON_LAYOUT>::aligned_block_row_inc(k, Aparam);
    break;
  default:
    assert(0);
  }
  assert(0);
  return -1;
}

ArrayLargeDim NabbitIndexingWrapper::aligned_block_col_inc(int k,
							   ArrayLargeDim Aparam,
							   NabbitArray2DLayout ltype) {
  switch (ltype) {
  case ROW_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::aligned_block_col_inc(k, Aparam);
    break;
  case COL_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::aligned_block_col_inc(k, Aparam);
    break;
  case MORTON_LAYOUT:
    return NabbitLayoutIndexing<MORTON_LAYOUT>::aligned_block_col_inc(k, Aparam);
    break;
  default:
    assert(0);
  }
  assert(0);
  return -1;
}




ArrayLargeDim NabbitIndexingWrapper::get_idx(ArrayDim si,
					     ArrayDim sj,
					     ArrayDim sub_n,
					     ArrayDim sub_m,
					     NabbitArray2DLayout inner_layout) {
  switch (inner_layout) {
  case ROW_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<ROW_MAJOR_LAYOUT>::get_idx(si, sj, sub_m);   

  case COL_MAJOR_LAYOUT:
    return NabbitLayoutIndexing<COL_MAJOR_LAYOUT>::get_idx(si, sj, sub_n);
    
  case MORTON_LAYOUT:
    return NabbitLayoutIndexing<MORTON_LAYOUT>::get_idx(si, sj, 0);
    
  default:
    assert(0);
  }

  return (ArrayLargeDim)-1;
}


					     


#endif // __ARRAY_LAYOUTS_H_
