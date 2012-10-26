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

#ifndef _SW_VISUAL_H_
#define _SW_VISUAL_H_


#include <nabbit_timers.h>
#include <nabbit_logging.h>

#include "image.h"
#include "sw_test_types.h"


// For the Smith-Waterman dynamic program,
// we want to log when blocks of the grid are computed.

struct SWRec {
  int start_i;  //  Records which block we
  int end_i;    //  are computing.
  int start_j;
  int end_j;    
};


#ifdef TRACK_THREAD_CPU_IDS

extern NabbitTaskGraphStats<SWRec>* sw_global_stats;

void update_image_for_start_compute(color_image* cimage,
				    NabbitNodeRecord<SWRec>* nrec,
				    int proc_id,
				    int image_count,
				    double current_time,
				    int sample_factor);

void update_image_for_finish_compute(color_image* cimage,
				     NabbitNodeRecord<SWRec>* nrec,
				     int proc_id,
				     int image_count,
				     double current_time,
				     int sample_factor);

// Scans through the log and generates a sequence of images
// 
void process_sw_log(int n, int B, int test_type, int P, bool verbose);

#endif

#endif
