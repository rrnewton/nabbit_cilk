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

#ifndef __CONCURRENT_HASH_TABLE_H
#define __CONCURRENT_HASH_TABLE_H


/**
 * A simple implementation of a concurrent hash table.  This table is
 * optimized to support "insert_if_absent" operations.
 *
 */

#include "concurrent_linked_list.h"


class ConcurrentHashTable {

 private:
  ConcurrentLinkedList* volatile* buckets;
  int num_buckets;



  // Tries to create a linked list at the given bucket.
  // This operation may return OP_FOUND, if a list is already there,
  // or OP_INSERTED, if a new list was created for the bucket.
  //
  // Returns a pointer to the list for the bucket.   
  ConcurrentLinkedList*  try_create_list(int bucket_index,
					 LOpStatus* code) {
    int retry_count = 0;
    ConcurrentLinkedList* empty_list = new ConcurrentLinkedList();
    assert(empty_list != NULL);

    bool is_empty = (buckets[bucket_index] == NULL);

    while (is_empty && (retry_count < 10)) {
      is_empty = !(__sync_bool_compare_and_swap(&buckets[bucket_index],
						NULL,
						empty_list));
      retry_count++;
    }

    if (buckets[bucket_index] == NULL) {
      delete empty_list;
      *code = OP_FAILED;
      return NULL;
    }

    if (buckets[bucket_index] != empty_list) {
      // Someone else succeeded in creating the list.
      delete empty_list;
      *code = OP_FOUND;
    } else {
      *code = OP_INSERTED;
    }    
    return buckets[bucket_index];    
  }
  

 public:
  ConcurrentHashTable(int initial_num_buckets) {
    buckets = NULL;
    num_buckets = 0;
    assert(initial_num_buckets > 0);
    if (initial_num_buckets > 0) {
      num_buckets = initial_num_buckets;

      buckets = new ConcurrentLinkedList* [num_buckets];
      assert(buckets != NULL);

      for (int i = 0; i < num_buckets; i++) {
	buckets[i] = NULL;
      }
    }

  }

  ~ConcurrentHashTable() {
    // Delete the list for each bucket.
    for (int i = 0; i < num_buckets; i++) {
      if (buckets[i] != NULL) {
	delete buckets[i];
      }      
    }

    // Delete the array of buckets.
    delete [] buckets;    
  }


  void print_table() {
    int num_nonempty_buckets = 0;
    printf("HashTable %p: num_buckets = %d\n",
	   this, num_buckets);
    for (int i = 0; i < num_buckets; i++) {
      if (buckets[i] != NULL) {
	printf("--- Bucket %d: \n", i);
	buckets[i]->print_list();
	printf("\n");
	num_nonempty_buckets++;
      }
    }
    printf("Nonempty_bucket_count = %d\n",
	   num_nonempty_buckets);
  }

  // For now, use a simple hash function.
  inline int hashcode(long long key) {
    return key % num_buckets;
  }

  void* search(long long k,
	       LOpStatus *code) {
    int idx = hashcode(k);
    if (buckets[idx] == NULL) {
      *code = OP_NOT_FOUND;
      return NULL;
    } else {
      return buckets[idx]->search(k, code);
    }
  }


  void* insert_if_absent(long long k,
			 void* val,
			 LOpStatus *code) {

    int retry_count = 0;
    int idx = hashcode(k);

    while ((buckets[idx] == NULL) && (retry_count < 10)) {
      try_create_list(idx, code);
      retry_count++;
    }


    if (buckets[idx] == NULL) {
      *code = OP_FAILED;
      return NULL;
    }
    
    // Otherwise, if we get to this point, we have a list for that
    // bucket.  Atomically try to insert into the list.
    return buckets[idx]->insert_if_absent(k,
					  val,
					  code);
  }


  // Return a list of keys of elements in the hash table.
  long long* get_keys(long long* final_size) {

    long long size_to_return = 0;
    long long* a = NULL;


    for (int idx = 0; idx < num_buckets; idx++) {
      if (buckets[idx] != NULL) {
	buckets[idx]->update_size_estimate();
	size_to_return += buckets[idx]->get_size_estimate();
      }
    }


    long long current_start = 0;
    if (size_to_return > 0) {
      long long current_bucket_size = 0;
      a = new long long[size_to_return];
      assert(a != NULL);

      for (int idx = 0; idx < num_buckets; idx++) {
	long long retrieved_size;

	if (buckets[idx] != NULL) {
	  current_bucket_size = buckets[idx]->get_size_estimate();

	  //	printf("Copying: Bucket %d, bucket size is %d\n",
	  //	       idx, current_bucket_size);
	  // truncate the end if there are too many elements.
	  if ((current_start + current_bucket_size) > size_to_return) {
	    current_bucket_size = size_to_return - current_start;
	    printf("Current_bucket_size = %llu, size_to_return = %llu, current_start = %llu\n",
		   current_bucket_size,
		   size_to_return,
		   current_start);
	    printf("Truncated bucket %d to %llu\n",
		   idx, current_bucket_size);
	  }

	  buckets[idx]->get_n_keys(a + current_start,
				   current_bucket_size,
				   &retrieved_size);
	  assert(retrieved_size == current_bucket_size);
	  current_start += retrieved_size;
	  if (current_start >= size_to_return) {
	    break;
	  }
	}
      }
    }

    printf("Final current = %llu, final size to return is %llu\n",
	   current_start,
	   size_to_return);
   
    *final_size = size_to_return;
    return a;    
  }
};


#endif
