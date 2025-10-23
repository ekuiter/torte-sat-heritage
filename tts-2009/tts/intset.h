#ifndef INTSET_H
#define INTSET_H


typedef struct IntIntSetStruct
{
	int *elements;
	int capacity, numOfElements;
} IntSet, *IntSetPtr;

extern IntSetPtr newIntSet();
extern void printIntSet (IntSetPtr s);

void addToIntSet (IntSetPtr s, int p);
void addToSortedIntSet (IntSetPtr s, int p);

void intPush (IntSetPtr s, int p);
int intPop (IntSetPtr s);
int intTop (IntSetPtr s);

extern int intSetPos (IntSetPtr s, int p);
extern void copyIntSet (IntSetPtr s1, IntSetPtr s2);
extern IntSetPtr newCopyIntSet (IntSetPtr s2);
extern void unionToSortedIntSet (IntSetPtr result, IntSetPtr s1);
extern void unionIntSet (IntSetPtr result, IntSetPtr s1, IntSetPtr s2);
extern void unionToIntSet (IntSetPtr result, IntSetPtr s1);
extern void differenceToIntSet (IntSetPtr result, IntSetPtr s1);
extern void intersectIntSet (IntSetPtr result, IntSetPtr s1, IntSetPtr s2);
extern void intersectToIntSet (IntSetPtr result, IntSetPtr s1);

extern int sortedIntSetCompare (IntSetPtr a, IntSetPtr b);

int intSetsDisjoint (IntSetPtr s1, IntSetPtr s2);

extern IntSetPtr newSingletonIntSet (int p);

extern int subsetIntSet (IntSetPtr s1, IntSetPtr s2);
extern int subsetSortedIntSet (IntSetPtr s1, IntSetPtr s2);
extern int equalSortedIntSet (IntSetPtr s1, IntSetPtr s2);
extern int equalIntSet (IntSetPtr s1, IntSetPtr s2);


extern void sortIntSet (IntSetPtr s);
extern void removeDuplicatesIntSet (IntSetPtr s);
extern void freeIntSet (IntSetPtr s);
extern void clearIntSet (IntSetPtr s);


void addToIntSetNoCheck (IntSetPtr s, int p);

extern int intSetMemberOfRemove (IntSetPtr s, int p);
extern int intSetMemberOf (IntSetPtr s, int p);
extern int intSetMemberOfRemoveU (IntSetPtr s, int p);
extern int intSetMemberOfU (IntSetPtr s, int p);

#endif
