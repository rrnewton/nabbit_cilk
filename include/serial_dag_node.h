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

#ifndef __SERIAL_DAG_NODE_H_
#define __SERIAL_DAG_NODE_H_


#include <dag_status.h>
#include <dynamic_array.h>

class SerialDAGNode;


typedef DynamicArray<SerialDAGNode*> DAGNodeArray;


class SerialDAGNode {

 public:
  long long key;
  DAGNodeArray* children;

  // Constructors
  SerialDAGNode(long long k);
  SerialDAGNode(long long k,
		int num_predecessors);
  ~SerialDAGNode();

  // Methods to call when constructing the DAG statically.
  void init_node(int default_children_count);
  void init_node();
  void add_child(SerialDAGNode* child);

  void root_visit();  

 protected:
  DAGNodeStatus get_status();      

  virtual void InitNode() = 0;
  virtual void Compute() = 0;

 private:
  // Status field
  DAGNodeStatus volatile status;

  // Methods which change and get status.
  inline void mark_as_visited();
    
};




SerialDAGNode::SerialDAGNode(long long k) 
  :  key(k),
     children(NULL),
     status(NODE_UNVISITED) {
}

// The same as the previous construct, except we pass in a default
// size for the blocking array.
SerialDAGNode::SerialDAGNode(long long k,
			     int num_predecessors)
  :  key(k),
     children(NULL),
     status(NODE_UNVISITED) {
}
     
SerialDAGNode::~SerialDAGNode() {
  if (this->children) {
    delete this->children;
  }
}

void SerialDAGNode::mark_as_visited() {
  bool valid = (this->status == NODE_UNVISITED);
  this->status = NODE_VISITED;
  assert(valid);
}

DAGNodeStatus SerialDAGNode::get_status() {
  return this->status;
}


/***************************************************************/
// Methods for constructing the dag statically. 
void SerialDAGNode::init_node(int default_children_count) {
  this->children = new DAGNodeArray(default_children_count);
  // Call user-defined initialization.
  this->InitNode();
}

void SerialDAGNode::init_node() {
  init_node(5);
}

void SerialDAGNode::add_child(SerialDAGNode* child) {
  this->children->add(child);
}


/***************************************************************/
// Method for traversing the dag. 

void SerialDAGNode::root_visit(void) {
  assert(this->get_status() == NODE_UNVISITED);
  this->mark_as_visited();

  int num_children = this->children->size_estimate();
  for (int i = 0; i < num_children; ++i) {
    SerialDAGNode* child_node = (SerialDAGNode*)this->children->get(i);
    if (child_node->get_status() == NODE_UNVISITED) {
      child_node->root_visit();
      assert(child_node->get_status() == NODE_VISITED);
    }
  }
  this->Compute();
}


#endif
