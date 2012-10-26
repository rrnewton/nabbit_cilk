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
 * Compute method for nodes in the random-DAG microbenchmark, and the
 * methods for creating the static DAG for Nabbit to evaluate.
 *
 */  

#ifndef __COUNT_PATHS_NODE_H_
#define __COUNT_PATHS_NODE_H_


#include <dag_node.h>
#include "det_count.h"


template <class NodeType>
class DetPathsDAGNode: public DAGNode<NodeType> {
  
 protected:
  // Global parameters for this computation.
  CountPathDAGParams* params;
  
  int result;
  int path_length;

  DetPathsDAGNode(long long k,
		  CountPathDAGParams* params);

  void InitNode();
  void SpawnChildren();
  void Compute();

  
 public:

  DetPathsDAGNode(CountPathDAGParams *params);
  static void GenerateTestDag(CountPathDAGParams* params);
  int GetResult();
  int GetPathLength();


  // Helper method for GenerateTestDag.
  DetPathsDAGNode* create_test_child(long long k);

  // Methods which visits all the children of the dag, and stores
  // the resulting nodes into node_check_map.
  friend void SerialVisitDag<DetPathsDAGNode>(DetPathsDAGNode* current_root,
					      ConcurrentHashTable* node_check_map);
  
  // Checks the result at each node, and sees whether it is the sum of
  // the values at all of its children (and checks whether the sink
  // node has value of 1).
  friend bool CheckResult<DetPathsDAGNode>(DetPathsDAGNode* root,
					   bool verbose);

 private:
  DynamicArray<NodeType* >* children;
  //  DynamicArray<DetPathsDAGNode*>* children;  
  void assert_precompute_status(void);
};



// Creates a root node for the dag.
template <class NodeType>
DetPathsDAGNode<NodeType>::DetPathsDAGNode(CountPathDAGParams *params)
  : DAGNode<NodeType>((long long)0),
    params(params) {
}


template <class NodeType>
DetPathsDAGNode<NodeType>::DetPathsDAGNode(long long k,
					   CountPathDAGParams* params)
  : DAGNode<NodeType>(k),
    params(params) {
}


template <class NodeType>
DetPathsDAGNode<NodeType>* DetPathsDAGNode<NodeType>::create_test_child(long long k) {
  DetPathsDAGNode<NodeType>* temp_child;
  temp_child = new DetPathsDAGNode(k,
				   this->params);
  return temp_child;
}


template <class NodeType>
void DetPathsDAGNode<NodeType>::InitNode() {
  this->result = 0;
  this->path_length = 0;
  this->children = this->predecessors;
}

template <>
void DetPathsDAGNode<DynamicSerialNode>::assert_precompute_status(void) {
  assert(this->get_status() == NODE_VISITED);
}
template <>
void DetPathsDAGNode<DynamicNabbitNode>::assert_precompute_status(void) {
  assert(this->get_status() == NODE_EXPANDED);
}
template <>
void DetPathsDAGNode<StaticNabbitNode>::assert_precompute_status(void) {
}
template <>
void DetPathsDAGNode<StaticSerialNode>::assert_precompute_status(void) {
}


template <class NodeType>
void DetPathsDAGNode<NodeType>::Compute() {
  assert_precompute_status();
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
    DetPathsDAGNode<NodeType>* child = (DetPathsDAGNode<NodeType>*)this->predecessors->get(i);
    result_val += child->result;
    path_val = MAX(path_val, 1+child->path_length);
  }

  int compute_val = DetCountWork(params, this->key);
  this->result = result_val + compute_val;
  this->path_length = path_val;  
}


template <>
void DetPathsDAGNode<StaticSerialNode>::Compute() {
  assert_precompute_status();
  int result_val = 0;
  int path_val = 0;

  if (this->key == params->MAX_DAG_ID) {

#if PRINT_DEBUG_STATEMENTS == 1
    printf("COMPUTE FOUND key = %llu",
	   key);
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
    DetPathsDAGNode<StaticSerialNode>* child = (DetPathsDAGNode<StaticSerialNode>*)this->predecessors->get(i);
    result_val += child->result;
    path_val = MAX(path_val, 1+child->path_length);
  }

  int compute_val = DetCountWork(params, this->key);
  this->result = result_val + compute_val;
  this->path_length = path_val;  
}

template <>
void DetPathsDAGNode<StaticNabbitNode>::Compute() {
  assert_precompute_status();
  int result_val = 0;
  int path_val = 0;

  if (this->key == params->MAX_DAG_ID) {

#if PRINT_DEBUG_STATEMENTS == 1
    printf("COMPUTE FOUND key = %llu",
	   key);
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
    DetPathsDAGNode<StaticNabbitNode>* child = (DetPathsDAGNode<StaticNabbitNode>*)this->predecessors->get(i);
    result_val += child->result;
    path_val = MAX(path_val, 1+child->path_length);
  }

  int compute_val = DetCountWork(params, this->key);
  this->result = result_val + compute_val;
  this->path_length = path_val;  
}




template <class NodeType>
int DetPathsDAGNode<NodeType>::GetResult() {
  return this->result;
}

template <class NodeType>
int DetPathsDAGNode<NodeType>::GetPathLength() {
  return this->path_length;
}





// Method which creates a random DAG from params->children_map
//
template <class NodeType>
void DetPathsDAGNode<NodeType>::GenerateTestDag(CountPathDAGParams* params) {

  if (params->dag_type == 0) {
    GenerateChildrenMap(params);
  }
  else {
    GeneratePipelineChildrenMap(params);
  }

  //  GenerateChildrenMap(params);
  params->sdag_map = new ConcurrentHashTable(10 + params->MAX_DAG_ID / 100);

  int num_nodes = 0;
  int num_edges = 0;
  
  for (int k = params->MAX_DAG_ID; k >= 0; k--) {

    DetPathsDAGNode<NodeType>* current_node = NULL;
    ConcurrentLinkedList* current_list;
    
    LOpStatus l_code = OP_FAILED;   
    while (l_code == OP_FAILED) {
      current_list = (ConcurrentLinkedList*)params->children_map->search(k,
									 &l_code);
    }

    if (current_list != NULL) {
      current_node = new DetPathsDAGNode<NodeType>(k, params);
      current_node->init_node();
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
	DetPathsDAGNode<NodeType>* child_node = NULL;
	assert(child_key > k);

	// Check to make sure the node is 
	LOpStatus lcheck_code = OP_FAILED;
	while (lcheck_code == OP_FAILED) {
	  child_node = (DetPathsDAGNode<NodeType>*)params->sdag_map->search(child_key,
									    &lcheck_code);
	}

	assert(lcheck_code == OP_FOUND);
	assert(child_node != NULL);

	current_node->add_child(child_node);
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
