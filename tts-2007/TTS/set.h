#ifndef SET_H
#define SET_H

#include "bag.h"

typedef struct SetStruct
{
	void **elements;
	int capacity, numOfElements;
} Set, *SetPtr;

extern SetPtr newSet();
extern void printSet (SetPtr s);

void addToSet (SetPtr s, void *p);
void addToSortedSet (SetPtr s, void *p);


extern int setPos (SetPtr s, void *p);
extern void copySet (SetPtr s1, SetPtr s2);
extern SetPtr newCopySet (SetPtr s2);
extern SetPtr newCopySetFromBag (BagPtr s2);
extern void unionSet (SetPtr result, SetPtr s1, SetPtr s2);
extern void unionToSet (SetPtr result, SetPtr s1);
extern void differenceToSet (SetPtr result, SetPtr s1);
extern void intersectSet (SetPtr result, SetPtr s1, SetPtr s2);
extern void intersectToSet (SetPtr result, SetPtr s1);

extern int sortedSetCompare (SetPtr a, SetPtr b);

int setsDisjoint (SetPtr s1, SetPtr s2);

extern SetPtr newSingletonSet (void *p);

extern int subsetSet (SetPtr s1, SetPtr s2);
extern int subsetSortedSet (SetPtr s1, SetPtr s2);
extern int equalSortedSet (SetPtr s1, SetPtr s2);
extern int equalSet (SetPtr s1, SetPtr s2);

typedef int (*Comparison)(const void *p1, const void *p2);

extern void sortSet (SetPtr s);
extern void removeDuplicatesSet (SetPtr s);
extern void freeSet (SetPtr s);
extern void clearSet (SetPtr s);

extern void sortSetCompare (SetPtr s, Comparison c);

void addToSetNoCheck (SetPtr s, void *p);

extern int setMemberOfRemove (SetPtr s, void *p);
extern int setMemberOf (SetPtr s, void *p);
extern int setMemberOfRemoveU (SetPtr s, void *p);
extern int setMemberOfU (SetPtr s, void *p);

#endif
