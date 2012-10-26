// Code for the Nabbit task graph library
// Random DAG microbenchmark. 
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


/****************************************************
 *
 * Counting paths through a random DAG, using dynamic Nabbit.
 *
 */  

#ifndef __DYN_COUNT_NODE_H_
#define __DYN_COUNT_NODE_H_


#include <dynamic_nabbit_node.h>
#include <dynamic_serial_node.h>
#include "det_count.h"

// #define PRINT_DEBUG_STATEMENTS 1


template <class DynNodeType>
class DynPathCountNode;


// The hash table for storing nodes.  Actually, we should have already
// generated all the necessary nodes already, but not initialized them
// yet.
template <class DynNodeType>
class DynPathHashTable: public TaskGraphHashTable {

 public:  
  ConcurrentHashTable* H;  

  DynPathHashTable(ConcurrentHashTable* H_);

  void* get_task(long long key);
  
  int insert_task_if_absent(long long key);
};


template <class DynNodeType>
class DynPathCountNode: public DynNodeType {
  
 protected:
  // Global parameters for this computation.
  CountPathDAGParams* params;
  DynamicArray<DynPathCountNode*>* children;
  
  int result;
  int path_length;

  DynPathCountNode(long long k,
		   CountPathDAGParams* params,
		   DynPathHashTable<DynNodeType>* H);
  
  void Init();
  void Compute();
  void Generate();

  
 public:

  DynPathCountNode(CountPathDAGParams *params, DynPathHashTable<DynNodeType>* H);
  static void GenerateTestDag(CountPathDAGParams* params);
  int GetResult();
  int GetPathLength();


  // Helper method for GenerateTestDag.
  DynPathCountNode* create_test_child(long long k);

  // Methods which visits all the children of the dag, and stores
  // the resulting nodes into node_check_map.
  friend void SerialVisitDag<DynPathCountNode>(DynPathCountNode* current_root,
					       ConcurrentHashTable* node_check_map);
  
  // Checks the result at each node, and sees whether it is the sum of
  // the values at all of its children (and checks whether the sink
  // node has value of 1).
  friend bool CheckResult<DynPathCountNode>(DynPathCountNode* root,
					    bool verbose);

};


template <class DynNodeType>
DynPathHashTable<DynNodeType>::DynPathHashTable(ConcurrentHashTable* H_):
  H(H_) {
}


template <class DynNodeType>
void* DynPathHashTable<DynNodeType>::get_task(long long key) {
  DynPathCountNode<DynNodeType>* n = NULL;
  LOpStatus code = OP_FAILED;
  while (code == OP_FAILED) {
    n = (DynPathCountNode<DynNodeType>*)H->search(key,
						  &code);
  }

  if (n != NULL) {
    if (n->get_status() >= NODE_VISITED) {
      return (void*)n;
    }
  }
  return NULL;
}

template <class DynNodeType>
int DynPathHashTable<DynNodeType>::insert_task_if_absent(long long key) {

  DynPathCountNode<DynNodeType>* n = NULL;
  LOpStatus code = OP_FAILED;
  bool success = false;
  while (code == OP_FAILED) {
    n = (DynPathCountNode<DynNodeType>*)H->search(key,
						  &code);
  }
  assert(n != NULL);
  if (n->get_status() >= NODE_VISITED) {

#if PRINT_DEBUG_STATEMENTS == 1
    printf("HASH insert if absent on key %llu. failed\n",
	   key);
#endif

    return 0;
  }
  
  while (n->get_status() == NODE_UNVISITED) {
    success = n->try_mark_as_visited();
  }

  if (success) {
#if PRINT_DEBUG_STATEMENTS == 1
    printf("HASH insert if absent on key %llu. success\n",
	   key);
#endif    
    return 1;
  }
  else {
    assert(n->get_status() >= NODE_VISITED);

#if PRINT_DEBUG_STATEMENTS == 1
    printf("HASH insert if absent on key %llu. failed\n",
	   key);
#endif
    return 0;
  }
}





// Creates a root node for the dag.
template <class DynNodeType>
DynPathCountNode<DynNodeType>::DynPathCountNode(CountPathDAGParams *params,
						DynPathHashTable<DynNodeType>* H)
  : DynNodeType(0, H),
    params(params),
    children(NULL) {
}

template <class DynNodeType>
DynPathCountNode<DynNodeType>::DynPathCountNode(long long k,
						CountPathDAGParams* params,
						DynPathHashTable<DynNodeType>* H)
  : DynNodeType(k, H),
    params(params),
    children(NULL) {
}


template <class DynNodeType>
void DynPathCountNode<DynNodeType>::Init() {
  int i = 0;

#if PRINT_DEBUG_STATEMENTS == 1
  printf("Calling Init() for key %llu\n", this->key);
#endif
  this->result = 0;
  this->path_length = 0;

  for (i = 0; i < this->children->size_estimate(); ++i) {
    DynPathCountNode<DynNodeType>* child = (DynPathCountNode<DynNodeType>*)this->children->get(i);
    add_dep(child->key);
  }

  
}

template <class DynNodeType>
void DynPathCountNode<DynNodeType>::Compute() {
  int result_val = 0;
  int path_val = 0;

  if (this->key == params->MAX_DAG_ID) {

#if PRINT_DEBUG_STATEMENTS == 1
    printf("COMPUTE FOUND key = %llu",
	   this->key);
#endif

    result_val = 1;
    path_val = 1;
  }


#if PRINT_DEBUG_STATEMENTS == 1
  printf("Computing on node %llu, with %d children \n",
	 this->key,
	 this->predecessors->size_estimate());
#endif

  for (int i = 0; i < this->predecessors->size_estimate(); i++) {

    long long pred_key = this->predecessors->get(i);   
    DynPathCountNode<DynNodeType>* child = (DynPathCountNode<DynNodeType>*)this->H->get_task(pred_key);

    if (child == NULL) {
      printf("ERROR: current_key = %llu, compute is searching for task with key %llu. Didn't find it\n",
	     this->key,
	     pred_key);
    }
    assert(child != NULL);
    result_val += child->result;
    path_val = MAX(path_val, 1+child->path_length);
  }
  
  int compute_val = DetCountWork(params, this->key);
  this->result = result_val + compute_val;
  this->path_length = path_val;  
}


template <class DynNodeType>
int DynPathCountNode<DynNodeType>::GetResult() {
  return this->result;
}

template <class DynNodeType>
int DynPathCountNode<DynNodeType>::GetPathLength() {
  return this->path_length;
}

template <class DynNodeType>
void DynPathCountNode<DynNodeType>::Generate() {
  
  //   printf("Executing Generate on key %llu\n", this->key);

  if (params->do_generate) {

    ConcurrentLinkedList* gen_list;
    LOpStatus l_code = OP_FAILED;
    do {
      gen_list = (ConcurrentLinkedList*)params->gen_map->search(this->key,
								&l_code);
    } while (l_code == OP_FAILED);


    if (gen_list != NULL) {
      ListNode* c = gen_list->get_list_head();
      while (c != NULL) {
	long long gen_key = c->hashkey;
	//	printf("From node %llu: generating random child %llu\n",
	//	       this->key, gen_key);
	this->generate_task(gen_key);
	c = c->next;
      }
    }
    else {
      //      printf("Null list? gen_list = %p, this->key == %llu\n",
      //	     gen_list, this->key);
    }        
  }
}


// Method which creates a random DAG from params->children_map
// This method fills in the "sdag_map hash table.
// 
template <class DynNodeType>
void DynPathCountNode<DynNodeType>::GenerateTestDag(CountPathDAGParams* params) {
  
  if (params->dag_type == 0) {
    GenerateChildrenMap(params);
  }
  else {
    GeneratePipelineChildrenMap(params);
  }

  //  GenerateChildrenMap(params);
  params->sdag_map = new ConcurrentHashTable(10 + params->MAX_DAG_ID / 100);
  params->dynamicHashTable = (void*) new DynPathHashTable<DynNodeType>(params->sdag_map);
  assert(params->dynamicHashTable);

  int num_nodes = 0;
  int num_edges = 0;
    
  for (int k = params->MAX_DAG_ID; k >= 0; k--) {

    DynPathCountNode<DynNodeType>* current_node = NULL;
    ConcurrentLinkedList* current_list;
    
    LOpStatus l_code = OP_FAILED;   
    while (l_code == OP_FAILED) {
      current_list = (ConcurrentLinkedList*)params->children_map->search(k,
									 &l_code);
    }

    if (current_list != NULL) {
      current_node = new DynPathCountNode<DynNodeType>(k, params,
						       (DynPathHashTable<DynNodeType>*)params->dynamicHashTable);
      // Create the children array.
      current_node->children = new DynamicArray<DynPathCountNode<DynNodeType>*>(2);
      num_nodes++;
      
      LOpStatus n_code = OP_FAILED;
      while (n_code == OP_FAILED){
	params->sdag_map->insert_if_absent(k,
					   current_node,
					   &n_code);
	//	printf("Inserted node with key %d\n", k);
      }
      assert(n_code == OP_INSERTED);

      ListNode* c = current_list->get_list_head();
      while (c != NULL) {
	long long child_key = c->hashkey;
	DynPathCountNode<DynNodeType>* child_node = NULL;
	assert(child_key > k);

	// Check to make sure the node is 
	LOpStatus lcheck_code = OP_FAILED;
	while (lcheck_code == OP_FAILED) {
	  child_node = (DynPathCountNode<DynNodeType>*)params->sdag_map->search(child_key,
										&lcheck_code);
	}

	assert(lcheck_code == OP_FOUND);
	assert(child_node != NULL);

	current_node->children->add(child_node);
	num_edges++;

	c = c->next;
      }      
    }

    if (k == params->MAX_DAG_ID) {
      params->sink = (void*)current_node;
    }

    if (k == 0) {
      params->root = (void*)current_node;
    }    
  }
  
  assert(num_nodes == params->num_nodes);
  if (num_edges != params->num_edges) {
    printf("ERROR: num_edges is %d, params->num_edges = %d\n",
	   num_edges, params->num_edges);
  }
  assert(num_edges == params->num_edges);
}




#endif
