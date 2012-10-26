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

// This code defines the Nabbit static task graph node for the dynamic
// program.


#ifndef __SW_DAG_NODE_H
#define __SW_DAG_NODE_H

#include <dag_node.h>
#include <array2d_base.h>
#include <array2d_morton.h>

#include "sw_matrix_kernels.h"
#include "SWDagParams.h"

// #define DEBUG_PRINT

template <class NodeType>
class SWDAGNode: public NodeType {

 private:
  friend class SWDAGParams<SWDAGNode<NodeType> >;    
  SWDAGParams<SWDAGNode<NodeType> >* params;
  int result;

  SWDAGNode(long long k,
	    SWDAGParams<SWDAGNode<NodeType> >* params);
  
  void InitNode();
  void Compute();

#ifdef TRACK_THREAD_CPU_IDS  
  // int init_id;
  // No visit image unless we are using dynamic Nabbit. 
  int compute_id;
#endif

  
 public:
  SWDAGNode();

  int GetResult();

  void CheckResult();
  
};


template <class NodeType>
SWDAGNode<NodeType>::SWDAGNode()
  : NodeType(0, 3)
{
  
}


template <class NodeType>
SWDAGNode<NodeType>::SWDAGNode(long long k,
			       SWDAGParams<SWDAGNode<NodeType> >* params)
  : NodeType(k),
    params(params) {
}
		    
template <class NodeType>
void SWDAGNode<NodeType>::InitNode() {
  this->result = 0;
}

template <class NodeType>
void SWDAGNode<NodeType>::Compute() {
  assert(this->get_status() == NODE_EXPANDED);  
  this->result = params->ComputeAtKey(this->key);
}

template <>
void SWDAGNode<StaticSerialNode>::Compute() {
  this->result = params->ComputeAtKey(this->key);
}

template <>
void SWDAGNode<StaticNabbitNode>::Compute() {
  this->result = params->ComputeAtKey(this->key);

#ifdef TRACK_THREAD_CPU_IDS
  this->compute_id = cilk::current_worker_id();
#endif  
}


template <class NodeType>
int SWDAGNode<NodeType>::GetResult() {
  return this->result;
}





#endif
