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

#ifndef __DYNAMIC_SERIAL_NODE_H_
#define __DYNAMIC_SERIAL_NODE_H_

#include <dag_status.h>
#include <dynamic_array.h>
#include <task_graph_hash_table.h>

// Debugging flag.

#define PRINT_STATE_CHANGES 0

#define NABBIT_PRINT_DEBUG 0

class DynamicSerialNode;

typedef DynamicArray<long long> DTGSKeyArray;
typedef DynamicArray<DynamicSerialNode*> DynamicSerialNodeArray;

class DynamicSerialNode {

 public:
  long long key; 
  TaskGraphHashTable* H;
  DTGSKeyArray* predecessors;
  
  // Constructors for a node.
  DynamicSerialNode(long long k, TaskGraphHashTable* H);
  DynamicSerialNode(long long k, TaskGraphHashTable* H, int num_succ);
  ~DynamicSerialNode();

  
  void add_dep(long long key);
  void generate_task(long long key);
  
  bool init_root_and_compute(long long root_key);

  DAGNodeStatus get_status();
  inline bool try_mark_as_visited();
  
 protected:
    
  virtual void Init() = 0;
  virtual void Compute() = 0;
  virtual void Generate() = 0;



 private:
  DAGNodeStatus volatile status;
  volatile int join_counter;

  DynamicSerialNodeArray* succ_to_notify;
  DTGSKeyArray* generated_tasks;

  volatile int notify_counter; 
  volatile int blocking_lock;

  inline void mark_as_visited();

  inline void mark_as_expanded();

  inline void mark_as_computed();
  
  inline void mark_as_completed();

  //  inline bool try_acquire_blocking_lock();
  //  inline void acquire_blocking_lock();
  //  inline void release_blocking_lock();


  void try_init_pred_and_compute(long long pred_key); 
  void init_node_and_compute();
  void compute_and_notify();

};



// When constructing a DynamicSerialNode, we need to initialize the blocking
// array because when a new node n gets put into the hash table, other
// nodes may block on n, and add themselves to this array, even though
// n hasn't been expanded yet.
DynamicSerialNode::DynamicSerialNode(long long k,
				     TaskGraphHashTable* H_)
  :  key(k),
     H(H_),
     predecessors(NULL),
     status(NODE_UNVISITED),
     join_counter(1),
     succ_to_notify(new DynamicSerialNodeArray(4)),
     generated_tasks(NULL),     
     blocking_lock(0) {
}

// The same as the previous construct, except we pass in a default
// size for the blocking array.

DynamicSerialNode::DynamicSerialNode(long long k,
				     TaskGraphHashTable* H_,
				     int num_succ) 
  :  key(k),
     H(H_),
     predecessors(NULL),
     status(NODE_UNVISITED),
     join_counter(1),
     succ_to_notify(new DynamicSerialNodeArray(num_succ)),
     generated_tasks(NULL),
     blocking_lock(0) {
}


DynamicSerialNode::~DynamicSerialNode() {
  if (this->predecessors) {
    delete this->predecessors;
  }
  if (this->succ_to_notify) {
    delete this->succ_to_notify;
  }
  if (this->generated_tasks) {
    delete this->generated_tasks;
  }
}


/* bool DynamicSerialNode::try_acquire_blocking_lock() { */
/*   bool acquired = false; */
/*   acquired = __sync_bool_compare_and_swap(&this->blocking_lock, */
/* 					  0, */
/* 					  1); */
/*   return acquired; */
/* } */

/* void DynamicSerialNode::acquire_blocking_lock() { */
/*   bool acquired = false; */
/*   while (!acquired) { */
/*     acquired = __sync_bool_compare_and_swap(&this->blocking_lock, */
/* 					    0, */
/* 					    1); */
/*   }       */
/* } */

/* void DynamicSerialNode::release_blocking_lock() { */
/*   assert(this->blocking_lock > 0); */
/*   __sync_lock_release(&this->blocking_lock); */
/* } */

bool DynamicSerialNode::try_mark_as_visited() {

  bool valid = __sync_bool_compare_and_swap(&this->status,
					    NODE_UNVISITED,
					    NODE_VISITED);
  return valid;
}


void DynamicSerialNode::mark_as_visited() {
  bool valid = __sync_bool_compare_and_swap(&this->status,
					    NODE_UNVISITED,
					    NODE_VISITED);
  assert(valid);
  if (PRINT_STATE_CHANGES) {
    printf("--- Key %llu: marking as VISITED. join_counter = %d\n",
	   this->key,
	   this->join_counter);
  }
}

void DynamicSerialNode::mark_as_expanded() {
  bool valid = __sync_bool_compare_and_swap(&this->status,
					    NODE_VISITED,
					    NODE_EXPANDED);
  if (!valid) {
    printf("Mark as expanded: Worker %d, key = %llu, status = %d\n",
	   GET_WORKER_ID, // cilk::current_worker_id(),
	   this->key,
	   this->status);
  }
  assert(valid);

  if (PRINT_STATE_CHANGES) {
    printf("--- Key %llu: marking as EXPANDED. join_counter = %d\n",
	   this->key,
	   this->join_counter);
  }
}



void DynamicSerialNode::mark_as_computed() {
  bool valid = __sync_bool_compare_and_swap(&this->status,
					    NODE_EXPANDED,
					    NODE_COMPUTED);
  assert(valid);
  if (PRINT_STATE_CHANGES) {
    printf("--- Key %llu: marking as COMPUTED. join_counter = %d\n",
	   this->key,
	   this->join_counter);
  }
}

/* // To switch from computed to completed, we need to be holding the */
/* // lock on the blocking array. */
/* bool DynamicSerialNode::try_mark_as_completed() { */
/*   bool val = false; */
/*   acquire_blocking_lock(); */
/*   { */
/*     if (this->notify_counter == this->succ_to_notify->size_estimate()) { */
/*       bool valid = __sync_bool_compare_and_swap(&this->status, */
/* 						NODE_COMPUTED, */
/* 						NODE_COMPLETED); */
/*       assert(valid); */
/*       val = true; */
/*     } */
/*   } */
/*   release_blocking_lock(); */

/*   if (PRINT_STATE_CHANGES) { */
/*     if (val) { */
/*       printf("--- Key %llu: marked as COMPLETED. join_counter = %d\n", */
/* 	     this->key, */
/* 	     this->join_counter); */
/*     } */
/*   } */
/*   return val; */
/* } */

void DynamicSerialNode::mark_as_completed() {
  assert(this->status == NODE_COMPUTED);
  this->status = NODE_COMPLETED;
}


DAGNodeStatus DynamicSerialNode::get_status() {
  return this->status;
}

void DynamicSerialNode::add_dep(long long key) {
  this->predecessors->add(key);

  this->join_counter++;
  //__sync_add_and_fetch(&this->join_counter,
  //		       1);  
}

void DynamicSerialNode::generate_task(long long key) {
  this->generated_tasks->add(key);
}


/***************************************************************/
// Methods for constructing the dag statically.

void DynamicSerialNode::try_init_pred_and_compute(long long pred_key) {

  bool inserted = false;
  DynamicSerialNode* actualPredNode;

#if NABBIT_PRINT_DEBUG == 1
  printf("inside try_init_pred_and_compute: pred_key = %llu, this->key = %llu\n",
	 pred_key, this->key);
#endif
  
  actualPredNode = (DynamicSerialNode*)H->get_task(pred_key);

  // Keep trying to insert the node until we get something.
  while (!actualPredNode) {
    inserted = H->insert_task_if_absent(pred_key);
    actualPredNode = (DynamicSerialNode*)H->get_task(pred_key);
  }

  if (inserted) {
    //    actualPredNode->mark_as_visited();
    actualPredNode->init_node_and_compute();
  }
  
  // Continue, whether or not 
  {
    bool pred_finished = true;

    //    actualPredNode->acquire_blocking_lock();
    DAGNodeStatus other_status = actualPredNode->status;

    if (other_status < NODE_COMPUTED) {
      actualPredNode->succ_to_notify->add(this);
      pred_finished = false;
    }

#if NABBIT_PRINT_DEBUG == 1
    printf("inserted = %d. finished? %d\n", inserted, pred_finished);
#endif
	   
    //    actualPredNode->release_blocking_lock();

    if (pred_finished) {
      //      int val = __sync_add_and_fetch(&this->join_counter,
      //				     -1);
      this->join_counter--;
      int val = this->join_counter;      
      if (val == 0) {
	//	cilk_spawn this->compute_and_notify();
	this->compute_and_notify();      
      }
    }
  }
}



void DynamicSerialNode::init_node_and_compute() {

  int default_children_count = 4;
  int i;
  this->predecessors = new DTGSKeyArray(default_children_count);
  Init();

  this->mark_as_expanded();

  // First try to init + compute predecessors.
  for (i = 0; i < this->predecessors->size_estimate(); ++i) {
    long long pred_key = this->predecessors->get(i);
    //    cilk_spawn try_init_pred_and_compute(pred_key);
    try_init_pred_and_compute(pred_key);
  }

  {
    int val;
    this->join_counter--;
    val = this->join_counter;
    //    val = __sync_add_and_fetch(&this->join_counter, -1);
    
    if (val == 0) {
      compute_and_notify();
    }
  }
}



/***************************************************************/
// Methods which call Compute() and do bookkeepping.

void DynamicSerialNode::compute_and_notify() {

#if NABBIT_PRINT_DEBUG == 1
  printf("COMPUTE AND NOTIFY called on key %llu, worker %d\n",
  	 this->key,
	 GET_WORKER_ID, //cilk::current_worker_id()
        );
#endif
  this->Compute();
  this->mark_as_computed();

  this->generated_tasks = new DTGSKeyArray(4);
  this->Generate();

  for (int i = 0; i < this->generated_tasks->size_estimate(); ++i) {
    long long gen_key = this->generated_tasks->get(i);
    init_root_and_compute(gen_key);
  }

  this->notify_counter = 0;
  int end_to_notify = 0;


  // Keep notifying the children in the blocking array until we
  // have reached the end of the array.
  //
  // Note that the blocking array might grow, as more nodes are
  // being expanded.  On the other hand, the size of the
  // blocking array always monotonically increases.  
  bool done = false;
  while (!done) {

    end_to_notify = this->succ_to_notify->size_estimate();
    
    // Handle the current range of values in the blocking array.
    //    cilk_for (int i = this->notify_counter; i < end_to_notify; i++) {
    for (int i = this->notify_counter; i < end_to_notify; i++) {
      
      DynamicSerialNode* current_succ = this->succ_to_notify->get(i);
      
      assert(current_succ->join_counter > 0);

      assert((current_succ->status == NODE_VISITED) ||
	     (current_succ->status == NODE_EXPANDED));

      current_succ->join_counter--;
      int updated_val = current_succ->join_counter;

      //      int updated_val = __sync_add_and_fetch(&current_succ->join_counter,
      //					     -1);
      
      if (updated_val == 0) {
	assert((current_succ->status == NODE_EXPANDED));

	// The parent node has been EXPANDED.  Now we should
	// push the parent node onto our deque.

#if NABBIT_PRINT_DEBUG == 1
	printf("Worker %d enabling current_succ with key = %llu.  Node's status is %d\n",
	       GET_WORKER_ID, // cilk::current_worker_id(),
	       current_succ->key,
	       current_succ->status);
#endif
	//	cilk_spawn current_succ->compute_and_notify();
	current_succ->compute_and_notify();
      }
    }

    this->notify_counter = end_to_notify;
    
    // Check to see if we have notified everyone in the
    // blocking array.

    //    done = this->try_mark_as_completed();
    this->mark_as_completed();
    done = true;
  }

  assert(this->status == NODE_COMPLETED);
}


bool DynamicSerialNode::init_root_and_compute(long long root_key) {

  bool inserted = false;
  DynamicSerialNode* actualNode = (DynamicSerialNode*)H->get_task(root_key);
  
  // Keep trying to insert the node until we get something.
  while (!actualNode) {
    inserted = H->insert_task_if_absent(root_key);
    actualNode = (DynamicSerialNode*)H->get_task(root_key);
  }

  if (inserted) {
    //    actualNode->mark_as_visited();

    //    printf("Actually inserted key %llu as a root\n", root_key);
    //    cilk_spawn actualNode->init_node_and_compute();

    actualNode->init_node_and_compute();
  }  
  return inserted;
}








#endif
