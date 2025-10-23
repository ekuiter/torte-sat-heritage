#ifndef TERNARYTREE_H
#define TERNARYTREE_H

#include "subsettree.h"
#include "proposition.h"
#include "list.h"

#ifdef PROOF
#include "proof.h"
#endif

typedef struct TernaryTreeStruct
{
	int isFalsifiable, isSatisfiable, leftSimpler, rightSimpler;
	
	int firstVar, prevFirstVar, numOfVars;
	

	struct TernaryTreeStruct *left,*right,*both;
	
	int tries, numOfClauses;
	

	SubsetTreePtr subsets;
	
	SetPtr clauses /*, posLiterals, negLiterals, allPosLiterals, allNegLiterals */;
	

		
} TernaryTree, *TernaryTreePtr;

typedef struct SetListStruct
{
	TernaryTreePtr *elements;
	unsigned char *flags;
	int numOfElements, capacity, numOfIds;
} SetList, *SetListPtr;

typedef TernaryTreePtr typekey;


#ifdef TRACE
	extern FILE *treeTrace;
#endif

extern int tries, improvements;

extern List *levelLists, *tempLevelLists;


extern TernaryTreePtr trueTernaryTree, falseTernaryTree;

typedef void *treeWalker(TernaryTreePtr, int);
extern void traverseTernaryTree (TernaryTreePtr, int, int, treeWalker);


extern TernaryTreePtr newTernaryTree();
extern void initLevelLists (int m);
extern TernaryTreePtr buildTernaryTree (int variable, int isCentre, PropositionPtr p);
extern void createLevelLists2 (int level, TernaryTreePtr ptr);
extern void allocateLevelList2Ids (int maxLevel);
extern void printStaticDotFile (TernaryTreePtr);
extern void preFill (PropositionPtr p);

extern void setUsed (int level, TernaryTreePtr ptr);
extern void clearUsed();
extern void minimizeTree ();

extern void initTernaryDebug (int numOfVariables);
extern void printTernaryDebug ();

extern void printTernaryInfo(int numOfVariables);
extern TernaryTreePtr  buildAllTernary (SetPtr);

extern int tryTernary (AssignmentPtr assignment,int level, SetListPtr ternaryTrees, SetPtr falseSet

#ifdef PROOF
					, ProofClausePtr *proofClause
#endif
					);

extern int buildAndTryTernary (AssignmentPtr assignment,int level, SetPtr ternaryTrees, SetPtr falseSet

#ifdef PROOF
					, ProofClausePtr *proofClause
#endif
					);

extern void initTernaryTrees (PropositionPtr p);
extern void finaliseTernaryTrees();
extern TernaryTreePtr ternaryTreeFromProposition (SetPtr clauses);

extern void saveToNodeSets (TernaryTreePtr,int);

extern void printNodeSets();
extern void buildClashingSets();



#endif
