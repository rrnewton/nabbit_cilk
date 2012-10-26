// Code for the Nabbit task graph library
//
// Miscellaneous methods for copying matrices and converting between
// layouts.
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

#ifndef __MATRIX_UTILS_H
#define __MATRIX_UTILS_H

#include <array2d_row.h>
#include <array2d_morton.h>


#define MAX(m, n) ((m) > (n) ? (m) : (n))


/******************************************************/
// Simple matrix functions.

// Fill a 1D array with random elements.
void fill_random_1D(int* a, int length, int range) {
  for (int i = 0; i < length; i++) {
    a[i] = rand() % range;    
  }
}


// Fill a 2D array with random elements.
template <class MatrixType>
void fill_random_2D(MatrixType* M, int range) {
  
  int i = 0;
  for (ArrayLargeDim row_idx = M->row_iterator();
       M->has_next_row(&row_idx);
       M->increment_row(&row_idx)) {
    int j = 0;

    for (ArrayLargeDim col_idx = M->col_iterator();
	 M->has_next_col(&col_idx);
	 M->increment_col(&col_idx)) {

      int tval = rand() % range;

      M->idx_set(&row_idx,
		 &col_idx,
		 tval);
      assert(M->get(i, j) == tval);
      j++;
    }
    assert(j == M->get_width());
    i++;
  }
  assert(i == M->get_height());
}


// Copy from one matrix to another; the two matrices can use
// different layouts.
template <class MatrixType1, class MatrixType2>
void copy_2D(MatrixType1* M, MatrixType2* M2) {

  assert(M->get_width() == M2->get_width());
  assert(M->get_height() == M2->get_height());
  
  int i = 0;
  for (ArrayLargeDim row_idx = M->row_iterator();
       M->has_next_row(&row_idx);
       M->increment_row(&row_idx)) {
    int j = 0;

    for (ArrayLargeDim col_idx = M->col_iterator();
	 M->has_next_col(&col_idx);
	 M->increment_col(&col_idx)) {

      int tval = M->idx_get(&row_idx,
			    &col_idx);
      M2->set(i, j, tval);
      j++;
    }
    assert(j == M->get_width());
    i++;
  }
  assert(i == M->get_height());
}


// Sets zero'th row and columns to 0.
template <class MatrixType>
void zero_top_and_left_borders(MatrixType* M) {
  for (int j = 0; j < M->get_width(); j++) {
    M->set(0, j, 0);
  }
  for (int i = 0; i < M->get_height(); i++) {
    M->set(i, 0, 0);
  }
}


#endif
