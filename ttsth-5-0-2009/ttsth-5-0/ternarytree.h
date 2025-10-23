#ifndef TERNARYTREE_H
#define TERNARYTREE_H

#include "subsettree.h"
#include "proposition.h"
#include "list.h"
#include "intset.h"
#include "hashtable.h"


#define LEFT_PARENT 0
#define RIGHT_PARENT 1
#define BOTH_PARENT 2

#ifdef THREADS
#include <pthread.h>
#endif

extern int largestSetSize, setSizeExceeded;
extern SetPtr allTTNodes;

typedef struct SetNodeStruct
{
	unsigned char *bytes;
	struct SetNodeStruct **children;
	int numOfNodes;
} SetNode, *SetNodePtr;


typedef struct SEEntryStruct
{
	int pos;
	struct TernaryTreeStruct *tree;
} SEEntry, *SEEntryPtr;

extern SEEntryPtr newseBuffer();

typedef struct TernaryTreeStruct
{
	int isFalsifiable, isSatisfiable;

	int firstVar, parentVar, isSpine, minParent, maxParent, *actualParent;
	unsigned char *inLeft, *inRight;
	

	int *isSelected;

	struct TernaryTreeStruct *left,*right,*both,*parent;

	//int tries, numOfClauses;


	SubsetTreePtr subsets;
	IntSetPtr allParents;
	//SetPtr descendants;
	//SetPtr subsets1;

	double weight;
	int setCount;
	
	//HashTablePtr subsetElementsTable;
	//SetPtr subsetElements;
	//SEEntryPtr seBuffer;
	//SetNodePtr setNode;

#ifdef THREADS
pthread_rwlock_t nodeLock;
#endif 


#ifdef PROOF
	int id;
	IntSetPtr originalVariables;
	long int resId;
#endif

} TernaryTree, *TernaryTreePtr;

typedef struct SetListStruct
{
	TernaryTreePtr *elements;
	unsigned char *flags;
	int numOfElements, capacity, numOfIds;
} SetList, *SetListPtr;

typedef TernaryTreePtr typekey;

extern void setListSolver();
extern ListPtr setList;

#ifdef TRACE
	extern FILE *treeTrace;
#endif


extern AssignmentPtr getAnswer();

extern void clearAnswerFound();

extern void checkP (int r);

extern void stopSavingFalseSets();

extern TernaryTreePtr addClauseToTernaryTree (TernaryTreePtr oldTree, ClausePtr clause);

extern int iterativeWalk (AssignmentPtr answer, TernaryTreePtr root);

extern int tries, improvements, subsetptrs, unitClauseImprovements;

extern int maxTTNSetSize;

extern List *levelLists, *tempLevelLists;

extern void addDescendants (TernaryTreePtr node);
extern int processDescendants (TernaryTreePtr start);

extern int scanTree (AssignmentPtr answer,TernaryTreePtr root);

extern TernaryTreePtr trueTernaryTree, falseTernaryTree;

typedef void *treeWalker(TernaryTreePtr, int);
extern void traverseTernaryTree (TernaryTreePtr, int, int, treeWalker);


extern TernaryTreePtr newTernaryTree();
extern void initLevelLists (int m);
extern TernaryTreePtr buildTernaryTree (int variable, int isCentre, PropositionPtr p);
extern TernaryTreePtr ternaryTreeFromProposition2 (SetPtr clauses, int trunkVar, int parentVar);
extern void setSpine (TernaryTreePtr root);
extern void createLevelLists2 (int level, TernaryTreePtr ptr);
extern void allocateLevelList2Ids (int maxLevel);
extern void preFill (PropositionPtr p);
extern TernaryTreePtr solveByMerging(TernaryTreePtr root);
extern void addSetUpwards (SetPtr startSet);

extern void setUsed (int level, TernaryTreePtr ptr);
extern void clearUsed();
extern void minimizeTree ();

extern void initTernaryDebug (int numOfVariables);
extern void printTernaryDebug ();

extern void printTernaryInfo(int numOfVariables);
extern TernaryTreePtr  buildAllTernary (SetPtr);

extern int tryTernary (AssignmentPtr assignment,int level, SetListPtr ternaryTrees, SetPtr falseSet);
extern int newTryTernary (int tid, AssignmentPtr assignment, TernaryTreePtr root);

extern int buildAndTryTernary (AssignmentPtr assignment,int level, SetPtr ternaryTrees, SetPtr falseSet, int *falseId, int *resId);
extern void fred (AssignmentPtr assignment,int lastLevel, TernaryTreePtr root, SetPtr falseSet, int *falseId, int *resId);

extern void initTernaryTrees (PropositionPtr p);
extern void reinitTernaryTrees (PropositionPtr p);
extern void freeTreeBuildingResources();

extern void finaliseTernaryTrees();
extern TernaryTreePtr ternaryTreeFromProposition (SetPtr clauses, int parentVar);

extern void saveToNodeSets (TernaryTreePtr,int);

extern void printNodeSets();
extern void buildClashingSets();

extern void fillBottomUp(PropositionPtr p, TernaryTreePtr root);

extern void printStaticDotFile (char *, TernaryTreePtr);

extern TernaryTreePtr assignFutureVar (TernaryTreePtr node, int v, int positive);
#endif
