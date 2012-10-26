// Code for the Nabbit task graph library
//
// Dynamic Programming benchmark.
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


#ifndef __SW_COMPUTE_EF_H
#define __SW_COMPUTE_EF_H


#include <reducer_max.h>


#include <array2d_row.h>
#include <array2d_morton.h>
#include "matrix_utils.h"


/**************************************************************/
// Various methods for computing  Eij and Fij. 
//

// These methods should work with both row-major and morton layouts.
template <class MatrixType>
int computeEij_getters(int* gamma,
		       MatrixType* M,
		       int i, int j);
template <class MatrixType>
int computeFij_getters(int* gamma,
		       MatrixType* M,
		       int i, int j);
template <class MatrixType>
int computeEij_iterator(int* gamma,
			MatrixType* M,
			int i, int j);
template <class MatrixType>
int computeFij_iterator(int* gamma,
			MatrixType* M,
			int i, int j);

template <class MatrixType>
int computeEij_reducer(int* gamma,
		       MatrixType* M,
		       int i, int j);
// I haven't implemented the computeFij yet.

template <class MatrixType, int base_size>
int computeEij_parallel_divide_and_conquer(int* gamma,
					   MatrixType *M,
					   int i, int j,
					   int start_i, int end_i);
template <class MatrixType, int base_size>
int computeFij_parallel_divide_and_conquer(int* gamma,
					   MatrixType *M,
					   int i, int j,
					   int start_i, int end_i);
template <class MatrixType>
int computeE_dummy(int* gamma,
		   MatrixType* M,
		   int i, int j);
template <class MatrixType>
int computeF_dummy(int* gamma,
		   MatrixType* M,
		   int i, int j);




/**************************************************************/
// The "final" methods for computing Eij and Fij that we are using as
// defaults.

#ifndef COMPUTE_CONSTANT_EF

inline int computeEij(int* gamma,
		      NabbitArray2DRowMajor<int>* M,
		      int i, int j) {
  return computeEij_getters(gamma, M, i, j);
}

inline int computeFij(int* gamma,
		      NabbitArray2DRowMajor<int>* M,
		      int i, int j) {
  return computeFij_getters(gamma, M, i, j);  
}

template <uint8_t PAD_LEVEL, int M_PADDING>
inline int computeEij(int* gamma,
		      NabbitArray2DMorton<int, PAD_LEVEL, M_PADDING> *M,
		      int i, int j) {
  return computeEij_iterator(gamma, M, i, j);
}

template <uint8_t PAD_LEVEL, int M_PADDING>
inline int computeFij(int* gamma,
		      NabbitArray2DMorton<int, PAD_LEVEL, M_PADDING>* M,
		      int i, int j) {
  return computeFij_iterator(gamma, M, i, j);
}

#else
// The O(n^2) DP does only constant work to compute E and F values.
template <class MatrixType>
inline int computeEij(int* gamma,
		      MatrixType* M,
		      int i, int j) {
  //  return computeE_constant(gamma, M, i, j);
  return computeE_no_scan(gamma, M, i, j);
}

template <class MatrixType>
inline int computeFij(int* gamma,
		      MatrixType* M, 
		      int i, int j) {
  //  return computeF_constant(gamma, M, i, j);
  return computeF_no_scan(gamma, M, i, j);
}
#endif





/**************************************************************/
// The actual implementations


// Computation of Eij and Fij which uses getters
template <class MatrixType>
int computeEij_getters(int* gamma,
		       MatrixType* M,
		       int i, int j) {
  int gamma_val = gamma[i];
  int max_val = M->get(0, j) + gamma_val;
  for (int k_i = 1; k_i <= i-1; k_i++) {
    int test_val = M->get(k_i, j) + gamma[i-k_i];
    if (test_val > max_val) {
      max_val = test_val;
    }
  }
  return max_val;  
}

template <class MatrixType>
int computeFij_getters(int* gamma,
		       MatrixType* M,
		       int i, int j) {
  int gamma_val = gamma[j];
  int max_val = M->get(i, 0) + gamma_val;

  for (int k_j = 1; k_j <= j-1; k_j++) {
    int test_val = M->get(i, k_j) + gamma[j - k_j];
    if (test_val > max_val) {
      max_val = test_val;
    }
  }  
  return max_val;    
}



/**************************************************************/
// The computation of Eij and Fij which uses iterators.

template <class MatrixType>
int computeEij_iterator(int* gamma,
			MatrixType* M,
			int i, int j) {

    // The version which uses a row and column iterator
    int gamma_val = gamma[i];   
    ArrayLargeDim k_i = M->row_iterator(0);
    ArrayLargeDim col_idx = M->col_iterator(j);

    int max_val = M->idx_get(&k_i,
			     &col_idx) + gamma_val;

    for (int k_i_count = 1; k_i_count <= i-1; ++k_i_count) {
      M->increment_row(&k_i);
      
      int test_val = M->idx_get(&k_i,
				&col_idx) + gamma[i-k_i_count];
      max_val = MAX(max_val, test_val);
    }
    return max_val;    
}

template <class MatrixType>
int computeFij_iterator(int* gamma,
			MatrixType* M,
			int i, int j) {
  // The version which uses a row and column iterator
  int gamma_val = gamma[j];   

  ArrayLargeDim row_idx = M->row_iterator(i);
  ArrayLargeDim k_j = M->col_iterator(0);

  int max_val = M->idx_get(&row_idx,
			   &k_j) + gamma_val;
    
  for (int k_j_count = 1; k_j_count <= j-1; ++k_j_count) {
    M->increment_col(&k_j);
    int test_val = M->idx_get(&row_idx, &k_j) + gamma[j-k_j_count];
    max_val = MAX(max_val, test_val);
  }
  return max_val;    
}




/*************************************************************/
// The code which uses reducers. 
template <class MatrixType>
int computeEij_reducer(int* gamma,
		       MatrixType* M,
		       int i, int j) {

  cilk::hyperobject<cilk::reducer_max<int> > max_val(INT_MIN);

  cilk_for(int k_i = 0; k_i <= i - 1; ++k_i) {
    int test_val = M->get(k_i, j) + gamma[i-k_i];
    max_val() = cilk::max_of(max_val(), test_val);
  }
  return max_val().get_value();
}








/*************************************************************/
// The code which attempts to do a divide and conquer.


template <class MatrixType, int base_size>
int computeEij_parallel_divide_and_conquer(int* gamma,
					   MatrixType *M,
					   int i, int j,
					   int start_i, int end_i) {
  if ((end_i - start_i) < base_size) {
    // The version which uses a row and column iterator
    int gamma_val = gamma[i - start_i];

    ArrayLargeDim k_i = M->row_iterator(start_i);
    ArrayLargeDim col_idx = M->col_iterator(j);

    //    printf("Initial: k_i = %lu, col_idx = %lu\n",
    //	   k_i, col_idx);
    int max_val = M->idx_get(&k_i,
			     &col_idx) + gamma_val;

    for (int k_i_count = start_i+1; k_i_count < end_i; ++k_i_count) {
      M->increment_row(&k_i);

      int test_val = M->idx_get(&k_i,
				&col_idx) + gamma[i-k_i_count];
      max_val = MAX(max_val, test_val);
    }
    return max_val;    
  }
  else {
    int val1, val2;
    int mid_i = (start_i + end_i) / 2;
    
    val1 = cilk_spawn computeEijParallel(gamma,
					 M, i, j,
					 start_i, mid_i);

    val2 = cilk_spawn computeEijParallel(gamma,
					 M, i, j,
					 mid_i, end_i);
    cilk_sync;    
    return MAX(val1, val2);
  }
}


template <class MatrixType, int base_size>
int computeFij_parallel_divide_and_conquer(int* gamma,
					   MatrixType *M,
					   int i, int j,
					   int start_j, int end_j) {  
  if ((end_j - start_j) < base_size) {

    // The version which uses a row and column iterator
    int gamma_val = gamma[j - start_j];

    ArrayLargeDim row_idx = M->row_iterator(i);
    ArrayLargeDim k_j = M->col_iterator(start_j);

    //    printf("Initial: k_i = %lu, col_idx = %lu\n",
    //	   k_i, col_idx);
    int max_val = M->idx_get(&row_idx, &k_j) + gamma_val;
    
    for (int k_j_count = start_j+1; k_j_count < end_j; ++k_j_count) {
      M->increment_col(&k_j);
      int test_val = M->idx_get(&row_idx, &k_j) + gamma[j-k_j_count];
      max_val = MAX(max_val, test_val);
    }
    return max_val;    
  }
  else {
    int val1, val2;
    int mid_j = (start_j + end_j) / 2;
    
    val1 = cilk_spawn computeFijParallel(gamma,
					 M, i, j,
					 start_j, mid_j);

    val2 = cilk_spawn computeFijParallel(gamma,
					 M, i, j,
					 mid_j, end_j);
    cilk_sync;

    return MAX(val1, val2);
  }
}



/************************************************************/
// Dummy versions of the code.

template <class MatrixType>
int computeE_dummy(int* gamma,
		   MatrixType* M,
		   int i, int j) {

    // The version which uses a row and column iterator
    int gamma_val = gamma[i];   
    ArrayLargeDim k_i = M->row_iterator(0);
    ArrayLargeDim col_idx = M->col_iterator(j);

    //    printf("Initial: k_i = %lu, col_idx = %lu\n",
    //	   k_i, col_idx);
    int max_val = M->idx_get(&k_i,
			     &col_idx) + gamma_val;
    
    for (int k_i_count = 1; k_i_count <= i-1; ++k_i_count) {
      //  M->increment_row(&k_i);

    // Doesn't access right value of gamma.
      int test_val = M->idx_get(&k_i,
				&col_idx) + gamma[i];
      max_val = MAX(max_val, test_val);
    }
    return max_val;    
}
		       

template <class MatrixType>
int computeF_dummy(int* gamma,
		   MatrixType* M,
		   int i, int j) {
  // The version which uses a row and column iterator
  int gamma_val = gamma[i];   

  ArrayLargeDim row_idx = M->row_iterator(i);
  ArrayLargeDim k_j = M->col_iterator(0);

  //    printf("Initial: k_i = %lu, col_idx = %lu\n",
  //	   k_i, col_idx);
  int max_val = M->idx_get(&row_idx,
			   &k_j) + gamma_val;
    
  for (int k_j_count = 1; k_j_count <= j-1; ++k_j_count) {
    //    M->increment_col(&k_j);

    // Doesn't access right value of gamma.
    int test_val = M->idx_get(&row_idx, &k_j) + gamma[j];
    max_val = MAX(max_val, test_val);
  }
  //    printf("Done with computeEij(%d, %d)\n", i, j);
  return max_val;    
}


template <class MatrixType>
int computeE_constant(int* gamma,
		      MatrixType* M,
		      int i, int j) {

  // The version which uses a row and column iterator
  int ans = gamma[i];
  if (i > 0) {
    ArrayLargeDim row_idx = M->row_iterator(i-1);
    ArrayLargeDim col_idx = M->col_iterator(j);
    ans += M->idx_get(&row_idx,
		      &col_idx);
  }
  return ans;
}

template <class MatrixType>
int computeF_constant(int* gamma,
		      MatrixType* M,
		      int i, int j) {
  // The version which uses a row and column iterator
  int ans = gamma[j];
  if (j > 0) {
    ArrayLargeDim row_idx = M->row_iterator(i);
    ArrayLargeDim col_idx = M->col_iterator(j-1);
    ans += M->idx_get(&row_idx,
		      &col_idx);
  }
  return ans;
}


template <class MatrixType>
int computeE_no_scan(int* gamma,
		     MatrixType* M,
		     int i, int j) {
  int ans = 0;
  ArrayLargeDim col_idx = M->col_iterator(j);

  for (int k_i_count = 1; k_i_count <= i-1; ++k_i_count) {
    ans = gamma[i] + k_i_count;
    if (i > 0) {
      ArrayLargeDim row_idx = M->row_iterator(i-1);
      ans += M->idx_get(&row_idx,
			&col_idx);
    }
  }
  return ans;
}

template <class MatrixType>
int computeF_no_scan(int* gamma,
		     MatrixType* M,
		     int i, int j) {
  int ans = 0;
  ArrayLargeDim row_idx = M->row_iterator(i);
  for (int k_j_count = 1; k_j_count <= j-1; ++k_j_count) {
    ans = gamma[j] + k_j_count;
    if (j > 0) {
      ArrayLargeDim col_idx = M->col_iterator(j-1);
      ans += M->idx_get(&row_idx,
			&col_idx);
    }
  }

  return ans;
}

		       
#endif
