# Makefile, modified for the Cilk++ example Makefiles.


# The directory where the .h to test are located.
DEFAULT_DIR=../include

# The directory for other common utilities
UTIL_DIR=../util

# The names of the tests to run.
TEST_NAMES = dynamic_array concurrent_linked_list concurrent_hash_table
OTHER_TESTS = malloc_test

CILKPP	= cilk++
LIBARG	=  -O2 -Wall # -lmiser

# The extra include files
INCLUDES = -I $(UTIL_DIR) -I $(DEFAULT_DIR)
UTILS = example_util_gettime.h qsort.h 
UTIL_FILES = $(addprefix $(UTIL_DIR)/,$(UTILS))

TARGETS = $(addprefix test_, $(TEST_NAMES)) $(OTHER_TESTS)


.PHONY: all clean


all: $(TARGETS)

# Pattern rule for building tests cases.
test_%: %_test.cilk $(UTIL_FILES) $(DEFAULT_DIR)/%.h
	$(CILKPP) $< $(INCLUDES) $(LIBARG) -o $@


malloc_test: malloc_test.cilk $(UTIL_FILES)
	$(CILKPP) $< $(INCLUDES) $(LIBARG) -o $@

clean:
	rm -f $(TARGET) $(TARGETS)
