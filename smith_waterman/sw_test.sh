#!/bin/bash
#
#  
# Sample script for running Smith-Waterman dynamic program.
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
# This simple bash script runs tests for various matrix sizes, block
# sizes, and test types.
#
#

# Maximum number of processors to test.
maxP=8  

# Which test types to run.
# (See SWComputeType enum in sw_compute.cilk for details).
maxTestType=5


B=16
for N in 1000 2000 5000 # 15000 
do
for ((P=$maxP; P>=1 ;P/=2)) do
echo "-----------------------N=$N-----------------------------"
    M=$N
    echo "***********N = $N, B=$B ********************"
    for test_type in 1 2 3 4 5 
    do
#    for ((test_type=1; test_type<=$maxTestType; test_type+=1)) do 
        "./swblock_$B" $N $M $test_type 0 -cilk_set_worker_count=$P
    done
done
done

