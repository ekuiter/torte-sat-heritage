CC = g++ -m32 -Wall -O3 -DNDEBUG
HEADERS = Cnf.h CnfManager.h SatSolver.h
OBJS = Cnf.o CnfManager.o SatSolver.o main.o

tinisat: $(OBJS) 
	$(CC) $(OBJS) -static -o tinisat  

$(OBJS): $(HEADERS) Makefile

.cpp.o: 
	$(CC) -c $< 
