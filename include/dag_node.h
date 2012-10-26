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

#ifndef __DAG_NODE_H_
#define __DAG_NODE_H_

/**************************************************
 * dag_node.h
 *
 *  This header file wraps all the various DAG nodes we have
 *  implemented so far into one templated class.
 * 
 *  This structure (hopefully?) makes it easier for users to switch
 *  between DAG nodes, by switching the template parameter.
 *
 *  Alternatively, the user can subclass from the DAG node type
 *  directly.
 */


#include "static_serial_node.h"
#include "static_nabbit_node.h"
#include "dynamic_serial_node.h"
#include "dynamic_nabbit_node.h"


// Possible status for a node.
typedef enum { SERIAL_STATIC_TRAVERSAL=0,
	       STATIC_NABBIT_TRAVERSAL=1,
	       SERIAL_DYNAMIC_TRAVERSAL=2,
	       DYNAMIC_NABBIT_TRAVERSAL=3
} DAGTraversalType;


// The supported types of nodes: 
//
// 1. StaticSerialNode
// 2. StaticNabbitNode
// 3. DynamicSerialNode
// 4. DynamicNabbitNode


template <class NodeType>
class DAGNode: public NodeType {

 protected:
  DAGNode(long long k);
  DAGNode(long long k,
	  int num_predecessors);  
};


template <class NodeType>
DAGNode<NodeType>::DAGNode(long long k)
  :  NodeType(k) {
}

template <class NodeType>
DAGNode<NodeType>::DAGNode(long long k, int num_predecessors)
  :  NodeType(k, num_predecessors) {
}


#endif

