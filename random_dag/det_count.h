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
 * Counting paths through a random DAG, using static Nabbit.
 *
 */  

#ifndef __DET_COUNT_H_
#define __DET_COUNT_H_


#include <concurrent_hash_table.h>


#define MAX(a, b) (((a) > (b)) ? (a) : (b))

const int LargePrime = 251700653;

#ifdef DEFAULT_COMPUTE_BASE
const int COMPUTE_BASE = DEFAULT_COMPUTE_BASE;
#else
const int COMPUTE_BASE = 100;
#endif



// Structure defining parameters for the dag.
struct CountPathDAGParams {
  int MAX_DAG_ID;
  int MAX_DEGREE;
  int WORK_VALUE;
  int NUM_GEN;
  
  int PIPE_WIDTH;
  int PIPE_LOOKAHEAD;
  bool nodes_with_parallelism;
  void* root;
  void* sink;

  bool use_multiple_roots;
  bool do_generate;
  DynamicArray<long long>* root_keys;

  // Stores the nodes of the dag.
  ConcurrentHashTable* sdag_map;
  void* dynamicHashTable;

  // Stores lists of children for each index. 
  ConcurrentHashTable* children_map;

  // Stores keys to randomly generate for each child.
  ConcurrentHashTable* gen_map;
  
  bool use_random_online_map;
  int num_nodes;
  int num_edges;
  int dag_type;

};



static ConcurrentLinkedList* find_or_create_child_list(CountPathDAGParams* params,
						       int rand_child);


static ConcurrentLinkedList* create_gen_list(CountPathDAGParams* params,
					     int recurse_on_child_map) {

  ConcurrentLinkedList* gen_list = new ConcurrentLinkedList();
  for (int i = 0; i < params->NUM_GEN; i++) {
    int rval = 1 + rand() % (params->MAX_DAG_ID-1);
    LOpStatus code = OP_FAILED;
    do {
      gen_list->insert_if_absent(rval,
				 (void*)rval,
				 &code);

      if (recurse_on_child_map) {
	find_or_create_child_list(params,
				  rval);
      }
    } while (code == OP_FAILED);
  }
  return gen_list;
}


static ConcurrentLinkedList* find_or_create_child_list(CountPathDAGParams* params,
						       int rand_child) {

  ConcurrentLinkedList* randchild_list = NULL;

  //  printf("Calling find_or_create_list for child %d\n", rand_child);

  LOpStatus childmap_code = OP_FAILED;
  while (childmap_code == OP_FAILED) {
    randchild_list = (ConcurrentLinkedList*)params->children_map->search(rand_child,
									 &childmap_code);
  }

  childmap_code = OP_FAILED;
  if (randchild_list == NULL) {
    randchild_list = new ConcurrentLinkedList();
    params->num_nodes++;

    do {
      params->children_map->insert_if_absent(rand_child,
					     randchild_list,
					     &childmap_code);
      //      printf("calling insert_if_absent for rand_child %d\n",
      //	     rand_child);
    } while (childmap_code == OP_FAILED);
    assert(childmap_code == OP_INSERTED);

    if (params->do_generate) {
      LOpStatus genlist_code = OP_FAILED;
      ConcurrentLinkedList* gen_list = create_gen_list(params, true);
      params->gen_map->insert_if_absent(rand_child,
					gen_list,
					&genlist_code);
      assert(genlist_code == OP_INSERTED);

      //      ConcurrentLinkedList* test_list = NULL;
      //      test_list = (ConcurrentLinkedList*)params->gen_map->search(rand_child,
      //								 &genlist_code);
      //      assert(genlist_code == OP_FOUND);
    }
  }
  assert(randchild_list != NULL);
  return randchild_list;
}

// Method which creates a random DAG.
//
// The mapping from key to dag node is stored in params->sdag_map.
// The root of the dag is stored in params->root.
//
// This code can be modified to store the list of child indices
// separately, into params->children_map.
// (For now, I disabled this computation with a boolean flag.)
static void GenerateChildrenMap(CountPathDAGParams* params) {
  
  params->children_map = new ConcurrentHashTable(10 + (params->MAX_DAG_ID+2)/100);
  params->gen_map = new ConcurrentHashTable(10 + (params->MAX_DAG_ID+2)/100);

  params->num_nodes = 1;

  ConcurrentLinkedList* zero_list = new ConcurrentLinkedList();
  LOpStatus lcode = OP_FAILED;
  while (lcode == OP_FAILED) {
    params->children_map->insert_if_absent(0,
					   zero_list,
					   &lcode);
  }
  assert(lcode == OP_INSERTED);
  params->num_nodes = 1;

  if (params->use_multiple_roots) {
    params->root_keys = new DynamicArray<long long>(params->MAX_DAG_ID/100);
    // Insert other random nodes as roots
    for (int i = 0; i < params->MAX_DAG_ID/100; i++) {
      int rand_root = rand() % params->MAX_DAG_ID;

      //      printf("Creating a list for a random root %d\n", rand_root);
      ConcurrentLinkedList* randchild_list = NULL;
      randchild_list = find_or_create_child_list(params, rand_root);
      assert(randchild_list != NULL);
      params->root_keys->add(rand_root);
    }
  }
  else {
    params->root_keys = NULL;
  }

  
  for (int i = 0; i < params->MAX_DAG_ID; i++) {    

    ConcurrentLinkedList* current_list;
    
    LOpStatus l_code = OP_FAILED;
    while (l_code == OP_FAILED) {
      current_list = (ConcurrentLinkedList*)params->children_map->search(i,
									 &l_code);
    }

    if (current_list != NULL) {
      int rand_degree = 1 + rand() % params->MAX_DEGREE;
      int range = (params->MAX_DAG_ID - i);
      
      if (range > 0) {
	for (int j = 0; j < rand_degree; j++) {
	  long long rand_child = (i + 1) + rand() % range;


	  // Look for the children list associated with rand_child.
	  // If there is no list there, create one.

	  ConcurrentLinkedList* randchild_list = NULL;
	  randchild_list = find_or_create_child_list(params, rand_child);
	  assert(randchild_list != NULL);
	 
	  // Update the current list of children indices.
	  LOpStatus lcode = OP_FAILED;
	  do {
	    current_list->insert_if_absent(rand_child,
					   (void*)rand_child,
					   &lcode);
	  } while (lcode == OP_FAILED);

	  if (lcode == OP_INSERTED) {
	    //	    printf("For key = %llu, inserted randchild=%llu\n",
	    //		   i, rand_child);		   
	    params->num_edges++;
	  } else {
	    //	    printf("For key = %llu, already found randchild=%llu\n",
	    //		   i, rand_child);
	    assert(lcode == OP_FOUND);
	  }
	}
      }

      //      printf("The final list at %d: ", i);
      //      current_list->print_list();
      //      printf("\n");
    }
  }
}




// Method which creates a random DAG.
//
// The mapping from key to dag node is stored in params->sdag_map.
// The root of the dag is stored in params->root.
//
// This code can be modified to store the list of child indices
// separately, into params->children_map.
// (For now, I disabled this computation with a boolean flag.)
static void GeneratePipelineChildrenMap(CountPathDAGParams* params) {
  params->children_map = new ConcurrentHashTable(5 + (params->MAX_DAG_ID/5));
  
  int num_stages = params->MAX_DAG_ID/params->PIPE_WIDTH;
  assert(num_stages > 0);
  int max_node_id = num_stages * params->PIPE_WIDTH;
  
  // Create source.
  ConcurrentLinkedList* zero_list = new ConcurrentLinkedList();
  LOpStatus lcode = OP_FAILED;
  while (lcode == OP_FAILED) {
    params->children_map->insert_if_absent(0,
					   zero_list,
					   &lcode);
  }
  assert(lcode == OP_INSERTED);
  params->num_nodes = 1;
  params->num_edges = 0;

  // Create children for source node.
  for (int i = 1; i <= params->PIPE_WIDTH; i++) {
    // Update the current list of children indices.
    LOpStatus lcode = OP_FAILED;
    zero_list->insert_if_absent(i,
				(void*)i,
				&lcode);
    assert(lcode == OP_INSERTED);
    params->num_edges++;
  }


  // Create all the nodes and edges along the chains.
  
  // At iteration i, create nodes W*i, W*i-1, W*i-2, .. W*i - (W-1)
  // This corresponds to level i in the DAG. 
  for (int i = 1; i <= num_stages ; i++) {
    for (int j = 0; j< params->PIPE_WIDTH; j++) {
      int idx = params->PIPE_WIDTH*i - j;

      ConcurrentLinkedList* current_list = new ConcurrentLinkedList();
      LOpStatus lcode = OP_FAILED;
      while (lcode == OP_FAILED) {
	params->children_map->insert_if_absent(idx,
					       current_list,
					       &lcode);
      }
      assert(lcode == OP_INSERTED);
      params->num_nodes++;


      // Now for the stage, add the "default" child.
      {
	int next_child = idx + params->PIPE_WIDTH;
	if (next_child <= max_node_id) {
	  LOpStatus list_code = OP_FAILED;
	  while (list_code == OP_FAILED) {
	    current_list->insert_if_absent(next_child,
					   (void*)next_child,
					   &list_code);
	  }
	  assert(list_code == OP_INSERTED);
	  params->num_edges++;
	}
	//	printf("At key = %d, added next_child = %d\n",
	//	       idx, next_child);
      }

      // Now add the random children.
      {
	for (int k = 0; k < params->MAX_DEGREE; k++) {
	  int rand_child = params->PIPE_WIDTH*i+1  + rand() % params->PIPE_LOOKAHEAD;
	  LOpStatus list_code = OP_FAILED;
	  if (rand_child <= max_node_id) {
	    while (list_code == OP_FAILED) {
	      current_list->insert_if_absent(rand_child,
					     (void*)rand_child,
					     &list_code);
	    }


	    //	    printf("At key = %d, added random_child = %d\n",
	    //		   idx, rand_child);

	  
	    if (list_code == OP_INSERTED) {
	      //	    printf("For key = %llu, inserted randchild=%llu\n",
	      //		   i, rand_child);
	      params->num_edges++;
	    } else {
	      //	      printf("For k = %llu, already found randchild=%llu\n",
	      //		     k, rand_child);
	      assert(list_code == OP_FOUND);
	    }	
	  }
	}
      }    
    }
  }
}



template <class CountNode>
static bool CheckResult(CountNode* root,
			bool verbose) {
  
  int num_nodes = 0;
  int num_edges = 0;
  int max_dag_id = root->params->MAX_DAG_ID;

  //  ConcurrentHashTable* node_check_map;
  //  node_check_map = new ConcurrentHashTable(max_dag_id);
  //  SerialVisitDag(root, node_check_map);
  
  // Check the sum at every node.
  for (int k = max_dag_id; k>= 0; k--) {
    LOpStatus code = OP_FAILED;
    CountNode* n = NULL;
    while (code == OP_FAILED) {
      n = (CountNode*)root->params->sdag_map->search(k,
						     &code);
    }
    if (n != NULL) {
      int test_result = (k == max_dag_id ? 1 : 0);      
      num_nodes++;
      num_edges += n->children->size_estimate();

      assert(k == n->key);
      if (k == max_dag_id) {
	assert(n->children->size_estimate() == 0);
      }
      
      for (int it1 = 0;
	   it1 < n->children->size_estimate();
	   ++it1) {

	CountNode* child = (CountNode*)n->children->get(it1);
	int child_result = child->result;

	assert(child->key > k);
	test_result += child_result;
      }

      // Add the constant term from the computation.
      test_result += DetCountWork(root->params,
				  k);

      if (test_result != n->result) {
	printf("ERROR: checking node %d, test_result = %d, n->result = %d\n",
	       k, test_result, n->result);
      }
      assert(test_result == n->result);
    }
  }

  if (verbose) {
    printf("Number of nodes = %d. Number of edges = %d\n",
	   num_nodes, num_edges);
  }
  
  if (root->params->use_random_online_map) {
    root->params->num_nodes = num_nodes;
    root->params->num_edges = num_edges;
  } else {
    assert(num_nodes == root->params->num_nodes);
    assert(num_edges == root->params->num_edges);
  }

  if (verbose) {
    printf("Final result: CORRECT\n");
  }
  return true;
}



template<class CountNode>
static void SerialVisitDag(CountNode* current_root,
			   ConcurrentHashTable* node_check_map) {
  if (current_root == NULL) {
    return;
  }

  CountNode* result;
  LOpStatus code = OP_FAILED;
  while (code == OP_FAILED) {
    result = (CountNode*)node_check_map->search(current_root->key,
						&code);
  }
  if (code == OP_FOUND) {
    return;
  }

  
  // Otherwise, not visited; put this node in.
  CountNode* insert_result;
  insert_result = (CountNode*)node_check_map->insert_if_absent(current_root->key,
							       (void*)current_root,
							       &code);
  assert(insert_result == current_root);


  // Then, visit all the children.
  for (int i = 0; i < current_root->children->size_estimate(); i++) {
    CountNode* temp_child = (CountNode*)current_root->children->get(i);
    SerialVisitDag(temp_child,
		   node_check_map);
  }
}


static inline int DetCountReduceVals(int v1, int v2) {
  long long v = v1 * v2;
  return (int)(v % LargePrime);
}


static int DetCountWorkBase(int n,
			    int gval) {
  long long val = 1;
  for (int i = 1; i < n; i++) {
    val = val * gval;
    val = val % LargePrime;
  }
  return (int)val;  
}

template <int CBASE>
static int DetCountWorkHelper(int n,
			      int gval) {
  if (n < CBASE) {
    return DetCountWorkBase(n, gval);
  }
  else {
    long long v1, v2;

    v1 = cilk_spawn DetCountWorkHelper<COMPUTE_BASE>(n/2,
						       gval);
    v2 = cilk_spawn DetCountWorkHelper<COMPUTE_BASE>(n - n/2,
						     gval);
    cilk_sync;

    return DetCountReduceVals(v1, v2);
  }
}

static int DetCountWork(CountPathDAGParams* params, long long gval) {
  int ans;
  if (params->nodes_with_parallelism) {
    ans = DetCountWorkHelper<COMPUTE_BASE>(params->WORK_VALUE,
					   (int)(gval % LargePrime/3));
  }
  else {
    ans = DetCountWorkBase(params->WORK_VALUE,
			   (int)(gval % LargePrime/3));
  }
  return ans;  
}


#endif
