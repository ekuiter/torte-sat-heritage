CC = g++
CFLAGS = -m32 -O3 -fomit-frame-pointer -D NDEBUG -static #-fno-strict-aliasing #-Wall 

EXEC_FILE = rsat

EXEC_FILES = rsat

SOLVER_SRC = cnf_manager.cpp\
	     cnf_read.cpp\
	     sat.cpp\
             unit_resolution.cpp\
	     conflict_analysis.cpp\
	     heap.cpp\
	     mem_alloc.cpp\
	     main.cpp\
	     fcode.cpp\

SOLVER_OBJS=$(SOLVER_SRC:.cpp=.o)

HEADERS = flags.h globals.h types.h heap.h structures.h

rsat: solver.o
	$(CC) $(CFLAGS) $(SOLVER_OBJS) -o $(EXEC_FILE)

main.o: main.cpp
	$(CC) $(CFLAGS) -c main.cpp

solver.o: $(SOLVER_SRC) $(HEADERS)
	$(CC) $(CFLAGS) -c $(SOLVER_SRC) 

all: rsat

clean: 
	rm -f *.o $(EXEC_FILES)
