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

#ifndef __SAMPLE_DAG_NODE_H
#define __SAMPLE_DAG_NODE_H

#include <dag_node.h>

const int SAMPLE_DAG_SIZE = 10;

template <class NodeType>
class SampleDAGNode: public NodeType {

 private:
  void InitNode();
  void Compute();
  
 public:
  SampleDAGNode();
  void* params;
  int result;
  

};


template <class NodeType>
SampleDAGNode<NodeType>::SampleDAGNode() 
  : NodeType(0, 3),
    params(NULL) {    
}


template <class NodeType>
void SampleDAGNode<NodeType>::InitNode() {
  if (this->key < SAMPLE_DAG_SIZE-1) {
    this->result = (int)this->key;
  }
  else {
    // Source node has no value associated with it.
    this->result = 0;
  }
  printf("InitNode with key %llu: initialized result to %d\n",
	 this->key,
	 this->result);
}

template <class NodeType>
void SampleDAGNode<NodeType>::Compute() {

  for (int i = 0; i < this->predecessors->size_estimate(); i++) {
    SampleDAGNode<NodeType>* child = (SampleDAGNode<NodeType>*)this->predecessors->get(i);
    this->result += child->result;
  }

  printf("At key %llu: computed value %d\n",
	 this->key,
	 this->result);
}


#endif
