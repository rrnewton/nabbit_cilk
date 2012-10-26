#!/bin/sh
#  
#  
# Sample script for running Random DAG microbenchmark. 
# 
# Copyright (c) 2010 Jim Sukha
# 
# 
# 
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use, copy,
# modify, merge, publish, distribute, sublicense, and/or sell copies
# of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# 
#
# Runs test cases for strongly dynamic graphs.

maxP=8
maxTestType=5

for dag_type in 0
  do
  echo "********* DAG_TYPE=$dag_type *****************"

  for R in 1000 100 10000   # 10 100 1000
    do
    echo "-----------------------R=$R-----------------------------"
    for NodeWork in 100 1000 10000 1000000
      do
      for UseParallelNodes in 0 1
	do
        echo "***********NodeWork = $NodeWork, UseParallelNodes=$UseParallelNodes *****************"      
        for test_type in 4 5
        do 
 	  for ((P=1; P<=$maxP; P+=1)) do
	    rep_count=5
  #	  if [ $test_type -gt 1 ]
  #          then
  #	    rep_count=5
  #          fi
            for ((rep=0; rep<rep_count; rep+=1)) do
  	      "./detpath_test" $R $test_type $NodeWork $UseParallelNodes $dag_type -cilk_set_worker_count=$P
            done
	  done
        done
      done
    done
  done
done
