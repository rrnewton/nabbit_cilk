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

// Sample program that illustrates how to use Static Nabbit.
//
// TODO(jsukha): Add an example that illustrates how to use dynamic
// Nabbit.

#include <cassert>
#include <cstdlib>
#include <iostream>


#include <cilk/cilk_api.h>
#include <cilk/cilk.h>
#include "sample_dag_node.h"

typedef enum {
  TEST_SERIAL = 0,
  TEST_STATIC_NABBIT = 1,
  TEST_ALL,
} SampleTestType;


// Constructs a DAG with sink node 0, and
// source node SAMPLE_DAG_SIZE-1.
//
// The value of each node is its key + the values of its
// immediate predecessors.
template <class NodeType>
void create_static_DAG(SampleDAGNode<NodeType>* nodes, int n) {
  assert(n <= SAMPLE_DAG_SIZE);
  for (int i = 0; i < n; i++) {
    nodes[i].key = i;
    nodes[i].params = NULL;
    nodes[i].init_node();    
  }

  nodes[0].add_dep(&nodes[1]);
  nodes[0].add_dep(&nodes[2]);

  nodes[1].add_dep(&nodes[3]);
  nodes[1].add_dep(&nodes[4]);
  nodes[1].add_dep(&nodes[5]);

  nodes[2].add_dep(&nodes[3]);
  nodes[2].add_dep(&nodes[5]);

  nodes[3].add_dep(&nodes[6]);
  nodes[4].add_dep(&nodes[6]);
  nodes[5].add_dep(&nodes[7]);

  nodes[6].add_dep(&nodes[SAMPLE_DAG_SIZE-1]);
  nodes[7].add_dep(&nodes[SAMPLE_DAG_SIZE-1]);
}


void run_test(SampleTestType test_type) {  
  switch (test_type) {
  case TEST_SERIAL:
    {
      SampleDAGNode<StaticSerialNode> nodes[SAMPLE_DAG_SIZE];
      create_static_DAG(nodes, SAMPLE_DAG_SIZE);
      nodes[SAMPLE_DAG_SIZE-1].source_compute();
      assert(nodes[0].result == 55);
    }
    break;
  case TEST_STATIC_NABBIT:
    {
      SampleDAGNode<StaticNabbitNode> nodes[SAMPLE_DAG_SIZE];
      create_static_DAG(nodes, SAMPLE_DAG_SIZE);
      nodes[SAMPLE_DAG_SIZE-1].source_compute();
      assert(nodes[0].result == 55);
    }
    break;    
  default:
    printf("No test type %d\n", test_type);
    assert(0);
  }  
}
				
int main(int argc, char *argv[])
{

  SampleTestType test_type = TEST_SERIAL;
  //  int P = cilk::current_worker_count();
  int P = __cilkrts_get_total_workers();
  
  if (argc >= 2) {
    test_type = (SampleTestType)atoi(argv[1]);
    printf("Test_type = %d, P = %d\n",
	   test_type, P);
    run_test(test_type);
  }
  else {
    int q;
    for (q = (int)TEST_SERIAL;
	 q < (int)TEST_ALL;
	 q++) {

      printf("*************************\n");
      printf("Test_type = %d, P = %d\n",
	     q, P);
      run_test((SampleTestType)q);
      printf("\n\n");
    }
  }  
  return 0;
}


