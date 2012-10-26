# Makefile, modified for the Cilk++ example Makefiles.


# The directory where the .h to test are located.
DAGEVAL_DIR=../include


CILKPP	= cilk++
LIBARG	=  -O2 -Wall # -lmiser
TARGET = sample
SRC	= $(addsuffix .cilk,$(TARGET))


# The extra include files
INCLUDES = -I $(DAGEVAL_DIR)

H_FILES = sample_dag_node.h

.PHONY: clean

$(TARGET): $(SRC) $(H_FILES) $(O_FILES)
	$(CILKPP) $(SRC) $(INCLUDES) $(LIBARG) -o $@


clean:
	rm -f $(TARGET) 
