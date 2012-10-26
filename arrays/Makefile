
CILKPP	= cilk++
LIBARG	=  -O3 -Wall # -lmiser
TARGETS = test_convert test_arrays
SRC	= $(addsuffix .cilk,$(TARGET))

INCLUDES = 


# The array indexing code doesn't rely on other Nabbit or Cilk++
# library code.  Otherwise, we might use these definitions.

# # The directory where the .h to test are located.
# DEFAULT_DIR=../include

# # The directory for other common utilities
# UTIL_DIR=../util
# The extra include files
#INCLUDES = # -I $(UTIL_DIR) -I $(DEFAULT_DIR)
#UTILS = example_util_gettime.h qsort.h 
#UTIL_FILES = $(addprefix $(UTIL_DIR)/,$(UTILS))


H_FILES = nabbit_array_defs.h morton.h array_layouts.h block_layouts.h	\
convert.h array2d_base.h array2d_row.h array2d_morton.h
TEST_FILES = $(addsuffix .cilk, $(TARGETS))



.PHONY: clean run_tests tarball


all: $(TARGETS)

test_%:  test_%.cilk $(H_FILES)
	$(CILKPP) $< $(INCLUDES) $(LIBARG) -o $@

check: $(TARGETS)
	./test_convert
	./test_arrays

clean:
	rm -rf $(TARGETS) $(PACKAGE_DIR) nabbit_arrays.tar.gz



## Code for creating the package file.

PACKAGE_DIR = nabbit_arrays
PACKAGE_FILES = $(H_FILES) $(TEST_FILES) README Makefile
PACKAGE_NAME = nabbit_arrays

tarball: nabbit_arrays.tar.gz

nabbit_arrays.tar.gz: $(H_FILES) $(TEST_FILES)
	mkdir -p $(PACKAGE_DIR)
	cp $(PACKAGE_FILES) $(PACKAGE_DIR)
	tar -cvf nabbit_arrays.tar $(PACKAGE_DIR)
	gzip nabbit_arrays.tar
	rm -rf $(PACKAGE_DIR)
