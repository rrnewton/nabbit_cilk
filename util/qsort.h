/*
 * qsort.cilk
 *
 * An implementation of quicksort using Cilk parallelization.
 *
 * Copyright (c) 2007-2008 Cilk Arts, Inc.  55 Cambridge Street,
 * Burlington, MA 01803.  Patents pending.  All rights reserved. You may
 * freely use the sample code to guide development of your own works,
 * provided that you reproduce this notice in any works you make that
 * use the sample code.  This sample code is provided "AS IS" without
 * warranty of any kind, either express or implied, including but not
 * limited to any implied warranty of non-infringement, merchantability
 * or fitness for a particular purpose.  In no event shall Cilk Arts,
 * Inc. be liable for any direct, indirect, special, or consequential
 * damages, or any other damages whatsoever, for any use of or reliance
 * on this sample code, including, without limitation, any lost
 * opportunity, lost profits, business interruption, loss of programs or
 * data, even if expressly advised of or otherwise aware of the
 * possibility of such damages, whether in an action of contract,
 * negligence, tort, or otherwise.
 *
 */

// This quicksort code was copied from the Cilk++ examples for use in
// Nabbit.
// Copyright (c) 2010 Jim Sukha

#ifndef __QSORT_H_
#define __QSORT_H_

#include <iostream>
#include <cstdlib>
#include <cilk.h>


// Sort the range between bidirectional iterators begin and end.
// end is one past the final element in the range.
// Use the Quick Sort algorithm, using recursive divide and conquer.
// This function is NOT the same as the Standard C Library qsort() function.
// This implementation is pure C++ code before Cilk++ conversion.
void sample_qsort(long long * begin, long long * end)
{
  
  if (begin != end) {
       
    // Pick a random element as pivot and swap it to the end!
    long long section_size = ((long long)end - (long long)begin) / sizeof(long long);
    int rand_element = rand() % section_size;
    long long* rand_slot = (begin + rand_element);
      
    //      printf("section_size is %d, rand_element = %d\n",
    //     section_size, rand_element);

    --end;  // Exclude last element (pivot) from partition
    using std::swap;
    swap(*end, *rand_slot);
    long long * middle = std::partition(begin, end,
					std::bind2nd(std::less<long long>(), *end));
    using std::swap;
    swap(*end, *middle);    // move pivot to middle
    cilk_spawn sample_qsort(begin, middle);
    sample_qsort(++middle, ++end); // Exclude pivot and restore end
    cilk_sync;
  }
}


// Confirm that a is sorted and that each element contains the index.
void check_sort(long long* a, long long n) {
  for (long long i = 0; i < n - 1; ++i) {
    if (a[i] >= a[i + 1] || a[i] != i) {
      std::cout << "Sort failed at location i=" << i << " a[i] = "
		<< a[i] << " a[i+1] = " << a[i + 1] << std::endl;
    }
  }
  std::cout << "Sort succeeded." << std::endl;
}

// Checks that a[i] < a[i+1] for all i in [0, n-1)
void check_strictly_increasing(long long* a, long long n) {
  int num_errors = 0;
  for (long long i = 0; i < n-1; ++i) {
    if (a[i] >= a[i+1]) {
      std::cout << "Not strictly increasing at location i=" << i << " a[i] = "
		<< a[i] << " a[i+1] = " << a[i+1] << std::endl;
      num_errors++;
    }    
  }
  std::cout << "Errors in sort: " << num_errors << std::endl;
}



#endif
