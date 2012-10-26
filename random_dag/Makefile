# Makefile, modified for the Cilk++ example Makefiles.


# The directory where the .h to test are located.
DEFAULT_DIR=../include

# The directory for other common utilities
UTIL_DIR=../util

# The names of the tests to run.
TEST_NAMES = det_path

CILKPP	= cilk++
LIBARG	=  -O2 -Wall # -lmiser

# The extra include files
INCLUDES = -I $(UTIL_DIR) -I $(DEFAULT_DIR)
UTILS = example_util_gettime.h qsort.h 
UTIL_FILES = $(addprefix $(UTIL_DIR)/,$(UTILS))

H_FILES = det_count.h count_paths_node.h dyn_count_node.h


TARGET = detpath_test

.PHONY: clean


# Pattern rule for building tests cases.
detpath_test: detpath_test.cilk $(UTIL_FILES) $(H_FILES)
	$(CILKPP) $< $(INCLUDES) $(LIBARG) -o $@

clean:
	rm -f $(TARGET) 
