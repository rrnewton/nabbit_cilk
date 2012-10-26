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

#ifndef __STATIC_SERIAL_NODE_H_
#define __STATIC_SERIAL_NODE_H_


#include <dag_status.h>
#include <dynamic_array.h>

// Debugging flag.
//#define NABBIT_PRINT_DEBUG 1

class StaticSerialNode;
typedef DynamicArray<StaticSerialNode*> StaticSerialNodeArray;


class StaticSerialNode {

 public:
  long long key;
  StaticSerialNodeArray* predecessors;
  StaticSerialNodeArray* successors;
  //  StaticSerialNodeArray* children;

  // Constructors for a node.
  StaticSerialNode(long long k);
  StaticSerialNode(long long k, int num_predecessors);

  ~StaticSerialNode();

  // Methods to call when constructing a DAG statically.
  void init_node(int default_degree);
  void init_node();

  void add_dep(StaticSerialNode* child);
  
  void add_child(StaticSerialNode* child);
  void source_compute();
  
 protected:
  virtual void InitNode() = 0;
  virtual void Compute() = 0;

 private:
  volatile int join_counter; 
  void compute_and_notify();

};



// When constructing a StaticSerialNode, we need to initialize the blocking
// array because when a new node n gets put into the hash table, other
// nodes may block on n, and add themselves to this array, even though
// n hasn't been expanded yet.
StaticSerialNode::StaticSerialNode(long long k) 
  :  key(k),
     predecessors(NULL),
     successors(NULL) {
}

StaticSerialNode::StaticSerialNode(long long k, int num_predecessors) 
  :  key(k),
     predecessors(NULL),
     successors(NULL) {
}

     
StaticSerialNode::~StaticSerialNode() {
  if (this->predecessors) {
    delete this->predecessors;
  }
  if (this->successors != NULL) {
    delete this->successors;
  }
}


/***************************************************************/
// Methods for constructing the dag statically. 

void StaticSerialNode::init_node(int default_degree) {

  this->predecessors = new StaticSerialNodeArray(default_degree);
  this->successors = new StaticSerialNodeArray(default_degree);
  this->join_counter = 0;
  //  this->children = this->predecessors;

  // Call user-defined initialization.
  this->InitNode();
}

void StaticSerialNode::init_node() {
  init_node(5);
}



// Both "this" node and dep_node should have been initialized already.
void StaticSerialNode::add_dep(StaticSerialNode* dep_node) {

  // Add an edge from dep_node -> this.
  this->predecessors->add(dep_node);
  dep_node->successors->add(this);
  this->join_counter++;
}

void StaticSerialNode::add_child(StaticSerialNode* dep_node) {
  // Add an edge from dep_node -> this.
  this->predecessors->add(dep_node);
  dep_node->successors->add(this);
  this->join_counter++;
}


void StaticSerialNode::source_compute(void) {
  this->compute_and_notify();
}


/***************************************************************/
// Methods which call Compute() and do bookkeepping.

void StaticSerialNode::compute_and_notify() {

#if NABBIT_PRINT_DEBUG == 1
  printf("COMPUTE AND NOTIFY called on key %llu, worker %d\n",
  	 this->key,
	 cilk::current_worker_id());
#endif
  this->Compute();
  
  int end_to_notify = this->successors->size_estimate();

  // Handle the current range of values in the blocking array.
  //    cilk_for (int i = this->notify_counter; i < end_to_notify; i++) {
  for (int i = 0; i < end_to_notify; i++) {

    StaticSerialNode* current_succ = this->successors->get(i);
    if (current_succ->join_counter <= 0) {
      printf("ERROR: this key = %llu, current_succ = %p (key = %llu), join coutner = %d\n",
	     this->key, 
	     current_succ, current_succ->key,
	     current_succ->join_counter);
    }
    assert(current_succ->join_counter > 0);

    current_succ->join_counter--;
    int updated_val = current_succ->join_counter;

    if (updated_val == 0) {

#if NABBIT_PRINT_DEBUG == 1
      printf("Worker %d enabling current_pred with key = %llu.\n",
	     cilk::current_worker_id(),
	     current_succ->key);
#endif
      current_succ->compute_and_notify();
    }
  }
}



#endif
