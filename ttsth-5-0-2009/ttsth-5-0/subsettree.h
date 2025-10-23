#ifndef SUBSETTREE_H
#define SUBSETTREE_H


#include "set.h"

typedef struct SubsetTreeStruct
{
	void * tree;
	struct SubsetTreeStruct *present, *absent;
	//int count, size;
#ifdef PROOF
	int setId;
#endif
	int numAssignments;
	//int minSetSize;


} SubsetTree, *SubsetTreePtr;

#ifdef PROOF
	extern int nextSetId;
#endif

extern void returnSubsetTree (SubsetTreePtr s, int tid);

extern SetPtr allsubsets;
extern SubsetTreePtr endPoint;

extern void initSubsetTree();
extern int buffersAllocated;

extern int addFalseSetTop (int tid, SetPtr set

										);

//extern void addBFalseSetTop (void * n, SetPtr set );
//extern SetPtr findBFalseSubset (void * n, SetPtr s);

extern SetPtr findFalseSubset (SetPtr s, int * id);

#endif

extern void putBackSubsetTree(int tid);
extern SubsetTreePtr newSubsetTree (int tid);
extern void freeAllSubsetTrees();
