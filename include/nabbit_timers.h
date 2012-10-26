// Code for the Nabbit task graph library
//
// Some simple timers for Nabbit.
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

#ifndef _NABBIT_TIMERS_H_
#define _NABBIT_TIMERS_H_


#include <sys/time.h>


// Output from a processor's cycle counter.
typedef unsigned long long rTimeStruct;

class NabbitTimers {

 public:


  // Check the processor cycle counter.
  
  static inline void cycleCounter(rTimeStruct* tv) {
    //  The 64-bit version
    unsigned int low,high;
    __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
    *tv = (((unsigned long long)high)<<32)+low;
  }

  // Convert output from the the cycle counter to
  // seconds.
  static double rtimeToSec(rTimeStruct rtime,
			   rTimeStruct base_rtime,
			   double base_walltime,
			   double cycles_per_sec) {
    long long rtime_diff = rtime - base_rtime;
    return base_walltime + rtime_diff / cycles_per_sec;
  }  

  // Convert gettimeofday structure to seconds.
  static double tvToSec(struct timeval tv) {
    return tv.tv_sec + (1.0e-6 * tv.tv_usec);
  }
};

#endif
