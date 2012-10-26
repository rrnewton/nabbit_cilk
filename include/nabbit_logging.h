// Code for the Nabbit task graph library
//
// Generic code for logging statistics for Nabbit.
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

#ifndef _NABBIT_LOGGING_H_
#define _NABBIT_LOGGING_H_

//#include <cstdlib>
//#include <iostream>
#include <sys/time.h>
#include <vector>
#include <cilk.h>

#include "nabbit_timers.h"


/**********************************************
 * Module for logging in Nabbit. 
 *
 *  The NabbitTaskGraphStats class defines a log object which
 *  maintains buffers on each worker thread for storing log records.
 *
 *  The log has buffers for two types of objects:
 * 
 *    1. NabbitTimeRecord: this record calls gettimeofday and rdtsc.
 *       The idea is to use this record to correlate between the
 *       processor's cycle counter and the current time of day.
 * 
 *    2. NabbitNodeRecord: this record stores the processor id, and
 *       records a start and end time (which we assume is obtained by
 *       reading the processor's cycle counter).
 *
 *  In theory, one can approximately reconstruct how a DAG is executed
 *  by logging the start/end times of the compute of each node online,
 *  and then then post-processing the log.
 *
 *  There could be weirdness if worker threads get migrated between
 *  processors.  The code currently doesn't handle this case.  More
 *  post-processing work would need to be done to make this code more
 *  robust...
 *
 *
 */

// Generic statistics for the computation of a block.
// The template parameter is a simple struct / class
// that encapsulates any additional user-data
// we want to log.
//
template <class RecType>
class NabbitNodeRecord {
 public:
  int compute_id;   // The id of the worker that computed the node.
  rTimeStruct start_ts;  // Starting timestamp.
  rTimeStruct end_ts;    // Ending timestamp.
  RecType data;

  void core_print() {
    printf("compute = %d, time = (%llu, +%llu)",
	   this->compute_id,
	   this->start_ts,
	   this->end_ts - this->start_ts);
  }

  void print() {
    core_print();
  }
};


struct NabbitTimeRecord {
  int proc_id;   // Processor id.
  rTimeStruct ts_before; // Time stamp before.
  rTimeStruct ts_after;  // Time stamp after.  
  struct timeval tv;  // Time value from gettimeofday.
};


typedef std::vector<struct NabbitTimeRecord>* NabbitTimeBuffer;


// When replaying the log to generate output images,
// we will have an array of P of these objects, one for
// each worker.
struct NabbitReplayObj {
  int current_noderec;
  rTimeStruct base_rtime;
  double base_walltime;
  double cycles_per_sec;
};




// Data structure which stores log entries.
//
// This structure is effectively P buffers, one for each
// worker thread we are running with.

template <class RecType>
class NabbitTaskGraphStats {

  static const int NEXT_TS_LIMIT_PADDING = 20;
  static const int BARRIER_BACKOFF_VAL = 10;
  static const unsigned int TREC_CYCLE_INTERVAL = 1000000;

  // Type of the node records.
  typedef std::vector<NabbitNodeRecord<RecType> >* NabbitNodeBuffer;
  
 private:
  
  bool collection_enabled;
  int P;

  NabbitNodeBuffer* node_log;
  NabbitTimeBuffer* time_log;
  rTimeStruct* next_ts_limit;

  volatile int time_barrier_counter;

  inline rTimeStruct get_next_ts_limit(int p) {
    return this->next_ts_limit[NEXT_TS_LIMIT_PADDING*p];
  }

  inline void set_next_ts_limit(int p, rTimeStruct val) {
    this->next_ts_limit[NEXT_TS_LIMIT_PADDING*p] = val;
  }

 public:

  // Constructor takes in P, the number of worker threads
  // we are running with.
  NabbitTaskGraphStats(int P_) : collection_enabled(false),
    P(P_), node_log(NULL), time_log(NULL) {
    assert(P > 0);
      assert(P <= 128);

      printf("Creating TaskGraphStats for %d workers\n",
	     this->P);
      
      node_log = new NabbitNodeBuffer[P];      
      assert(node_log);
      time_log = new NabbitTimeBuffer[P];
      assert(time_log);

      next_ts_limit = new rTimeStruct[NEXT_TS_LIMIT_PADDING*P];
      
      for (int p = 0; p < P; p++) {
	node_log[p] = new std::vector<NabbitNodeRecord<RecType> >;
	time_log[p] = new std::vector<struct NabbitTimeRecord>;

	node_log[p]->clear();
	time_log[p]->clear();
	this->set_next_ts_limit(p, 0);
      }
      printf("Finished creating TaskGraphStats for %d workers\n",
	     this->P);
  }

  inline void set_collection(bool do_collect) {
    this->collection_enabled = do_collect;
  }
  inline bool is_collecting() const {
    return this->collection_enabled;
  }

  inline int get_num_timerecs(int p) {
    return (int)time_log[p]->size();
  }
  NabbitTimeRecord get_timerec(int p, int elem_num) {
    assert(elem_num < (int)time_log[p]->size());
    return time_log[p]->at(elem_num);
  }
  inline int get_num_noderecs(int p) {
    return (int)node_log[p]->size();
  }

  NabbitNodeRecord<RecType> get_noderec(int p, int elem_num) {
    if (elem_num >= (int)node_log[p]->size()) {
      printf("HERE: p = %d, elem_num = %d, size  = %d\n",
	     p,
	     elem_num,
	     (int)node_log[p]->size());
    }
    assert(elem_num < (int)node_log[p]->size());
    return node_log[p]->at(elem_num);
  }
    
  void print_timelog(int proc_id) {
    printf("Proc %d time log: length %zd\n",
	   proc_id,
	   time_log[proc_id]->size());

    for (unsigned int k = 0; k < time_log[proc_id]->size(); ++k) {
      NabbitTimeRecord trec = time_log[proc_id]->at(k);
      printf("%d: (%llu, %llu) (diff %llu) = %f\n",
	     k,
	     trec.ts_before,
	     trec.ts_after,
	     trec.ts_after - trec.ts_before,
	     1.0e6*trec.tv.tv_sec + trec.tv.tv_usec);
    }
  }

  void print_nodelog(int proc_id) {
    printf("Proc %d node log: length %zd\n",
	   proc_id,
	   node_log[proc_id]->size());   
    for (unsigned int k = 0; k < node_log[proc_id]->size(); ++k) {
      NabbitNodeRecord<RecType>* node_rec = &(node_log[proc_id]->at(k));
      printf("%d: ", k);
      node_rec->print();
      printf("\n");
    }
  }


  inline void add_noderec(NabbitNodeRecord<RecType>* node_rec) {
    node_log[node_rec->compute_id]->push_back(*node_rec);
    
    // Periodically, store the current time.
    if (node_rec->end_ts > (this->get_next_ts_limit(node_rec->compute_id))) {
      add_timerec(node_rec->compute_id);
      this->set_next_ts_limit(node_rec->compute_id,
			      node_rec->end_ts + TREC_CYCLE_INTERVAL);
    }
  }



  void add_timerec(int p) {
    NabbitTimeRecord trec;
    trec.proc_id = p;
    NabbitTimers::cycleCounter(&trec.ts_before);
    gettimeofday(&trec.tv, NULL);
    NabbitTimers::cycleCounter(&trec.ts_after);
    time_log[p]->push_back(trec);
  }
  
  ~NabbitTaskGraphStats() {
    assert(node_log);
    assert(time_log);
    assert(next_ts_limit);
    for (int p = 0; p < P; p++) {
      delete node_log[p];
      delete time_log[p];      
    }
    delete[] node_log;
    delete[] time_log;
    delete next_ts_limit;
  }


  // This method should only be executed from a serial section of the
  // main program, where we are guaranteed to have all P workers
  // stealing.  This method adds a time record for the current
  // worker, and then waits for the barrier count to hit P.
  
  void time_barrier_local(int P) {
    volatile int val = 0;
    int backoff_counter = 0;
    int spin_val = 0;
    int my_id = cilk::current_worker_id();

    __sync_add_and_fetch(&this->time_barrier_counter, 1);

    // Spin and wait for the counter to hit P.
    val = this->time_barrier_counter;
    while (val != P) {
      for (int spin_count = 0; spin_count < (1 << backoff_counter); spin_count++) {
	spin_val++;
      }

      if (backoff_counter < BARRIER_BACKOFF_VAL) {
	backoff_counter++;
      }
      val = this->time_barrier_counter;      
    }

    // Add a time record.
    add_timerec(my_id);    
  }

  // Global barrier which waits for all P workers to each record
  // a timerec.
  void global_time_barrier(int P) {
    assert((int)cilk::current_worker_count() == P);
    this->time_barrier_counter = 0;
    for (int my_id = 0; my_id < P; my_id++) {
      cilk_spawn time_barrier_local(P);
    }
    cilk_sync;
  }
};

  
#endif
