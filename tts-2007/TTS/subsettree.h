#ifndef SUBSETTREE_H
#define SUBSETTREE_H

#ifdef PROOF
#include "proof.h"
#endif

#include "set.h"

typedef struct SubsetTreeStruct
{
	void * tree;
	struct SubsetTreeStruct *present, *absent;
	//int minSetSize;

#ifdef PROOF
	ProofClausePtr proofClause;
#endif

} SubsetTree, *SubsetTreePtr;

extern void initSubsetTree();

extern void addFalseSetTop (SetPtr set

#ifdef PROOF
		, ProofClausePtr proofClause
#endif
										);

extern void addBFalseSetTop (void * n, SetPtr set );
extern SetPtr findBFalseSubset (void * n, SetPtr s);

extern SetPtr findFalseSubset (SetPtr s

#ifdef PROOF
		, ProofClausePtr *proofClause
#endif
							);

#endif

extern void putBackSubsetTree();
extern SubsetTreePtr newSubsetTree ();
