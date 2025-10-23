#include <stdio.h>
#include <stdlib.h>

#ifdef THREADS
#include <pthread.h>
#endif

#ifdef VISUALSTUDIO
#else
#include <sys/unistd.h>
#endif

#include "memutil.h"
#include "ternarytree.h"
#include "list.h"
#include "proposition.h"
#include "tts.h"
#include "set.h"
#include "intset.h"
#include "options.h"
#include "hashtable.h"
#include "cputime.h"
#include "dimacs.h"



#ifdef PROOF
#include "proof.h"
#endif

SetPtr allTTNodes;

SEEntryPtr newseBuffer()
{
	SEEntryPtr result;
	result = (SEEntryPtr) checkMalloc (sizeof(SEEntry),"");
	return result;
}

unsigned long seHashFunction (void * t)
{
	return    (unsigned long) ((SEEntryPtr) t)->tree;
}
int seHashKeyEqual (void * t1, void * t2)
{
	return  ((SEEntryPtr) t1)->tree == ((SEEntryPtr) t2)->tree;
}
void seHashKeyFree (void * t1)
{
}
void seHashKeyPrint (void * t)
{
}
int seHashKeyActive (void * t)
{
	return 1;
}



TernaryTreePtr newTernaryTree()
{
	TernaryTreePtr result;
	int t;
	
	result = checkMalloc (sizeof(TernaryTree),"TernaryTree");
	result->isFalsifiable = true;
	result->isSatisfiable = true;
	result->left = NULL;
	result->both = NULL;
	result->right = NULL;
	result->parent = NULL;
	result->subsets = NULL;
	//result->subsets1 = newSet();
	result->firstVar = 1<<30;
	result->parentVar = 0;
	result->minParent = 1<<30;
	result->allParents = newIntSet();
	result->maxParent = 0;
	result->isSelected = (int *) checkMalloc (numThreads*sizeof(int),"");
	result->inLeft = (unsigned char *) checkMalloc (numThreads*sizeof(unsigned char),"");
	result->inRight = (unsigned char *) checkMalloc (numThreads*sizeof(unsigned char),"");
	result->actualParent = (int *) checkMalloc (numThreads*sizeof(int),"");
	for (t=0;t<numThreads;t++)
		result->isSelected[t] = false;
	result->isSpine = false;
	//result->descendants = newSet();
	result->setCount = 0;
	
	//result->setNode = newSetNode();
	//result->seBuffer = newseBuffer();
	//result->subsetElements = newSet();
   // result->subsetElementsTable = newHashTable (&seHashFunction, &seHashKeyEqual, &seHashKeyFree, &seHashKeyPrint, &seHashKeyActive);

#ifdef PROOF
	result->id = -1;
#endif

#ifdef THREADS
		checkP(pthread_rwlock_init(&result->nodeLock, NULL));
#endif

//    result->ps = malloc(2000);

	addToSetNoCheck (allTTNodes, result);
	
	return result;
}

#ifdef PROOF
TernaryTreePtr newFalseTernaryTree (IntSetPtr originalVariables, long int resId)
{
	TernaryTreePtr result;

	
	result = newTernaryTree();
	result->isSatisfiable = false;
	result->left = result;
	result->right = result;
	result->both = result;
	result->id = 0;

	//result->originalVariables = originalVariables;
	result->resId = resId;

	return result;
}
#endif

char stringClauseList[50000];






unsigned long clauseHashFunction (ClausePtr clause)
{
	unsigned long result = 0;
	unsigned long i;

	for (i=0;i<clause->numOfVariables;i++)
		if (clause->positive[i])
			result += (2*i+2)*clause->variables[i];
		else
			result += (2*i+1)*clause->variables[i];

	return result;
}

unsigned long clausesHashFunction (SetPtr clauses)
{
	unsigned long result = 0;
	unsigned long i;
	for (i=0;i<clauses->numOfElements;i++)
		result += (i+2)*clauseHashFunction (clauses->elements[i]);
	return result;
}

unsigned long childrenHashFunction (TernaryTreePtr node)
{
	unsigned long result = 0;
	result = node->firstVar*2 + ((unsigned long )node->left)*3 + ((unsigned long )node->right)*5 + ((unsigned long )node->both)*7;
	return result;
}
unsigned long ttHashFunction (void * t)
{
	return    /*(((TernaryTreePtr)t)->parentVar)+ */  childrenHashFunction (t);
}

int ttChildrenHashKeyEqual (TernaryTreePtr t1, TernaryTreePtr t2)
{
	return (t1->firstVar == t2->firstVar) && (t1->left == t2->left) && (t1->both == t2->both) && (t1->right == t2->right);
}
int ttHashKeyEqual (void * t1, void * t2)
{
	return  ttChildrenHashKeyEqual (t1, t2);
}
void ttHashKeyFree (void * t1)
{
}
void ttHashKeyPrint (void * t)
{
}
int ttHashKeyActive (void * t)
{
	return 1;
}

HashTablePtr ttHashTable;
TernaryTreePtr ttHashEntry;

List *levelLists, *tempLevelLists, allNodes;


static int maxVariable;

#ifdef TRACE
	FILE *treeTrace;
#endif

static int *endedClauses;
static int *newFalses, *oldFalses, maxV, **setSizes;
static int *levelCounts, *visits, *levelImprovements, debugNumVariables;
static ListPtr *levelSets;
static SetPtr *levelNodeSets;

static double *totalVisitSize, *totalFalseSize;

TernaryTreePtr trueTernaryTree, falseTernaryTree;

static SetPtr tryCopySet, **nextFalseSet, **nextLeft, **nextRight;
static SetPtr  **leftSet, **rightSet, **middleSet, **allSet;

SetPtr * newSetArray (int numOfSets)
{
	SetPtr *result;
	int i;

	result = (SetPtr *) checkMalloc (numOfSets*sizeof(SetPtr),"");
	for (i=0;i<numOfSets;i++)
		result[i] = newSet();


	return result;
}


TernaryTreePtr combineTernaryTrees (TernaryTreePtr a, TernaryTreePtr b)
{
	TernaryTreePtr result;

	if (a == falseTernaryTree || b == falseTernaryTree)
		result = falseTernaryTree;
	else if (a == trueTernaryTree)
		result = b;
	else if (b == trueTernaryTree)
		result = a;
	else
	{
		result = newTernaryTree();
		result->left = combineTernaryTrees (a->left, b->left);
		result->both = combineTernaryTrees (a->both, b->both);
		result->right = combineTernaryTrees (a->right, b->right);
	}
	return result;
}

TernaryTreePtr topTree;

TernaryTreePtr buildTernaryTree (int variable, int isCentre, PropositionPtr p)
{
	TernaryTreePtr result;
	PropositionPtr leftP, rightP, bothP;




	result = newTernaryTree();
//    strcpy (result->ps, "");

	if (p == trueProposition)
	{
		result = trueTernaryTree;
	}
	else if (p == falseProposition)
	{
		result = falseTernaryTree;

	}
	else
	{
		leftP = propositionCurried(p,variable,true);
		result->left = buildTernaryTree (variable+1, false, leftP);
		bothP = propositionExcluding(p,variable);
		result->both = buildTernaryTree (variable+1, isCentre, bothP);
		rightP = propositionCurried(p,variable,false);
		result->right = buildTernaryTree (variable+1, false, rightP);

	}

	//addListLast (&levelLists[variable-1], result);

	return result;
}


int equalPtrs (TernaryTreePtr p1, TernaryTreePtr p2)
{
	int result;
	result = false;
	if (p1 == NULL && p2 == NULL) result = true;
	else if (p1 == NULL && !p2->isFalsifiable) result = true;
	else if (p2 == NULL && !p1->isFalsifiable) result = true;
	else if (p1 != NULL && !p1->isSatisfiable && p2 != NULL && !p2->isSatisfiable) result = true;
	else if (p1 != NULL && !p1->isFalsifiable && p2 != NULL && !p2->isFalsifiable) result = true;
	else if (p1 == p2) result = true;

	return result;
}

int equalNodes (TernaryTreePtr p1, TernaryTreePtr p2)
{
	return equalPtrs (p1, p2) ||
		( (p1 != NULL && p1->isSatisfiable && p1->isFalsifiable) &&
		(p2 != NULL && p2->isSatisfiable && p2->isFalsifiable) &&
		equalPtrs (p1->left, p2->left) &&
		equalPtrs (p1->both, p2->both) &&
		equalPtrs (p1->right, p2->right) );
}


void addIfNotPresent (ListPtr list, TernaryTreePtr tree)
{
	LinkPtr link;
	link = list->first;
	while (link != NULL && link->element != tree)
		link = link->next;

	if (link == NULL)
	{
		addListLast (list, tree);
	}
}

int max3 (int a, int b, int c)
{
	int max;
	max = a>b?a:b;
	max = max>c?max:c;
	return max;
}



void printTernaryInfo(int numOfVariables)
{
	int l;
	if (booleanOptions[PRINT_LEVEL_COUNTS])
	{
		for (l=0;l<=numOfVariables;l++)
			printf ("L=%d, visits=%d average visit=%.2f average false=%.2f\n", l, visits[l],
				visits[l]==0?0.0:(totalVisitSize[l]/visits[l]),visits[l]==0?0.0:(totalFalseSize[l]/visits[l]));
	}
}


int ll2id;




int tries, improvements, subsetptrs;

SetPtr leftCopy, rightCopy;

void addCoverage (int direction, SetPtr falseSet, SetPtr test, SetPtr cover)
{
	int l;
	TernaryTreePtr node;

	for (l=0;l<test->numOfElements;l++)
	{
		node = test->elements[l];
		if ( (direction&&setMemberOf (falseSet, node->left)) ||
				setMemberOf (falseSet, node->both) ||
				(!direction && setMemberOf (falseSet, node->right)))
				{
				addToSet (cover, node);
				}
	}
}

void findMiddles (SetPtr trees, SetPtr middles)
{
	int l;
	TernaryTreePtr node;

	for (l=0;l<trees->numOfElements;l++)
	{
		node = trees->elements[l];
		addToSet (middles, node->both);
	}
	sortSet (middles);
}



int addTwinCoverage (int level, SetPtr leftFalse, SetPtr rightFalse, SetPtr test, SetPtr cover)
{
	int l;
	TernaryTreePtr node;



	//qsort (test->elements, test->numOfElements, sizeof (TernaryTreePtr), prevFirstVarCompare);

	//printNodeSetClauses (test);
	//printf ("\n");


	for (l=0;l<test->numOfElements;l++)
	{
		node = test->elements[l];
		if (node->firstVar > level+1)
		{
			if (setMemberOf (leftFalse, node) || setMemberOf (rightFalse, node))
			{
				addToSetNoCheck (cover, node);
				setMemberOfRemove (leftFalse, node);
				setMemberOfRemove (rightFalse, node);
			}
		}
		else
		{
			if (setMemberOf (leftFalse, node->both) || setMemberOf (rightFalse, node->both))
			{
				addToSetNoCheck (cover, node);
				setMemberOfRemove (leftFalse, node->both);
				setMemberOfRemove (leftFalse, node->left);
				setMemberOfRemove (rightFalse, node->both);
				setMemberOfRemove (rightFalse, node->right);
			}
		}
	}

	for (l=0;l<test->numOfElements;l++)
	{
		node = test->elements[l];
		if (node->firstVar > level+1)
		{
		}
		else
		{
			if (setMemberOf (rightFalse, node->right)|| setMemberOf (leftFalse, node->left))
			{
				addToSetNoCheck (cover, node);
				setMemberOfRemove (leftFalse, node->left);
				setMemberOfRemove (rightFalse, node->right);
			}
		}
	}

	removeDuplicatesSet (cover);

	return (leftFalse->numOfElements == 0 && rightFalse->numOfElements == 0);
}


void addBothCoverage (int level, SetPtr falseSet, SetPtr test, SetPtr cover)
{
	int l;
	TernaryTreePtr node;


	clearSet (leftCopy);
	clearSet (rightCopy);

	for (l=0;l<test->numOfElements;l++)
	{
		node = test->elements[l];
		if (node->firstVar > level+1)
		{
			if (setMemberOf (falseSet, node))
			{
				addToSetNoCheck (cover, node);
				setMemberOfRemove (falseSet, node);
			}
		}
		else
		{
			if (setMemberOf (falseSet, node->both))
			{
				addToSetNoCheck (cover, node);
				setMemberOfRemove (falseSet, node->both);

			}
		}
	}

	if (falseSet->numOfElements > 0)
	{
		copySet (leftCopy, falseSet);
		copySet (rightCopy, falseSet);

		for (l=0;l<test->numOfElements;l++)
		{
			node = test->elements[l];
			if (node->firstVar > level+1)
			{
			}
			else
			{
				if (setMemberOf (leftCopy, node->left) || setMemberOf (rightCopy, node->right))
				{
					addToSetNoCheck (cover, node);
					setMemberOfRemove (leftCopy, node->left);
					setMemberOfRemove (rightCopy, node->right);
				}
			}
		}
	}

	removeDuplicatesSet (cover);

//printf ("Add both %d ", level); printSet (cover);  printf ("\n");
}



int setContainsFalse (SetPtr s)
{
	int result = false;
	int i;
	for (i=0;!result && i < s->numOfElements; i++)
		result = !((TernaryTreePtr)s->elements[i])->isSatisfiable;
	return result;
}



void fillNextTernarySets (int level, SetPtr start, SetPtr leftSet, SetPtr rightSet, SetPtr middleSet, int *leftSimpler,
						   int *rightSimpler, SetPtr posLiterals, SetPtr negLiterals)
{
	int l;
	TernaryTreePtr node;

	*leftSimpler = false;
	*rightSimpler = false;


	for (l=0;l<start->numOfElements;l++)
	{
		node = start->elements[l];



		if (node->firstVar > level+1)
		{
			addToSetNoCheck (leftSet,node);
			addToSetNoCheck (rightSet,node);
			addToSetNoCheck (middleSet, node);
		}
		else
		{
			if (node->both->isFalsifiable)
			{
				addToSetNoCheck (leftSet,node->both);
				addToSetNoCheck (rightSet,node->both);
				addToSetNoCheck (middleSet, node->both);
			}
		}
	}

	for (l=0;l<start->numOfElements;l++)
	{
		node = start->elements[l];
		if (node->firstVar > level+1)
		{
		}
		else
		{
			if (node->firstVar != level+1)
			{
			}
			if (node->left->isFalsifiable)
			{
				addToSetNoCheck (leftSet,node->left);
			}
			if (node->right->isFalsifiable)
			{
				addToSetNoCheck (rightSet,node->right);
			}
		}
	}

#ifdef PROOF
	if (!booleanOptions[RPT_FILE])
	{
		removeDuplicatesSet (leftSet);
		removeDuplicatesSet (rightSet);
		removeDuplicatesSet (middleSet);
	}
#else
	removeDuplicatesSet (leftSet);
	removeDuplicatesSet (rightSet);
	removeDuplicatesSet (middleSet);
#endif

	//printf ("%d %d %d\n", middleSet->numOfElements, leftSet->numOfElements - middleSet->numOfElements, rightSet->numOfElements-middleSet->numOfElements);


}

FILE *dotFile, *staticDotFile;

void initTernaryDebug (int numOfVariables)
{
	int v,s;
	maxV = numOfVariables;

	newFalses = (int *) checkMalloc ((numOfVariables+1)*sizeof(int), "newf");
	oldFalses = (int *) checkMalloc ((numOfVariables+1)*sizeof(int), "oldf");
	levelCounts = (int *)checkMalloc ( (numOfVariables+1)*sizeof(int), "oldf");
	endedClauses = (int *)checkMalloc ( (numOfVariables+1)*sizeof(int), "oldf");
	levelImprovements = (int *)checkMalloc ( (numOfVariables+1)*sizeof(int), "oldf");
	levelSets = (ListPtr *)checkMalloc ( (numOfVariables+1)*sizeof(int), "oldf");
	setSizes = (int **) twoDimArray (1+numOfVariables, 20);
	for (v=0;v<=numOfVariables;v++)
	{
		for (s=0;s<20;s++)
			setSizes[v][s] = 0;
		newFalses[v] = 0;
		visits[v] = 0;
		endedClauses[v] = 0;
		oldFalses[v] = 0;
		levelCounts[v] = 0;
		levelSets[v] = newList();
	}
	debugNumVariables = numOfVariables;
}

void printTernaryNode (TernaryTreePtr t)
{

}

#ifdef TRACE

void printTreeTrace(int numOfVariables)
{
	LinkPtr l;
	int id,v;
	TernaryTreePtr tree;
	SetPtr s;

	fprintf (treeTrace,"%d\n", numOfVariables);

	for (v=0;v<=numOfVariables;v++)
	{
		id = 0;
		for (l=levelLists[v].first;l!=NULL;l=l->next)
		{
			tree = l->element;
			if (tree == tree->minimized)
			{
				tree->id = -2;
			}
		}
	}

	trueTernaryTree->id =0;
	falseTernaryTree->id = 1;

	tree = levelLists[0].first->element;
	if (tree->id == -2)
		tree->id = -1;

	for (v=0;v<=numOfVariables;v++)
	{
		id = 0;
		for (l=levelLists[v].first;l!=NULL;l=l->next)
		{
			tree = l->element;
			if (tree->id == -1 && tree != trueTernaryTree && tree != falseTernaryTree)
			{
				tree->left->id = -1;
				tree->both->id = -1;
				tree->right->id = -1;
			}
		}
	}

	trueTernaryTree->id =0;
	falseTernaryTree->id = 1;

	for (v=0;v<=numOfVariables;v++)
	{
		id = 3;
		for (l=levelLists[v].first;l!=NULL;l=l->next)
		{
			tree = l->element;
			if (tree->id == -1)
			{
				tree->id = id++;
			}
		}
	}

	for (v=0;v<=debugNumVariables;v++)
	{
		for (l=levelLists[v].first;l!=NULL;l=l->next)
		{
			tree = l->element;
			if (tree->id >= 0)
			{
				if (tree == trueTernaryTree)
					fprintf (treeTrace,"%d %d %d %d ", tree->id, tree->id, tree->id, tree->id);
				else if (tree == falseTernaryTree)
					fprintf (treeTrace,"%d %d %d %d ", tree->id, trueTernaryTree->id, tree->id, trueTernaryTree->id);
				else
					fprintf (treeTrace,"%d %d %d %d ", tree->id, tree->left->id, tree->both->id, tree->right->id);
			}
		}
		fprintf (treeTrace,"-1\n");
	}

}

#endif

void printTernaryDebug ()
{
	int v,v1;


	for (v=0;v<=debugNumVariables;v++)
	{
		//printf ("Level %d: old = %d, new = %d\n", v, oldFalses[v], newFalses[v]);
		//printf ("Level %d: searches = %d, improvements = %d\n", v, levelCounts[v], levelImprovements[v]);
		printf ("Level %4d: ",v);

/*
		for (l=levelSets[v]->first;l!=NULL;l=l->next)
		{
			printf ("{ ");
			s = (SetPtr) l->element;
			for (v1=0;v1<s->numOfElements;v1++)
				printf ("%s ",((TernaryTreePtr)s->elements[v1])->ps);
			printf ("}\n");

		}
		printf ("\n");
*/
		printf ("%4d %4d %5d %4d |",visits[v], levelCounts[v], newFalses[v], endedClauses[v]);

		for (v1=0;v1<20;v1++)
			printf ("%4d ",setSizes[v][v1]);

		printf("\n");
	}

/*
	for (v=0;v<=debugNumVariables;v++)
	{
		printf ("Level %4d: ",v);


		for (l=levelLists[v].first;l!=NULL;l=l->next)
		{
			t = l->element;
			if (t == t->minimized)
			{
				printTernaryNode (t);
				printf (" <%s>  ", t->ps);
			}
		}
		printf ("\n");

		printf("\n");
	}
*/



}

void printTraceFile ()
{
	int v;

	FILE *f=fopen("trace.dat","w");

	for (v=0;v<=debugNumVariables;v++)
	{
		fprintf (f,"%4d ",v);


		fprintf (f,"%4d %4d %5d %4d ",visits[v], levelCounts[v], newFalses[v], endedClauses[v]);
		fprintf(f,"\n");
	}

	fclose (f);

}




int maxUnitLength;
SetPtr mainStem;
int mainStemClauses;
IntSetPtr *posVarClauseSets, *negVarClauseSets;
TernaryTreePtr *posUnitClauseTrees, *negUnitClauseTrees;
int *levelListNextId;

int unitLength (ClausePtr clause)
{
	int maxV, max2V,v;

	maxV = clause->variables[0];
	max2V = clause->variables[0];

	for (v=1;v<clause->numOfVariables;v++)
		if (clause->variables[v] > maxV)
		{
			max2V = maxV;
			maxV = clause->variables[v];
		}


	return 1+maxV-max2V;
}



int setOrder (SetPtr s1, SetPtr s2)
{
	int minLength,i;

	sortSet (s1);
	sortSet (s2);

	minLength = s1->numOfElements>s2->numOfElements?s2->numOfElements:s1->numOfElements;

	for (i=0;i<minLength && s1->elements[i]==s2->elements[i];i++);
	if (i == s1->numOfElements && i == s2->numOfElements)
		return 0;
	else if (i == s1->numOfElements)
		return -1;
	else if (i == s2->numOfElements)
		return 1;
	else if (s1->elements[i] < s2->elements[i])
		return -1;
	else
		return 1;
}





int ternaryTreeComparatorNew (const void * pt1, const void * pt2)
{
	TernaryTreePtr t1,t2;

	t1 = *((TernaryTreePtr *)pt1);
	t2 = *((TernaryTreePtr *)pt2);

	if (t1 < t2) return -1;
	if (t1 > t2) return 1;
	return 0;

	if (t1->firstVar > t2->firstVar)
		return -1;
	else if (t1->firstVar < t2->firstVar)
		return 1;
	else if (t1 > t2)
		return -1;
	else if (t1 < t2)
		return 1;
	else
		return 0;

}

int ternaryTreeComparatorFirstVar (const void * pt1, const void * pt2)
{
	TernaryTreePtr t1,t2;

	t1 = *((TernaryTreePtr *)pt1);
	t2 = *((TernaryTreePtr *)pt2);


	if (t1->firstVar > t2->firstVar)
		return -1;
	else if (t1->firstVar < t2->firstVar)
		return 1;
	else if (t1 > t2)
		return -1;
	else if (t1 < t2)
		return 1;
	else
		return 0;

}
int ternaryTreeComparatorNew1 (const void * pt1, const void * pt2)
{
	TernaryTreePtr t1,t2;

	t1 = *((TernaryTreePtr *)pt1);
	t2 = *((TernaryTreePtr *)pt2);

	if (t1->parentVar > t2->parentVar)
		return 1;
	else if (t1->parentVar < t2->parentVar)
		return -1;
	else if (t1 > t2)
		return 1;
	else if (t1 < t2)
		return -1;
	else
		return 0;

}


TernaryTreePtr *FTT;


typedef struct UnitClauseListStruct
{
	int v;
	SetPtr clauses;
} UnitClauseList, *UnitClauseListPtr;

UnitClauseListPtr newUnitClauseList(int v)
{
	UnitClauseListPtr result;

	result = (UnitClauseListPtr) checkMalloc (sizeof (UnitClauseList),"");
	result->v = v;
	result->clauses = newSet();
	return result;
}

void freeUnitClauseList (UnitClauseListPtr u)
{
	freeSet (u->clauses);
	free (u);
}

int unitClauseListCompare (void * u1, void * u2)
{
	int v1,v2;

	v1 = ((UnitClauseListPtr)u1)->v;
	v2 = ((UnitClauseListPtr)u2)->v;

	if (v1 < v2)
		return -1;
	else if (v1 > v2)
		return 1;
	else
		return 0;
}



SetPtr prioritySet;
int *lastPosLit, *lastNegLit;
SetPtr posLiterals, negLiterals;
int *posVarClauseCounts, *negVarClauseCounts;

SetPtr **negTrees, **posTrees;


void printClauseSet (SetPtr s)
{
	int i;

	if (s == NULL)
		printf ("null");
	else
	{
		for (i=0;i<s->numOfElements;i++)
			printf ("%s ", toStringClause(s->elements[i]));
	}
}


void fprintClauseSet (FILE *f, SetPtr s)
{
	int i;

	if (s == NULL)
		fprintf (f, "null");
	else
	{
		for (i=0;i<s->numOfElements;i++)
			fprintf (f, "%s ", toStringClause(s->elements[i]));
	}
}

void fprintNodeSet (FILE *f, SetPtr s)
{
	TernaryTreePtr node;
	int l;
	for (l=0;l<s->numOfElements;l++)
	{
		node = s->elements[l];
		//fprintf (f,"<%d>", node);
	}
}

int previousTrunkFirstVar;

SetPtr tempCopy;
SetPtr sortedFirstNext, sortedSecondNext, sortedMiddles;

static int logLimit = 100,logHeaderPrinted=0;
static double logCPULimit =0;
static char logMessageBuffer[200];
static char cpuprogress[11],paprogress[33];
static int paLevelCounts[7];

static TernaryTreePtr *spine;

#ifdef THREADS
static pthread_rwlock_t *levelLocks;
#endif 

int *sign;

void initTernaryTrees (PropositionPtr p)
{
	int v,v1,v3,t;
	int c;
	int maxLevel;
	int minPosVar, minNegVar;

	ClausePtr clause;

	allTTNodes = newSet();
	 
	if (stringOptions[DOT_FILE] != NULL)
	{
		dotFile = fopen (stringOptions[DOT_FILE],"w");
		fprintf (dotFile, "graph G {\nsize=\"11,20\";");
	}

	leftCopy = newSet();
	rightCopy = newSet();

	previousTrunkFirstVar = 1;

	maxLevel = p->maxVariable;
	maxVariable = maxLevel;

#ifdef PROOF
	if (booleanOptions[RPT_FILE])
	{
		sortedFirstNext = newSet();
		sortedSecondNext = newSet();
		sortedMiddles = newSet();
	}
#endif

	ttHashTable = newHashTable (&ttHashFunction, &ttHashKeyEqual, &ttHashKeyFree, &ttHashKeyPrint, &ttHashKeyActive);
	ttHashEntry = newTernaryTree();


	//levelNodeSets = (SetPtr *)checkMalloc ((1+maxLevel)*sizeof (SetPtr),"");
	//levelDependencies = (SetPtr *)checkMalloc ((1+maxLevel)*sizeof (SetPtr),"");
	//posLevelDependencies = (SetPtr *)checkMalloc ((1+maxLevel)*sizeof (SetPtr),"");
	//negLevelDependencies = (SetPtr *)checkMalloc ((1+maxLevel)*sizeof (SetPtr),"");

	//lastPosLit = (int *) checkMalloc ( (1+maxVariable)*sizeof (int),"");
	//lastNegLit = (int *) checkMalloc ( (1+maxVariable)*sizeof (int),"");
	posLiterals = newSet();
	negLiterals = newSet();

	prioritySet = newSet();

#ifdef THREADS
	levelLocks = (pthread_rwlock_t *) checkMalloc ((1+maxVariable)*sizeof(pthread_rwlock_t),"");
	for (v=0;v<=maxVariable;v++)
	{
		checkP(pthread_rwlock_init(&levelLocks[v], NULL));
	}
#endif

	//levelListNextId = (int *) checkMalloc ( (1+maxLevel)*sizeof (int *), "ids");
	leftSet = (SetPtr **) checkMalloc ( numThreads*sizeof (SetPtr *), "");
	rightSet = (SetPtr **) checkMalloc ( numThreads*sizeof (SetPtr *), "");
	middleSet = (SetPtr **) checkMalloc ( numThreads*sizeof (SetPtr *), "");
	allSet = (SetPtr **) checkMalloc ( numThreads*sizeof (SetPtr *), "");
	nextFalseSet = (SetPtr **) checkMalloc ( numThreads*sizeof (SetPtr *), "");
	nextLeft = (SetPtr **) checkMalloc ( numThreads*sizeof (SetPtr *), "");
	nextRight = (SetPtr **) checkMalloc ( numThreads*sizeof (SetPtr *), "");

	posTrees = (SetPtr **) checkMalloc ( numThreads*sizeof (SetPtr *), "");
	negTrees = (SetPtr **) checkMalloc ( numThreads*sizeof (SetPtr *), "");
	
		
	for (t=0;t<numThreads;t++)
	{
		posTrees[t] = (SetPtr *) checkMalloc ((1+maxVariable)*sizeof(SetPtr),"");
		negTrees[t] = (SetPtr *) checkMalloc ((1+maxVariable)*sizeof(SetPtr),"");
		leftSet[t] = (SetPtr *) checkMalloc ((1+maxVariable)*sizeof(SetPtr),"");
		rightSet[t] = (SetPtr *) checkMalloc ((1+maxVariable)*sizeof(SetPtr),"");
		middleSet[t] = (SetPtr *) checkMalloc ((1+maxVariable)*sizeof(SetPtr),"");
		allSet[t] = (SetPtr *) checkMalloc ((1+maxVariable)*sizeof(SetPtr),"");
		nextFalseSet[t] = (SetPtr *) checkMalloc ((1+maxVariable)*sizeof(SetPtr),"");
		nextLeft[t] = (SetPtr *) checkMalloc ((1+maxVariable)*sizeof(SetPtr),"");
		nextRight[t] = (SetPtr *) checkMalloc ((1+maxVariable)*sizeof(SetPtr),"");
		for (v=0;v<=maxVariable;v++)
		{
			posTrees[t][v] = newSet();
			negTrees[t][v] = newSet();
			leftSet[t][v] = newSet();
			rightSet[t][v] = newSet();
			middleSet[t][v] = newSet();
			allSet[t][v] = newSet();
			nextFalseSet[t][v] = newSet();
			nextLeft[t][v] = newSet();
			nextRight[t][v] = newSet();
		}
	}

	sign = (int *) checkMalloc ( (1+maxVariable) *sizeof(int),"sign");
	for (v=1;v<=maxVariable;v++)
		sign[v] = 1;
		

	spine = (TernaryTreePtr *) checkMalloc ((1+maxVariable)*sizeof(TernaryTreePtr),"");

	for (v=0;v<=maxVariable;v++)
	{
		spine[v] = NULL;
	}

	trueTernaryTree = newTernaryTree();
	trueTernaryTree->left = trueTernaryTree;
	trueTernaryTree->both = trueTernaryTree;
	trueTernaryTree->right = trueTernaryTree;
	trueTernaryTree->isFalsifiable = false;
	falseTernaryTree = newTernaryTree();
	falseTernaryTree->isSatisfiable = false;
	falseTernaryTree->left = falseTernaryTree;
	falseTernaryTree->both = falseTernaryTree;
	falseTernaryTree->right = falseTernaryTree;

#ifdef PROOF
	falseTernaryTree->id = 0;
	trueTernaryTree ->id = 1;
#endif


	tryCopySet = newSet();

	mainStem = newSet();

	for (c=0;c<p->numOfClauses;c++)
	{
		addToSetNoCheck (mainStem, (void *) p->clauses[c]);
	}
	mainStemClauses = mainStem->numOfElements;


	posVarClauseSets = (IntSetPtr *) checkMalloc ( (1+p->maxVariable)*sizeof(IntSetPtr),"sets");
	negVarClauseSets = (IntSetPtr *) checkMalloc ( (1+p->maxVariable)*sizeof(IntSetPtr),"sets");

	posVarClauseCounts = (int *) checkMalloc ( (1+p->maxVariable)*sizeof(int),"sets");
	negVarClauseCounts = (int *) checkMalloc ( (1+p->maxVariable)*sizeof(int),"sets");


	for (v=1; v <= p->maxVariable; v++)
	{
		posVarClauseSets[v] = newIntSet();
		negVarClauseSets[v] = newIntSet();
		posVarClauseCounts[v] = 0;
		negVarClauseCounts[v] = 0;
	}

	for (c=0;c<p->numOfClauses;c++)
	{
		clause = p->clauses[c];
		v3 = minVariable(clause);
		minPosVar = p->maxVariable+1;
		minNegVar = p->maxVariable+1;
		for (v=0;v<clause->numOfVariables;v++)
		{
			v1 = clause->variables[v];
			if (clause->positive[v])
			{
				addToIntSetNoCheck (posVarClauseSets[v1], c);
				if (v1<minPosVar)
					minPosVar = v1;
			}
			else
			{
				addToIntSetNoCheck (negVarClauseSets[v1], c);
				if (v1<minNegVar)
					minNegVar = v1;
			}
/*
			if (clause->positive[v])
			{
				if (lastPosLit[v1]<v3-1)
					lastPosLit[v1] = v3-1;
			}
			else
			{
				if (lastNegLit[v1]<v3-1)
					lastNegLit[v1] = v3-1;
			}
*/
		}
		if (minNegVar <= p->maxVariable)
			negVarClauseCounts[minNegVar]++;
		if (minPosVar <= p->maxVariable)
			posVarClauseCounts[minPosVar]++;
	}

	logLimit = 100;
	logHeaderPrinted=0;
	logCPULimit =0;

	for (v=0;v<32;v++)
		paprogress[v] = '.';
	paprogress [32] = '\0';

	for (v=0;v<6;v++)
		paLevelCounts[v] = 0;



}




void finaliseTernaryTrees()
{
	if (stringOptions[DOT_FILE] != NULL)
	{
		fprintf (dotFile, "}\n");
		fclose (dotFile);
	}
}

int setContainsEmptyClause (SetPtr clauses

#ifdef PROOF
							,IntSetPtr *originalVariables, long int *resId
#endif
															)
{
	ClausePtr clause;
	int c, found;

	found = false;
	for (c=0;!found && c<clauses->numOfElements;c++)
	{
		clause = clauses->elements[c];
		found = clause->numOfVariables == 0;

#ifdef PROOF
		if (booleanOptions[RPT_FILE])
		{
			//*originalVariables = clause->originalVariables;
			*resId = clause->resId;
			//copyIntSet (idVars[*resId], clause->originalVariables);
		}
#endif

	}
	return found;
}





SetPtr negativeUnitClauses (SetPtr clauses)
{
	SetPtr result;
	ClausePtr c1,c2;

	result = newSet();
	c1 = (ClausePtr) (clauses->elements[0]);
	c2 = newClause();
	addClauseVariable (c2, c1->positive[0]?-c1->variables[0]:c1->variables[0]);

	addToSet (result, c2);

	return result;
}
void pClause (ClausePtr c)
{
	int i;
	printf ("(");
	if (c==NULL)
		printf ("null clause)");
	else
	{
	for (i=0;i<c->numOfVariables;i++)
		printf ("%d%s",c->positive[i]?c->variables[i]:-c->variables[i],i==c->numOfVariables-1?")":",");
	if (c->numOfVariables==0) printf (")");
	}
}

void pClauseSet (SetPtr s)
{
	int i;
	if (s == NULL)
		printf ("main\n");
	else
	{
		for (i=0;i<s->numOfElements;i++)
			pClause (s->elements[i]);
		printf("\n");
	}
}

int toBeMerged (SetPtr clauses)
{
	ClausePtr clause;
	int i;
	int result=true;

	if (clauses->numOfElements > 4)
		return false;

	for (i=0;result && i<clauses->numOfElements;i++)
	{
		clause = (ClausePtr) clauses->elements[i];
		if (clause->numOfVariables > 1)
			result = false;

	}
	return result;
}

int allSameClauses (SetPtr clauses)
{
	int result = true;
	ClausePtr clause;
	int variable=1, positive=0,i;

	clause = clauses->elements[0];
	if (clause->numOfVariables == 1)
	{
		variable = clause->variables[0];
		positive = clause->positive[0];
	}
	else
		result = false;

	for (i=1;result && i<clauses->numOfElements;i++)
	{
		clause = clauses->elements[i];
		result = (clause->numOfVariables == 1 && clause->variables[0] == variable &&
					clause->positive[0] == positive);
	}
	return result;
}
SetPtr thesePosLiterals = NULL;
SetPtr theseNegLiterals = NULL;

int clauseSetFirstVar (SetPtr clauses)
{
	int i,result;
	ClausePtr c;
	result = maxVariable;

	for (i=0;i<clauses->numOfElements;i++)
	{
		c = (ClausePtr) clauses->elements[i];
		if (c->variables[0] < result)
			result = c->variables[0];
	}

	return result;
}


void saveDependencies (TernaryTreePtr node, int level)
{
	int l;
	if (node->isSatisfiable && node->isFalsifiable)
	{
		for (l=level;l<node->firstVar;l++)
		{
		}
	}
}

void saveToNodeSets (TernaryTreePtr node, int level)
{
	int l;
	if (node->isSatisfiable && node->isFalsifiable)
	{
		for (l=level;l<node->firstVar;l++)
		{
			addToSet (levelNodeSets[l], node);
		}
	}
}

void printNodeSets()
{
	int v;
	for (v=0;v<=maxVariable;v++)
	{
		printf ("Level %d, %d nodes\n", v, levelNodeSets[v]->numOfElements);
	}
}

SetPtr thisLevel, nextLevel;

#define CLASHSETS 20

void copyNonTrunk (SetPtr out, SetPtr in)
{
	int i;
	clearSet (out);
	for (i=0;i<in->numOfElements;i++)
		if (!( (TernaryTreePtr) in->elements[i])->isSpine)
		{
			addToSetNoCheck (out, ( (TernaryTreePtr) in->elements[i]));
		}
}

int inSetSet (SetPtr l, SetPtr s)
{
	int i, result;
	result = false;
	for (i=0;i<l->numOfElements && !result;i++)
		result = equalSortedSet (l->elements[i], s);
	return result;
}




int clauseSetHasVar (IntSetPtr s)
{
	int result = false;
	int i;
	for (i=0;i<s->numOfElements && !result;i++)
		result = mainStem->elements[(int)s->elements[i]] != NULL;
	return result;
}
int trunkHasVar (int v)
{
	int result = false;


	result = clauseSetHasVar (negVarClauseSets[v]) || clauseSetHasVar (posVarClauseSets[v]);
	return result;
}

int trunkFirstVar()
{
	while (previousTrunkFirstVar <= maxVariable && !trunkHasVar(previousTrunkFirstVar))
		previousTrunkFirstVar++;

	return previousTrunkFirstVar;
}

void addToUnitClauseTrees (SetPtr clauses, TernaryTreePtr node)
{
	ClausePtr clause;
	int c;
	
	for (c=0;c<clauses->numOfElements;c++)
	{
		clause = clauses->elements[c];
		if (clause->numOfVariables == 1)
		{
			if (clause->positive[0])
			{
				//addToSetNoCheck(posTrees[clause->variables[0]], node);
			}
			else
			{
				//addToSetNoCheck(negTrees[clause->variables[0]], node);
			}
		}
	}
}


TernaryTreePtr ternaryTreeFromProposition (SetPtr clauses, int parentVar)
{
	TernaryTreePtr result;
	int newEntry;
	

#ifdef PROOF
	IntSetPtr originalVariables;
	long int resId;
#endif


	//printf ("ttfp level(%d)", level);pClauseSet (clauses);



/*
	if (clauses != NULL)
		ttPropagateUnitClauses (clauses);
*/
	if (clauses == NULL)
	{
			if (mainStemClauses == 0)
			{
				freeSet (mainStem);
				result = trueTernaryTree;
			}
			else
			{
				result = newTernaryTree();
				//result->id = levelListNextId[0]++;
				//result->clauses = clauses;
			  //return result;
			}
	}
	else if (clauses->numOfElements == 0)
	{
		//freeSet (clauses);
		result = trueTernaryTree;
		//return trueTernaryTree;
	}
	else if (setContainsEmptyClause (clauses
#ifdef PROOF
											,&originalVariables, &resId
#endif
																	))
	{
		//freeSet (clauses);

#ifdef PROOF
		if (booleanOptions[RPT_FILE])
			result = newFalseTernaryTree (originalVariables,resId);
		else
			result = falseTernaryTree;
		
#else
		result = falseTernaryTree;
#endif
		//return result;
	}
	else
	{



		{
			sortClauseSet (clauses);
			ttHashEntry->parentVar = parentVar;
			//ttHashEntry->clauses = clauses;
			result = NULL;
			if (!booleanOptions[RPT_FILE] && !booleanOptions[UNIQUE_PARENT])
				result = hashTableSearchInsert (ttHashTable, ttHashEntry);
			if (result == NULL)
			{
				newEntry = true;
				result = ttHashEntry;

				//result->id = levelListNextId[0]++;
				ttHashEntry = newTernaryTree();
			}
			else
			{
				newEntry = false;
				freeSet (clauses);
			}

			//return result;
		}
	}
//    if (result->clauses != NULL)
//    {
//        result->firstVar = clauseSetFirstVar (result->clauses);
//    }
		
	

	if (parentVar < result->minParent)
		result->minParent = parentVar;
	if (parentVar > result->maxParent)
		result->maxParent = parentVar;

		
	return result;
}
int distributeClause (int variable, ClausePtr clause)
{
	int v,v1, found=false, result = 0;


	for (v=0;!found && v<clause->numOfVariables; v++)
		if (clause->variables[v] == variable)
		{
			found = true;
			result = clause->positive[v]?1:-1;
			for (v1=v;v1<clause->numOfVariables-1;v1++)
			{
				clause->variables[v1] = clause->variables[v1+1];
				clause->positive[v1] = clause->positive[v1+1];
			}
			clause->numOfVariables--;

		}
	return result;
}

TernaryTreePtr ternaryTreeFromProposition2 (SetPtr clauses, int trunkVar, int parentVar)
{
	TernaryTreePtr result=NULL, left=NULL, both=NULL, right=NULL;
	int c,c1,variable=0, direction;
	SetPtr leftClauses, bothClauses, rightClauses;
	IntSetPtr negClauses, posClauses;
	ClausePtr clause;
	int searchNeeded;
	IntSetPtr posLits, negLits;

#ifdef PROOF
	IntSetPtr originalVariables;
	long int resId;
#endif

	searchNeeded = true;
	posLits = newIntSet();
	negLits = newIntSet();
	if (clauses == NULL)
	{
		if (mainStemClauses == 0)
		{
			freeSet (mainStem);
			result = trueTernaryTree;
			searchNeeded = false;
		}
		else
		{
			variable = trunkVar;
			leftClauses = newSet();
			negClauses = negVarClauseSets[variable];
			for (c=0; c<negClauses->numOfElements;c++)
			{
				c1 = negClauses->elements[c];
				if (mainStem->elements[c1] != NULL)
				{
				   clause = ((ClausePtr)mainStem->elements[c1]);
	
				   mainStem->elements[c1] = NULL;
				   mainStemClauses--;
				   distributeClause (variable, clause);
				   addToSet (leftClauses, clause);
				}
			}
	
			rightClauses = newSet();
			posClauses = posVarClauseSets[variable];
			for (c=0; c<posClauses->numOfElements;c++)
			{
				c1 = (int)posClauses->elements[c];
				if (mainStem->elements[c1] != NULL)
				{
				   clause = ((ClausePtr)mainStem->elements[c1]);
	
				   mainStem->elements[c1] = NULL;
				   mainStemClauses--;
				   distributeClause (variable, clause);
				   addToSet (rightClauses, clause);
				}
			}

			left = ternaryTreeFromProposition2 (leftClauses, trunkVar+1, variable);
			both = ternaryTreeFromProposition2 (NULL, trunkVar+1, variable);
			right = ternaryTreeFromProposition2 (rightClauses, trunkVar+1, variable);

		}
	}
	else if (clauses->numOfElements == 0)
	{
		result = trueTernaryTree;
		searchNeeded = false;
		freeSet (clauses);
	}
	else if (setContainsEmptyClause (clauses
#ifdef PROOF
											,&originalVariables, &resId
#endif
																	))
	{
 #ifdef PROOF
		if (booleanOptions[RPT_FILE])
			result = newFalseTernaryTree (originalVariables,resId);
		else
			result = falseTernaryTree;
		
#else
		result = falseTernaryTree;
#endif
		searchNeeded = false;
		freeClauseSet (clauses);
	}
	else
	{
		variable =  clauseSetFirstVar (clauses);
		leftClauses = newSet();
		bothClauses = newSet();
		rightClauses = newSet();

		for (c=0;c<clauses->numOfElements;c++)
		{
			clause = /*newCopyClause*/((ClausePtr)clauses->elements[c]);
			direction = distributeClause (variable, clause);
			switch (direction)
			{
				case 1:    addToSet (rightClauses, clause);
							break;
				case 0:     addToSet (bothClauses, clauses->elements[c]);
							//freeClause (clause);
							break;
				case -1:     addToSet (leftClauses, clause);
							break;
			}
		}
		left = ternaryTreeFromProposition2 (leftClauses,trunkVar, variable);
		both = ternaryTreeFromProposition2 (bothClauses,trunkVar, variable);
		right = ternaryTreeFromProposition2 (rightClauses,trunkVar, variable);

		freeSet (clauses);

	}
 
	 if (searchNeeded)
	 {
		 ttHashEntry->firstVar = variable;
		 ttHashEntry->left = left;
		 ttHashEntry->both = both;
		 ttHashEntry->right = right;
		 result = hashTableSearchInsert (ttHashTable, ttHashEntry);
		 if (result == NULL)
		 {
			 result = ttHashEntry;
			 result->minParent = parentVar;
			 result->maxParent = parentVar;
			 result->firstVar = variable;
			 ttHashEntry = newTernaryTree();
		 }
		 else
		 {
			 if (parentVar < result->minParent)
				 result->minParent = parentVar;
			 if (parentVar > result->maxParent)
				 result->maxParent = parentVar;
		 }
		 addToIntSet (result->allParents, parentVar);
		if (!result->isFalsifiable)
			result->weight = maxVariable+1;
		else
		{
			falseTernaryTree->weight = result->firstVar;
			trueTernaryTree->weight = maxVariable+1;
			result->weight = result->left->weight;
			if (result->both->weight < result->weight) result->weight = result->both->weight;
			if (result->right->weight < result->weight) result->weight = result->right->weight;
		}

	 }
	 
	 freeIntSet (posLits);
	 freeIntSet (negLits);  
	return result;
}


#ifdef PROOF
void assignOriginalVariables (ClausePtr clause)
{
	int i,v;
/*
	clause->originalVariables = newIntSet();

	for (i=0;i<clause->numOfVariables;i++)
	{
		v = clause->variables[i];
		if (clause->positive[i])
			addToIntSet (clause->originalVariables, v);
		else
			addToIntSet (clause->originalVariables, -v);
	}
*/
}
#endif

void assignTernaryTreeChildren (int variable, TernaryTreePtr node)
{
	int c,c1, direction;
	IntSetPtr posClauses, negClauses;
	SetPtr leftClauses, bothClauses, rightClauses;
	SetPtr clauses=NULL;
	ClausePtr clause;

   // clauses = node->clauses;


	if (!node->isSatisfiable || !node->isFalsifiable)
		return;

	if (clauses == NULL)
	{
		leftClauses = newSet();
		negClauses = negVarClauseSets[variable];
		for (c=0; c<negClauses->numOfElements;c++)
		{
			c1 = negClauses->elements[c];
			if (mainStem->elements[c1] != NULL)
			{
			   clause = ((ClausePtr)mainStem->elements[c1]);


			   mainStem->elements[c1] = NULL;
			   mainStemClauses--;
			   distributeClause (variable, clause);
			   addToSet (leftClauses, clause);
			}
		}

		rightClauses = newSet();
		posClauses = posVarClauseSets[variable];
		for (c=0; c<posClauses->numOfElements;c++)
		{
			c1 = (int)posClauses->elements[c];
			if (mainStem->elements[c1] != NULL)
			{
			   clause = ((ClausePtr)mainStem->elements[c1]);

			   mainStem->elements[c1] = NULL;
			   mainStemClauses--;
			   distributeClause (variable, clause);
			   addToSet (rightClauses, clause);
			}
		}

		node->left = ternaryTreeFromProposition (leftClauses, variable);
		node->both = ternaryTreeFromProposition (NULL, variable);
		node->right = ternaryTreeFromProposition (rightClauses, variable);

	}


	else if (node->firstVar == variable)
	{
		leftClauses = newSet();
		bothClauses = newSet();
		rightClauses = newSet();

		for (c=0;c<clauses->numOfElements;c++)
		{
			clause = newCopyClause((ClausePtr)clauses->elements[c]);
			direction = distributeClause (variable, clause);
			switch (direction)
			{
				case 1:    addToSet (rightClauses, clause);
							break;
				case 0:     addToSet (bothClauses, clauses->elements[c]);
							freeClause (clause);
							break;
				case -1:     addToSet (leftClauses, clause);
							break;
			}
		}
		node->left = ternaryTreeFromProposition (leftClauses,node->firstVar);
		node->both = ternaryTreeFromProposition (bothClauses,node->firstVar);
		node->right = ternaryTreeFromProposition (rightClauses,node->firstVar);

		//freeSet (clauses);

	}

	

}

int assignAllTernaryTreeChildren (int variable, SetPtr nodes)
{
	TernaryTreePtr node;
	int i,result;

	result = false;

	for (i=0;i<nodes->numOfElements;i++)
	{
		node = (TernaryTreePtr) nodes->elements[i];
		if (node->both == NULL)
		{
			result = true;
			assignTernaryTreeChildren (variable, node);
		}
	}
	return result;
}


void dotFileLine (AssignmentPtr assignment, int level, int kind)
{
	int v;

	fprintf (dotFile,"\"");
	for (v=0;v<level;v++)
	{
		if (assignment->values[v])
			fprintf (dotFile, "%d",globalUnMapVariable[v+1]);
		else
			fprintf (dotFile, "-%d",globalUnMapVariable[v+1]);
		fprintf (dotFile,".");
	}
	if (level == maxVariable)
		fprintf (dotFile, "0");
	else
	{
		 if (assignment->values[level])
			fprintf (dotFile, "%d",globalUnMapVariable[level+1]);
		 else
			fprintf (dotFile, "-%d",globalUnMapVariable[level+1]);
	}
	fprintf (dotFile,"\" ");
	switch (kind)
	{
		case 0: fprintf (dotFile, "[label=\"\", shape=box, color=\"red\", style=filled]\n");
				break;

		case 1: fprintf (dotFile, "[label=\"\", shape=box, color=\"green\", style=filled]\n");
				break;

		case 2: fprintf (dotFile, "[label=\"\", shape=point, color=\"black\"]\n");
				break;
	}

	if (level > 0)
	{
		fprintf (dotFile,"\"");
		for (v=0;v<level;v++)
		{
			if (assignment->values[v])
				fprintf (dotFile, "%d",globalUnMapVariable[v+1]);
			else
				fprintf (dotFile, "-%d",globalUnMapVariable[v+1]);
			if (v<level-1)
				fprintf (dotFile,".");
		}
		fprintf (dotFile,"\" -- \"");
		for (v=0;v<level;v++)
		{
			if (assignment->values[v])
				fprintf (dotFile, "%d",globalUnMapVariable[v+1]);
			else
				fprintf (dotFile, "-%d",globalUnMapVariable[v+1]);
			fprintf (dotFile,".");
		}
		if (level == maxVariable)
			fprintf (dotFile, "0");
		else
		{
			if (assignment->values[level])
				fprintf (dotFile, "%d",globalUnMapVariable[level+1]);
			else
				fprintf (dotFile, "-%d",globalUnMapVariable[level+1]);
		}
		fprintf (dotFile,"\" ");
		fprintf (dotFile, "[label=\"%d\", fontcolor=green, color=green, style=bold];\n",
			assignment->values[level]?globalUnMapVariable[level]:-globalUnMapVariable[level]);
	}
}

int literalFromPair (int v, int p)
{
	return p?maxVariable+v:maxVariable-v;
}

void pairFromLiteral (int *nextV, int *firstDirection, int bestLiteral)
{
	if (bestLiteral > maxVariable)
	{
		*firstDirection = true;
		*nextV = bestLiteral-maxVariable;
	}
	else
	{
		*firstDirection = false;
		*nextV = maxVariable-bestLiteral;
	}
}



void indent (int level)
{
	int i;
	for (i=0;i<level;i++)
		printf ("  ");
}

int triesMod = 10;

int isLeaf (TernaryTreePtr t)
{
	return (!t->isSatisfiable) || (!t->isFalsifiable);
}

int chooseBit (unsigned long bits, int pos)
{
	int result = 1 & (bits>>pos);
	
	return result;
}

int countBits (unsigned long bits)
{
	int result = 0;
	int pos;
	for (pos=0;pos<32;pos++)
	{
		result += 1 & bits;
		bits >>=1;
	}
	return result;
}
unsigned long reduceBits (unsigned long bits)
{
	int pos;
	unsigned long result;
	result = 0;
	for (pos=0;pos<16;pos++)
	{
		result <<= 1;
		result |= chooseBit (bits, 2*pos) | chooseBit (bits, 2*pos+1);
	}
	return result;
}



TernaryTreePtr populateTernaryTree (int trunkFirstVar, TernaryTreePtr t)
{
		
	
	if (t->both == NULL)
	{
//        if (t->clauses == NULL)
//            t->firstVar = trunkFirstVar;
		assignTernaryTreeChildren (t->firstVar, t);
		t->both = populateTernaryTree (trunkFirstVar+1, t->both);
		t->left = populateTernaryTree (trunkFirstVar+1, t->left);
		t->right = populateTernaryTree (trunkFirstVar+1, t->right);
	}

		if (!t->isFalsifiable)
			t->weight = maxVariable+1;
		else
		{
			if (!t->left->isSatisfiable) t->left->weight = t->firstVar;
			if (!t->both->isSatisfiable) t->both->weight = t->firstVar;
			if (!t->right->isSatisfiable) t->right->weight = t->firstVar;
			t->weight = t->left->weight;
			if (t->both->weight < t->weight) t->weight = t->both->weight;
			if (t->right->weight < t->weight) t->weight = t->right->weight;
		}
		//printf ("weight %e\n", t->weight);

#ifndef PROOF
		if (!t->left->isSatisfiable && !t->right->isSatisfiable)
		{
			return falseTernaryTree;
		}
#endif

		if (!t->left->isFalsifiable && !t->right->isFalsifiable && !t->both->isFalsifiable)
		{
			return trueTernaryTree;
		}




	return t;
}

int treeSize (int depth, TernaryTreePtr root)
{
	if (!root->isSatisfiable)
		return depth;
	else if (!root->isFalsifiable)
		return 0;
	else return treeSize (depth+1, root->left) + treeSize (depth+1, root->both) + treeSize (depth+1, root->right);
}
TernaryTreePtr  buildAllTernary (SetPtr clauses)
{
	TernaryTreePtr result;
	
		


		
	result = ternaryTreeFromProposition (clauses,0);
	result = populateTernaryTree (1,result);
	topTree = result;
	
	printf ("c size = %d\n", treeSize(0,result));
	
	return result;
}


FILE *dotFile, *staticDotFile;

void printStaticDotFileNode (TernaryTreePtr node, int newValue);

void printStaticDotFileLink (TernaryTreePtr node, TernaryTreePtr child, char *label, char *colour, int newValue)
{
#ifdef DO_DOT_FILE
	if (!child->isFalsifiable)
	{
		fprintf (staticDotFile, "\"%dT\" [label=\"%dT\",shape=point,width=.1,height=.1]\n",node,child);
		fprintf (staticDotFile, "\"%d\" -- \"%dT\" [label=\"%s\",color=\"%s\"]\n",node,node,label,colour);
	}
	else if (!child->isSatisfiable)
	{
		fprintf (staticDotFile, "\"%dF\" [label=\"%dF\",shape=triangle,width=.2,height=.1]\n",node,child);
		fprintf (staticDotFile, "\"%d\" -- \"%dF\" [label=\"%s\",color=\"%s\"]\n",node,node,label,colour);
	}
	else
	{
		printStaticDotFileNode (child, newValue);
		fprintf (staticDotFile, "\"%d\" -- \"%d\" [label=\"%s\",color=\"%s\"]\n",node,child,label,colour);
	}
#endif
}

char setToStringBuffer[200];

char *setToString(IntSetPtr s)
{
	char t[10];
	int i;

	sprintf (setToStringBuffer, "{");
	for (i=0;s != NULL && i<s->numOfElements;i++)
	{
		if (i>0) strcat (setToStringBuffer, ",");
		sprintf (t, "%d", s->elements[i]);
		strcat (setToStringBuffer, t);
	}
	sprintf (t, "}");
	strcat (setToStringBuffer, t);
	return setToStringBuffer;

}
void printStaticDotFileNode (TernaryTreePtr node, int newValue)
{
#ifdef DO_DOT_FILE
	if (node->isSatisfiable && node->isFalsifiable && 0 != newValue)
	{
		

		fprintf (staticDotFile, "\"%d\" [label=\"%d(%d)<%d,%d>%s\",shape=box,color=\"black\"]\n",node, node, node->firstVar, node->minParent, node->maxParent,setToString(node->allParents));
		printStaticDotFileLink (node, node->left,"","red", newValue);
		printStaticDotFileLink (node, node->right,"","green", newValue);
		printStaticDotFileLink (node, node->both,"","black", newValue);
	}
#endif
}

static int nextNewValue=2;
void printStaticDotFile (char *filename, TernaryTreePtr root)
{
	int newValue;
	staticDotFile = fopen (filename,"w");
	newValue = nextNewValue++;

	fprintf (staticDotFile, "graph G {\nsize=\"60,90\";");
	printStaticDotFileNode (root, newValue);
	fprintf (staticDotFile,"}\n");

	fclose (staticDotFile);
}


void notePAProgress (int level)
{
	int v,count;

	//printf ("Level %d\n",level);

	for (v=level+1;v<6;v++)
		paLevelCounts[v] = 0;

	paLevelCounts[level]++;

	count = 0;

	for (v=0;v<=level;v++)
		count = 2*count + (paLevelCounts[v]-1);

	for (v=level+1;v<6;v++)
		count *= 2;

	//printf ("Count = %d\n", count);
	for (v=0;v<count;v++)
		paprogress[v] = '*';

}


void logProgress(int level)
{
	double cpuTime=getCPUTimeSinceStart();




	int i,count;

	if (cpuTime < 0.1) cpuTime = 0.1;


/*
	if ((tries%10 == 0) && ( (f = fopen(STOP_TTS_FILENAME,"r")) != NULL) )
	{
		unlink (STOP_TTS_FILENAME);
		sprintf (logMessageBuffer,"Stopped by presence of file <%s>",STOP_TTS_FILENAME);
		dimacsComment (logMessageBuffer);
		abortDIMACS();
	}

	if (intOptions[ASSIGNMENT_LIMIT] != -1 && tries >= intOptions[ASSIGNMENT_LIMIT])
	{
		sprintf (logMessageBuffer,"Assignment limit (%d) reached", intOptions[ASSIGNMENT_LIMIT]);
		dimacsComment (logMessageBuffer);
		abortDIMACS();
	}

*/
	if (intOptions[CPU_LIMIT] != -1 && cpuTime >= intOptions[CPU_LIMIT])
	{
		sprintf (logMessageBuffer,"CPU limit (%d) reached", intOptions[CPU_LIMIT]);
		dimacsComment (logMessageBuffer);
		abortDIMACS();
	}

	if (!logHeaderPrinted)
	{
		logHeaderPrinted = 1;
		dimacsComment ("");
		dimacsComment ("PA progress                      | CPU progress Partial assignments | improvements      CPU time        PA/sec        nodes");
		dimacsComment ("---------------------------------|----------------------------------|------------------------------------------------------");
	}

	if (intOptions[CPU_LIMIT] == -1)
		strcpy (cpuprogress,"          ");
	else
	{
		count = 10*cpuTime/intOptions[CPU_LIMIT];
		if (count>10) count = 10;

		for (i=0;i<count;i++) cpuprogress[i] = '*';
		for (i=count;i<10;i++) cpuprogress[i] = '.';
		cpuprogress[10] = '\0';
	}

	if (level < 6)
	{
		notePAProgress (level);
		sprintf (logMessageBuffer,"%s | %s    %18.d | %-12.d    %10.1f   %11.1f %12.d",paprogress,cpuprogress,tries,improvements,cpuTime,tries/cpuTime,subsetptrs);
		dimacsComment (logMessageBuffer);
	}
	else if (intOptions[CPU_LIMIT] != -1 && cpuTime >= logCPULimit)
	{

		sprintf (logMessageBuffer,"%s | %s    %18.d | %-12.d    %10.1f   %11.1f %12.d",paprogress,cpuprogress,tries,improvements,cpuTime,tries/cpuTime, subsetptrs);
		dimacsComment (logMessageBuffer);
		logCPULimit += intOptions[CPU_LIMIT]/50.0;

	}
	else if (false && tries >= logLimit)
	{

		sprintf (logMessageBuffer,"%s | %s    %18.d | %-12.d    %10.1f   %11.1f",paprogress,cpuprogress,tries,improvements,cpuTime,tries/cpuTime);
		dimacsComment (logMessageBuffer);
		logLimit *= 2;
	}
	fflush (stdout);
}

int tryLimit;

SetPtr all;
int largestSetSize, increaseRequests;
int stepSize;

int checkNode (int tid, int count, TernaryTreePtr root, SetPtr falseSet)
{
	int i;
	TernaryTreePtr node1, next;

if (count > 6) return true;
	
	if (!root->left->isSatisfiable && root->right->isSatisfiable)
	{
		for (i=0;i<allSet[tid][root->firstVar]->numOfElements;i++)
		{
			node1 = allSet[tid][root->firstVar]->elements[i];
			next = node1->right;
			if (next->isSatisfiable && next->isFalsifiable && !next->left->isSatisfiable && posTrees[tid][next->firstVar]->numOfElements > 0)
			{
				clearSet (falseSet);
				addToSet (falseSet, node1);
				addToSet (falseSet, posTrees[tid][next->firstVar]->elements[0]);
				if (count>1)printf ("<%d>",count);
				return false;
			}
			if (next->isSatisfiable && next->isFalsifiable && !next->right->isSatisfiable && negTrees[tid][next->firstVar]->numOfElements > 0)
			{
				clearSet (falseSet);
				addToSet (falseSet, node1);
				addToSet (falseSet, negTrees[tid][next->firstVar]->elements[0]);
				if (count>1)printf ("<%d>",count);
				return false;
			}
			if (!checkNode (tid, count+1, next, falseSet))
			{
				addToSet (falseSet, node1);
				return false;
			}
		}
	}
	if (!root->right->isSatisfiable && root->left->isSatisfiable)
	{
		for (i=0;i<allSet[tid][root->firstVar]->numOfElements;i++)
		{
			node1 = allSet[tid][root->firstVar]->elements[i];
			next = node1->left;
			if (next->isSatisfiable && next->isFalsifiable && !next->left->isSatisfiable && posTrees[tid][next->firstVar]->numOfElements > 0)
			{
				clearSet (falseSet);
				addToSet (falseSet, node1);
				addToSet (falseSet, posTrees[tid][next->firstVar]->elements[0]);
				if (count>1)printf ("<%d>",count);
				return false;
			}
			if (next->isSatisfiable && next->isFalsifiable && !next->right->isSatisfiable && negTrees[tid][next->firstVar]->numOfElements > 0)
			{
				clearSet (falseSet);
				addToSet (falseSet, node1);
				addToSet (falseSet, negTrees[tid][next->firstVar]->elements[0]);
				if (count>1)printf ("<%d>",count);
				return false;
			}
			if (!checkNode (tid, count+1, next, falseSet))
			{
				addToSet (falseSet, node1);
				return false;
			}
		}
	}
	return true;

}
int tryToAdd (int tid, int firstVar, TernaryTreePtr root, SetPtr b, int atStart, SetPtr falseSet)
{
	int found;
	found = false;
		
	if (root->isFalsifiable && !root->isSelected[tid] )
	{
		root->isSelected[tid] = true;
		root->actualParent[tid] = firstVar;
		addToSetNoCheck (allSet[tid][root->firstVar], root);
		addToSetNoCheck (b, root);

#ifdef PROOF
		return true;
#endif

		//addToSetNoCheck(all,root);
		if (!root->left->isSatisfiable)
			addToSet (negTrees[tid][root->firstVar], root);
		if (!root->right->isSatisfiable)
			addToSet (posTrees[tid][root->firstVar], root);
		if (posTrees[tid][root->firstVar]->numOfElements > 0 && negTrees[tid][root->firstVar]->numOfElements > 0)
		{
			clearSet (falseSet);
			addToSet (falseSet, posTrees[tid][root->firstVar]->elements[0]);
			addToSet (falseSet, negTrees[tid][root->firstVar]->elements[0]);
			//printf (".");
			return false;
		}
		else if (false)
		{
			if (!checkNode (tid, 0, root, falseSet))
			{
				addToSet (falseSet, root);
				//printf ("[%d]", falseSet->numOfElements);
				//printSet (falseSet);
				return false;
			}                    
		}
			
	 }
	 return true; 
}




void printIndent (int indent)
{
	int i;
	for (i=0;i<indent;i++) printf (" ");
}


int *globalCount;
SubsetTreePtr removeUnused (SubsetTreePtr root, int tid)
{
	SubsetTreePtr result;
	
	if (root == NULL)
		return NULL;
		
	if (root->present == endPoint)
	{
//        if (root->count == 0)
		{
			result = removeUnused (root->absent, tid);
			(*globalCount)--;
			returnSubsetTree (root, tid);
			return result;
		}
//        else
		{
			root->absent = removeUnused (root->absent, tid);
			return root;
		}
	}
	else
	{
		root->present = removeUnused (root->present, tid);
		root->absent = removeUnused (root->absent, tid);
		if (root->present == NULL)
		{
			result = root->absent;
			returnSubsetTree (root, tid);
			return result;
		}
		else
			return root;
	}
}

SubsetTreePtr topSubset = NULL;

SetPtr checkOneList (int tid, SetPtr subset, SubsetTreePtr root, int firstVar, unsigned long *setId)
{
	TernaryTreePtr node;
	SetPtr result1, result2;
	unsigned long setId2;
	

	if (root == NULL)
		return NULL;
	else if (root == endPoint)
		return newSet();
	else
	{


		while (root != NULL && (!( (TernaryTreePtr)root->tree)->isSelected[tid] /*|| ((TernaryTreePtr)root->tree)->firstVar < firstVar*/))
			root = root->absent;
		 if (root == NULL  /*|| ((TernaryTreePtr) root->tree ) ->firstVar < firstVar*/ )
			return false;
		result1 = checkOneList (tid, subset, root->absent, firstVar, setId);
		if (result1 != NULL) return result1;
		node = root->tree;
		if (node->isSelected[tid] /*&& node->firstVar >= firstVar */)
		{
			result2 = checkOneList (tid, subset, root->present, firstVar, &setId2);
			if ( ( (TernaryTreePtr)root->tree)->isSelected[tid] && root->present == endPoint)
			{
#ifdef PROOF
				setId2 = root->setId;
#endif
				//printf (" *** %d\n", root->numAssignments);
			}
			if (result2 != NULL)
			{
				if (result1 == NULL)
				{
#ifdef PROOF
					*setId = setId2;
#endif
					addToSetNoCheck (result2, root->tree);
					result1 = result2;
				}
				else
				{
					if (result2->numOfElements+1 < result1->numOfElements)
					{
						//printf ("<%d,%d>\n", numAssignments2, *numAssignments);
#ifdef PROOF
						*setId = setId2;
#endif
						addToSetNoCheck (result2, root->tree);
						freeSet (result1);
						result1 = result2;
					}
					else
					{
						freeSet (result2);
					}
				}
			}
		}

	}


	return result1;

}

SetPtr bitsCheckOneNode (TernaryTreePtr node, SetNodePtr sn, int pos, unsigned char *bytes)
{
	unsigned char b;
	int i,j;
	SetPtr result;
	
	if (sn == NULL)
		return newSet();
	result = NULL;   
	b = bytes[pos];
	
	for (i=0;result == NULL && i<sn->numOfNodes;i++)
	{
		if ( (b | sn->bytes[i]) == b)
			result = bitsCheckOneNode (node, sn->children[i], pos+1, bytes);
		if (result != NULL)
		{
			for (j=0;j<8;j++)
				if ( (sn->bytes[i] & (1<<j)) != 0)
				{
					//addToSetNoCheck (result, node->subsetElements->elements[pos*8+j]);
				}
		}
	}
	return result;
}
SetPtr bitsCheckOneList (int tid, SetPtr subset, TernaryTreePtr node, int firstVar)
{
	SetPtr result=NULL;
	
/*
	numBytes = (node->subsetElements->numOfElements+7)/8;
	for (i=0;i<numBytes;i++)
		bytes[i] = 0;
	for (i=0;i<node->subsetElements->numOfElements;i++)
	{
		t = node->subsetElements->elements[i];
		if (t->isSelected[tid])
			bytes[i/8] |= 1<<(i%8);
	}
	result = bitsCheckOneNode (node, node->setNode, 0, bytes);

	if (result != NULL)
	{
		qsort (result->elements, result->numOfElements, sizeof (TernaryTreePtr), ternaryTreeComparatorNew);
		//printf ("Yes!! "); printSet (result); printf("\n");
	}
*/    
	return result;
}

SetPtr checkForSubset (int tid, SetPtr subset, SetPtr superset, int firstVar, unsigned long *setId)
{
	int i;
	TernaryTreePtr node;
	SetPtr result, temp;
	unsigned long setIdTemp;


	//found = checkOneList (subset, allSubsets, firstVar);

	result = NULL;

	for (i=0; result == NULL && i<superset->numOfElements; i++)
	{
		node = superset->elements[i];
		
		//printf ("cn = %d\n", node);
#ifdef THREADS
	//checkP(pthread_rwlock_rdlock(&node->nodeLock));
#endif
		temp = checkOneList (tid, subset, node->subsets, firstVar,  &setIdTemp);
		//temp = bitsCheckOneList (tid, subset, node, firstVar);
#ifdef THREADS
	//checkP(pthread_rwlock_unlock(&node->nodeLock));
#endif
		if (result == NULL)
		{
			result = temp;
#ifdef PROOF
			*setId = setIdTemp;
#endif
		}
		else if (temp != NULL)
		{
			if (temp->numOfElements < result->numOfElements)
			{
#ifdef PROOF
				*setId = setIdTemp;
#endif
				freeSet (result);
				result = temp;
			 }
			else
				freeSet (temp);
		}
	}


	if (result != NULL)
	{
		//sortSet (result);
		//printf ("Yes!! "); printSet (result); printf("\n");
		
	}
	return result;
}

SubsetTreePtr everything=NULL;

SubsetTreePtr addFalseSetSubNew (int tid, SubsetTreePtr startPoint, int index, SetPtr set, int numAssignments, int setId)
{
	SubsetTreePtr newNode, result, next;

	result = startPoint;
	
	if (startPoint == endPoint)
	{
		return startPoint;
	}
	if (startPoint == NULL && index != set->numOfElements)
	{
		result = newSubsetTree(tid);
		if (result == NULL) return NULL;
	}
	if (index == set->numOfElements)
	{
		result = endPoint;
	}
	else
	{
		if ( result->tree == NULL)
			result->tree = set->elements[index];
		if ( result->tree == set->elements[index])
		{
			next = addFalseSetSubNew (tid,result->present, index+1, set, numAssignments, setId);
			if (next == NULL)
				return NULL;
			else
				result->present = next;
			if (index+1 == set->numOfElements)
			{
#ifdef PROOF
				result->setId = setId;
#endif
			}
			//printf ("AAA %d\n", numAssignments);
		}
		else if ( result->tree <set->elements[index]/*ternaryTreeComparatorFirstVar (&result->tree, &set->elements[index]) == -1*/)
		{
			next = addFalseSetSubNew (tid,result->absent, index, set, numAssignments, setId);
			if (next == NULL)
				return NULL;
			else
				result->absent = next;
		}
		else
		{
			newNode = newSubsetTree(tid);
			if (newNode == NULL) return NULL;
			newNode->tree = set->elements[index];
			newNode->absent = result;
			next = addFalseSetSubNew (tid,newNode->present, index+1, set, numAssignments, setId);
			if (next == NULL)
				return NULL;
			else
				newNode->present = next;
			if (index+1 == set->numOfElements)
			{
#ifdef PROOF
				newNode->setId = setId;
#endif
			}
			//printf ("BBB %d\n", numAssignments);
			result = newNode;
		 }
	}

	return result;
}

int allAbove (int firstVar, SetPtr s, SetPtr middles, SetPtr side)
{
	int answer=true;
	int i;

	for (i=0;answer && i<s->numOfElements; i++)
		//answer =  (( (TernaryTreePtr) s->elements[i])->parentVar < firstVar )  || setMemberOfU (middles, s->elements[i]);
		answer =  /* setMemberOfU (middles, s->elements[i]) || */(!setMemberOfU (side, s->elements[i]));

	return answer;
}
int totallyAbove (int firstVar, SetPtr s, SetPtr middles, SetPtr side)
{
	int answer=true;
	int i;

	for (i=0;answer && i<s->numOfElements; i++)
		//answer =  (( (TernaryTreePtr) s->elements[i])->parentVar < firstVar )  || setMemberOfU (middles, s->elements[i]);
		answer =  (!setMemberOfU (middles, s->elements[i])) && (!setMemberOfU (side, s->elements[i]));

	return answer;
}

int newFalseFound, checkFrequency=10, doLogging;

void checkP (int r)
{ 
	if (r != 0)
	{
		printf ("error %d\n",r);
		exit(r);
	}
}

int answerFoundSet;
AssignmentPtr answerFound;
AssignmentPtr getAnswer()
{
	return answerFound;
}

#ifdef THREADS
pthread_mutex_t answerFoundMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

void clearAnswerFound()
{
	answerFoundSet = 0;
}


void setAnswerFound (int tid, AssignmentPtr answer)
{
	
#ifdef THREADS
	pthread_mutex_lock (&answerFoundMutex);
#endif

	answerFound = answer;
	answerFoundSet = 1;

#ifdef THREADS
	pthread_mutex_unlock (&answerFoundMutex);
	pthread_exit(1);
#endif
}

double setWeight (SetPtr s)
{
	double result;
	TernaryTreePtr node;
	int i;
	
	result = 0;
	
	for (i=0;i<s->numOfElements;i++)
	{
		node = s->elements[i];
		result += node->weight;
	}
	
	return result;
}

void printAll1 (TernaryTreePtr root)
{
	if (root->isSatisfiable && root->isFalsifiable)
	{
		//printf ("<%d,%d>", root->descendants->numOfElements, root->setCount);
		printAll1 (root->left);
		printAll1 (root->both);
		printAll1 (root->right);
	}
}
 


void bitsAddFalseSubsetNew (int tid, TernaryTreePtr node, SetPtr falseSet)
{
	IntSetPtr poses;
	int maxPos;
	
	//printf ("adding "); printSet (falseSet); printf("\n");
	poses = newIntSet();
	maxPos = 0;
/*    
	for (i=0;i<falseSet->numOfElements;i++)
	{
		n = falseSet->elements[i];
		node->seBuffer->tree = n;
		newBuffer = hashTableSearchInsert (node->subsetElementsTable, node->seBuffer);
		if (newBuffer == NULL)
		{
			newBuffer = node->seBuffer;
			newBuffer->pos = node->subsetElements->numOfElements;
			addToSetNoCheck (node->subsetElements, n);
			node->seBuffer = newseBuffer();
		}
		if (newBuffer->pos > maxPos)
			maxPos = newBuffer->pos;
		addToIntSetNoCheck (poses, newBuffer->pos);

	}
	//printf ("<%d>", maxPos);
	numBytes = 1 + (maxPos)/8;
	bytes = (unsigned char *) checkMalloc (numBytes, "");
	
	for (b=0;b<numBytes;b++)
		bytes[b] = 0;
		
	for (i=0;i<poses->numOfElements;i++)
	{
		pos = poses->elements[i];
		bytes[pos/8] |= 1<<(pos%8);
	}
	addIntoSetNode (node->setNode, 0, numBytes, bytes);
	//printSet (poses);
	
	free (bytes);
	
	freeIntSet (poses);

*/
}

int countAssignments (SetPtr s, int firstVar, int lastVar)
{
	SetPtr left, right;
	int i, result=0;
	TernaryTreePtr node;
	
	if (setContainsFalse (s))
		return 0;
	else if (firstVar == lastVar)
		return 1;
	else
	{
		left = newSet();
		right = newSet();
		
		for (i=0;i<s->numOfElements;i++)
		{
			node = s->elements[i];
			if (node->firstVar > firstVar)
			{
				addToSet (left, node);
				addToSet (right, node); 
			}
			else
			{
				if (node->left->isFalsifiable)
					addToSet (left, node->left);
				if (node->right->isFalsifiable)
					addToSet (right, node->right);
				if (node->both->isFalsifiable)
				{
					addToSet (left, node->both);
					addToSet (right, node->both);
				}
			}
		}
		result = countAssignments (left, firstVar+1, lastVar) + countAssignments (right, firstVar+1, lastVar);
		
		freeSet (left);
		freeSet (right);
	}
	return result;
}

TernaryTreePtr topParent (int tid, TernaryTreePtr node)
{
	while (node->parent != NULL && node->parent->isSelected[tid])
	{
	   node = node->parent;
	}
		
	return node;
}

double falseSetCount=0.0, falseSetSize = 0.0;

void newAddFalseSubset (int tid, SetPtr falseSet, int setId)
{
	TernaryTreePtr highest, node;
	int highestMin,i, numAssignments=0;
	

	
	if (falseSet->numOfElements == 0)
	{
		newFalseFound = true;
		return;
	}

	highestMin = ( (TernaryTreePtr)falseSet->elements[0]) ->actualParent[tid];
	highest = (TernaryTreePtr)falseSet->elements[0];

	for (i=1;i<falseSet->numOfElements;i++)
	{
		node = falseSet->elements[i];
		if (node->actualParent [tid]> highestMin)
		{
			highest = node;
			highestMin = node->actualParent[tid];
		}
	}

#ifdef THREADS
			checkP(pthread_rwlock_wrlock(&highest->nodeLock));
#endif
	highest->subsets = addFalseSetSubNew (tid, highest->subsets, 0, falseSet, numAssignments, setId);
#ifdef THREADS
			checkP(pthread_rwlock_unlock(&highest->nodeLock));
#endif

/*
	node = falseSet->elements[0];
	highestMin = node->actualParent;
	highest = node;
	for (i=1;i<falseSet->numOfElements;i++)
	{
		node = falseSet->elements[i];
		if (node->actualParent > highestMin)
		{
			highestMin = node->actualParent;
			highest = node;
		}
	}

	for (i=0;i<falseSet->numOfElements;i++)
	{
		node = falseSet->elements[i];
		node->subsets = addFalseSetSubNew (tid, node->subsets, 0, falseSet, numAssignments, setId);
		if (( node->maxParent >= highestMin || ( (node->minParent == highestMin) && (node >= highest ))))
		{
#ifdef THREADS
			checkP(pthread_rwlock_wrlock(&node->nodeLock));
#endif
			if (false &&  (tries % intOptions[SET_BASE] <intOptions[SET_COUNT]))
			{
				printf ("<%d,",node->setCount );
				globalCount = &node->setCount;
				node->subsets = removeUnused (node->subsets, tid);
				printf ("%d>",node->setCount );
			}
			//printf ("<%d,%d>", node->descendants->numOfElements, falseSet->numOfElements);
			//bitsAddFalseSubsetNew (tid, node,falseSet);
			//node->subsets = addFalseSetSubNew (tid, node->subsets, 0, falseSet, numAssignments, setId);
			//unionToSet (node->descendants, falseSet);
			//node->setCount++;
			//count++;
			
			//printf ("<%d %d>", node->descendants->numOfElements, node->setCount);
#ifdef THREADS
			checkP(pthread_rwlock_unlock(&node->nodeLock));
#endif
			m1 = true;
		}
	}
	//printf ("<%d>",count);
*/    

}

int bufferSMRU (int tid, SetPtr s, void *p)
{
	TernaryTreePtr node;
	node = p;
	if (!node->left->isSatisfiable)
		setMemberOfRemoveU(negTrees[tid][node->firstVar], node);
	if (!node->right->isSatisfiable)
		setMemberOfRemoveU(posTrees[tid][node->firstVar], node);
	return setMemberOfRemoveU (s,p);
}

void addDescSet (TernaryTreePtr node, SetPtr falseSet)
{
	TernaryTreePtr t;
	int i;
	
	//elements = node->descendants;
	
	for (i=0;i<falseSet->numOfElements;i++)
	{
		t = falseSet->elements[i];
		
	}
}

int randomRange(int first, int last)
{
   return first + rand() % (last+1-first); 
}
int signsSatisfyNode (int tid, TernaryTreePtr node, int firstVar, int lastVar)
{
	int result,v;
	
	if (!node->isSatisfiable)
		result =  false;
	
	else if (!node->isFalsifiable || node->firstVar >lastVar)
		result =  true;
		  
	else if (sign[node->firstVar] == 1)
		result =  signsSatisfyNode (tid, node->left, firstVar, lastVar) && signsSatisfyNode(tid, node->both, firstVar, lastVar);
	else
		result = signsSatisfyNode (tid, node->both, firstVar, lastVar) && signsSatisfyNode(tid, node->right,firstVar, lastVar);
	

		if (!result)
		{
			v = randomRange(firstVar, lastVar);
			sign[v] = -sign[v];
		}
		
		return result;
}

int signsSatisfyRange (int tid, int firstVar, int lastVar)
{
	int v,i, result;
	
	result = true;
	for (v=firstVar; result && v<=lastVar; v++)
	{
		for (i=0; result &&  i<allSet[tid][v]->numOfElements; i++)
			result = signsSatisfyNode (tid, allSet[tid][v]->elements[i], firstVar, lastVar);
	}
	
	return result;
}


void setSigns (int tid, int firstVar)
{
	int lastVar, c, result;
	
	lastVar = firstVar + 10;
	if (lastVar > maxVariable)
		lastVar = maxVariable;
		
	result = false;
	
//    for (v=firstVar;v<=lastVar; v++)
//        sign[v] = -1;

	for (c=0;!result && c<1000;c++)
	{
		result = signsSatisfyRange (tid, firstVar, lastVar);
		//printf ("c=%d r=%d\n", c, result);
	}
	if (result) printf ("*");
	
//   if (result)
//   {
//       printf ("<%d>", firstVar);
//       for (v=firstVar;v<=lastVar;v++) printf ("%d ", v*sign[v]);
//       printf ("\n");
//   }
//   else printf ("\n-------------------\n");  
}
void satCount (SetPtr s, int firstVar, int lastVar, int *leftCount, int *rightCount)
{
	SetPtr left, middle, right;
	int l,r;
	SetPtr trees = newCopySet(s);
	if (firstVar > 1 && firstVar <= maxVariable)
		unionToSet (trees, allSet[0][firstVar]);
	if (setContainsFalse (trees))
	{
		*leftCount = 0;
		*rightCount = 0;
	}
	else if (firstVar == lastVar || firstVar > maxVariable)
	{
		*leftCount = 1;
		*rightCount = 1;
	}
	else
	{
		left = newSet();
		middle = newSet();
		right = newSet();
		fillNextTernarySets (firstVar-1, trees, left, right, middle, &l,&r, NULL,NULL);
		satCount (left, firstVar+1, lastVar, &l, &r);
		*leftCount = l + r;
		satCount (right, firstVar+1, lastVar, &l, &r);
		*rightCount = l + r;
		freeSet (left);
		freeSet (middle);
		freeSet (right);
	}
	freeSet (trees);
}

int bothSatCount (SetPtr s, int firstVar, int lastVar)
{
	int l,r;
	
	satCount (s, firstVar, lastVar, &l, &r);
	return l+r;
}

int chooseLeftFirst (SetPtr s)
{
	int i;
	TernaryTreePtr node;
	double resultl=maxVariable+1, resultr=maxVariable+1;
	int countl=1, countr=1;
	
	for (i=0;i<s->numOfElements;i++)
	{
		node = s->elements[i];
	//printf ("w %e %e\n", node->left->weight,node->right->weight);
	
		if (node->left->isFalsifiable) countl++;
		if (node->right->isFalsifiable) countr++;
		
			if (node->left->weight < resultl)
			{
				resultl += node->left->weight;
			}
			if (node->right->weight < resultr)
			{
				resultr += node->right->weight;
			}
	}
	//printf ("%e %e\n", resultl,resultr);
	if (countl == 1) return true;
	if (countr == 1) return false;
	return resultl < resultr;
}

void newBothCoverage (SetPtr trees, SetPtr toBeCovered, SetPtr cover)
{
	int i;
	TernaryTreePtr node;
	clearSet (cover);
	for (i=0;i<trees->numOfElements && toBeCovered->numOfElements > 0; i++)
	{
		node = trees->elements[i];
		if (setMemberOfRemoveU(toBeCovered, node->both))
			addToSet (cover, node);
	}
	unionToSet (cover, toBeCovered);
}


int newTryTernaryRec (int all, int tid, AssignmentPtr answer, int firstVar, int goneLeft, SetPtr falseSet, int *add,long int *resId)
{
	TernaryTreePtr node;
	int result, leftResult, rightResult, i, m1, m2, m3, m4, addLeftRight;
	int leftRun, rightRun;
	SetPtr trees, leftFalse, rightFalse, sub1, sub2;
	int leftCovered, rightCovered, leftFirst;
	int leftCount, rightCount;
	double leftWeight;
	int addFalseSet;
	unsigned long leftResId, rightResId, setId1, setId2;
	int resDone;

#ifdef THREADS
	if (answerFoundSet != 0)
		pthread_exit(0);
#endif

	
	//if (firstVar > 1 && firstVar <= 6) printf ("\nXX %d->%d\n", firstVar-1, answer->values[firstVar-1]);


	if (tid == -1)
	{
		printf ("v = %d   ", firstVar);
	for (i=1;i<firstVar;i++)
		if (answer->values[i]==1)
			fprintf (stdout, "%d ", i);
		else
			fprintf (stdout, "%d ", -i);
			
	fprintf (stdout,"\n");
	}
	
	if ((tid == 0) && (firstVar < 7 || tries%1000 == 0) )
			logProgress(firstVar-1);

	if (tid == 0)
		tries++;
	


	if (firstVar > maxVariable)
	{
		setAnswerFound (tid, answer);
		//printf ("c sat\n");
		return true;
	}

	//printf ("all "); printSet (all); printf ("\n");
	//printf ("first var = %d ", firstVar);

	trees = allSet[tid][firstVar];
	

	if (newFalseFound)
	{
		clearSet (falseSet);
		//printf ("ret %d\n", firstVar);
		return false;
	}

	leftResult = true;
	rightResult = true;
	clearSet (falseSet);
	addLeftRight = true;

	result = true;
	
	
	leftFalse = nextLeft[tid][firstVar];
	rightFalse = nextRight[tid][firstVar];
	clearSet (leftFalse);
	clearSet (rightFalse);

	//printSet (trees); printf ("\n");

	clearSet (leftSet[tid][firstVar]);
	clearSet (middleSet[tid][firstVar]);
	clearSet (rightSet[tid][firstVar]);

	clearSet (falseSet);

	leftResult = true;
	rightResult = true;

	resDone = false;

	//printAllSets(tid,firstVar);
	if (true)
	{
		sub1 = checkForSubset (tid, falseSet, middleSet[tid][firstVar-1], firstVar, &setId1);
	//sub1 = NULL;
		if (goneLeft)
			sub2 = checkForSubset (tid, falseSet, leftSet[tid][firstVar-1], firstVar, &setId2);
		else
			sub2 = checkForSubset (tid, falseSet, rightSet[tid][firstVar-1], firstVar, &setId2);
	
		if (sub1 != NULL || sub2 != NULL)
		{
			if (sub1 == NULL)
			{
#ifdef PROOF
				*resId = setId2;
#endif
				copySet (falseSet, sub2);
				freeSet (sub2);
			}
			else if (sub2 == NULL)
			{
#ifdef PROOF
				*resId = setId1;
#endif
				copySet (falseSet, sub1);
				freeSet (sub1);
			}
			else
			{
				if (sub1->numOfElements < sub2->numOfElements)
				{
#ifdef PROOF
					*resId = setId1;
#endif
					copySet (falseSet, sub1);
				}
				else
				{
#ifdef PROOF
					*resId = setId2;
#endif
					copySet (falseSet, sub2);
				}
				freeSet (sub1);
				freeSet (sub2);
			}
	
			

			if (tid == 0)
				improvements++;
			result = false;
		}
	}
	if (!result)
	{
		//printf ("found "); printSet (falseSet); printf (" %d\n", *resId);
		return false;
	}
	
	for (i=0;i<trees->numOfElements;i++)
		( (TernaryTreePtr) trees->elements[i])->isSelected[tid] = false;
  

	result = true;
	leftResult = true;
	rightResult = true;
	leftRun = false;
	rightRun = false;

	leftCovered = false;
	rightCovered = false;


	leftCount = 0;
	rightCount = 0;
	leftWeight = 0;
	
	*resId = 0;
	clearSet (falseSet);
	
	for (i=0;result && i<trees->numOfElements; i++)
	{
		node = trees->elements[i];
		if (node->left->isFalsifiable && !node->left->isSelected[tid])
		{
				leftCount++;
		}
		if (node->right->isFalsifiable && !node->right->isSelected[tid])
		{
				rightCount++;
		}
		if (!node->both->isSatisfiable)
		{
#ifdef PROOF
			*resId = node->both->resId;
			resDone = true;
#endif
			if (!node->isSpine)
				addToSetNoCheck (falseSet, node);
			leftCovered = true;
			rightCovered = true;
			result = false;
			*add = false;
		}
	}
	for (i=0;result && i<trees->numOfElements; i++)
	{
		node = trees->elements[i];
		if (!tryToAdd (tid, firstVar, node->both, middleSet[tid][firstVar], true, leftFalse))
		{
			result = false;
			newBothCoverage (trees, leftFalse, falseSet);
		}
	}
	if (result)
	{
	//printf ("V%d?", firstVar); printSet (middleSet[tid][firstVar]);printf ("\n");

		if (leftCount == 0)
			leftFirst = true;              
		else if (rightCount == 0)
			leftFirst = false;
		else if (tid == 0)
		{
			leftFirst = chooseLeftFirst (trees);
			//leftFirst = leftCount < rightCount;
			//printf ("%d %d\n", leftCount, rightCount);
		}
		else
			leftFirst = rand() % 2 == 0;
			
		if (leftFirst)
		{
			for (i=0;leftResult && i<trees->numOfElements;i++)
			{
				node = trees->elements[i];
				if (!node->left->isSatisfiable)
				{
#ifdef PROOF
					leftResId = node->left->resId;
#endif
					if (!node->isSpine)
						addToSetNoCheck (leftFalse, node->left);
					leftCovered = true;
					leftResult = false;
					*add = false;
					//printf ("L covered by %d\n", node);
				}
			}
			for (i=0;leftResult && i<trees->numOfElements;i++)
			{
				node = trees->elements[i];
				leftResult = tryToAdd (tid, firstVar, node->left, leftSet[tid][firstVar], false, leftFalse);
			}
			
			if (leftResult)
			{
				answer->values[firstVar] = 1;
				leftResult = newTryTernaryRec (all, tid, answer, firstVar+1, true, leftFalse, &addFalseSet, &leftResId);

				leftRun = true;
			}


			for (i=0;i<leftSet[tid][firstVar]->numOfElements;i++)
			{
				node = leftSet[tid][firstVar]->elements[i];
				bufferSMRU (tid,allSet[tid][node->firstVar], node);
				//setMemberOfRemoveU (all, node);
				node->isSelected[tid] = false;
			}

			if (!leftResult)
			{
				if ( leftRun && ( allAbove (firstVar, leftFalse, middleSet[tid][firstVar], leftSet[tid][firstVar])))
				{
#ifdef PROOF
					*resId = leftResId;
					rightResId = leftResId;
					resDone = true;
#endif
					rightResult = false;
					 copySet (rightFalse,leftFalse);
					
					//clearSet (rightFalse);
				 }
				else
				{
					for (i=0;rightResult && i < trees->numOfElements; i++)
					{
						node = trees->elements[i];
						if (!node->right->isSatisfiable)
						{
#ifdef PROOF
							rightResId = node->right->resId;
#endif
							if (!node->isSpine)
								addToSetNoCheck (rightFalse, node->right);
							 rightCovered = true;
							//printf ("R covered by %d\n", node);
							rightResult = false;
						}
					}
					for (i=0;rightResult && i < trees->numOfElements; i++)
					{
						node = trees->elements[i];
						rightResult = tryToAdd (tid, firstVar, node->right, rightSet[tid][firstVar], false, rightFalse);
					}
	
					if (rightResult)
					{
						answer->values[firstVar] = 0;
						rightResult = newTryTernaryRec (all, tid, answer, firstVar+1, false, rightFalse, &addFalseSet, &rightResId);
//                        if (!rightResult && (addFalseSet || firstVar % 2 == 0) && (!allAbove (firstVar, rightFalse, middleSet[tid][firstVar], rightSet[tid][firstVar]) || !allOnlyOneParent (rightFalse,middleSet[tid][firstVar])))
//                        {
//                            qsort (rightFalse->elements, rightFalse->numOfElements, sizeof (TernaryTreePtr), ternaryTreeComparatorFirstVar);
//                            newAddFalseSubset (tid,rightFalse);
//                        }
						if (!rightResult && allAbove (firstVar, rightFalse, middleSet[tid][firstVar], rightSet[tid][firstVar]))
						{
#ifdef PROOF
							*resId = rightResId;
							leftResId = rightResId;
							resDone = true;
#endif
							copySet (leftFalse, rightFalse);
						 }
						 else if (!rightResult)
						 {
 
						 }
					}
					for (i=0;i<rightSet[tid][firstVar]->numOfElements;i++)
					{
						node = rightSet[tid][firstVar]->elements[i];
						node->isSelected[tid] = false;
						bufferSMRU (tid,allSet[tid][node->firstVar], node);
					}
				}
				if (!rightResult)
				{
					//printf ("L="); printSet (leftFalse); printf ("R="); printSet (rightFalse);printf ("F="); printSet(falseSet);
					for (i=0;i<trees->numOfElements;i++)
					{
						node = trees->elements[i];
						node->inLeft[tid] = false;
						node->inRight[tid] = false;
						node->left->inLeft[tid] = false;
						node->left->inRight[tid] = false;
						node->both->inLeft[tid] = false;
						node->both->inRight[tid] = false;
						node->right->inLeft[tid] = false;
						node->right->inRight[tid] = false;
					}
					for (i=0;i<leftFalse->numOfElements;i++)
					{
						node = leftFalse->elements[i];
						node->inLeft[tid] = true;
					}
					for (i=0;i<rightFalse->numOfElements;i++)
					{
						node = rightFalse->elements[i];
						node->inRight[tid] = true;
					}
					if (spine[firstVar] != NULL)
					{
						node = spine[firstVar];
						node->inLeft[tid] = false;
						node->inRight[tid] = false;
						node->left->inLeft[tid] = false;
						node->both->inLeft[tid] = false;
						node->right->inRight[tid] = false;
						node->both->inRight[tid] = false;
					}
					//printSet (falseSet);printSet(leftFalse);printSet(rightFalse);
					for (i=0;i<falseSet->numOfElements;i++)
					{
						node = falseSet->elements[i];
						node->inLeft[tid] = false;
						node->inRight[tid] = false;
						node->left->inLeft[tid] = false;
						node->both->inLeft[tid] = false;
						node->right->inRight[tid] = false;
						node->both->inRight[tid] = false;
					}
					for (i=0;i<leftFalse->numOfElements;i++)
					{
						node = leftFalse->elements[i];
						if (node->inLeft[tid] && node->isSatisfiable && !setMemberOfU (middleSet[tid][firstVar], node) && !setMemberOfU (leftSet[tid][firstVar], node))
						{
							//printf ("L");
							node->inLeft[tid] = false;
							node->inRight[tid] = false;
							if (!node->isSpine)
								addToSetNoCheck (falseSet, node);
						}
					}
					//printf ("V%d", firstVar);printSet (rightFalse);printSet(middleSet[firstVar]);printSet(rightSet[firstVar]);printf("|");
					for (i=0;i<rightFalse->numOfElements;i++)
					{
						node = rightFalse->elements[i];
						//printf("[%d]", node);
						if (node->inRight[tid] && node->isSatisfiable && !setMemberOfU (middleSet[tid][firstVar], node) && !setMemberOfU (rightSet[tid][firstVar], node))
						{
							//printf ("R(%d)", node);
							node->inLeft[tid] = false;
							node->inRight[tid] = false;
							if (!node->isSpine)
								addToSetNoCheck (falseSet, node);
						}
					}
					for (i=0;i<trees->numOfElements;i++)
					{
						node = trees->elements[i];
						if (node->both->inRight[tid] || node->both->inLeft[tid])
						{
							node->both->inLeft[tid] = false;
							node->both->inRight[tid] = false;
							node->left->inLeft[tid] = false;
							node->right->inRight[tid] = false;
							if (!node->isSpine)
								addToSetNoCheck (falseSet, node);
						}
					}
					for (i=0;i<trees->numOfElements;i++)
					{
						node = trees->elements[i];
						if (node->left->inLeft[tid] && node->right->inRight[tid])
						{
							node->left->inLeft[tid] = false;
							node->right->inRight[tid] = false;
							node->both->inLeft[tid] = false;
							node->both->inRight[tid] = false;
							if ((!node->isSpine))
								addToSetNoCheck (falseSet, node);
						}
					}
					for (i=0;i<trees->numOfElements;i++)
					{
						node = trees->elements[i];
						m1 = node->right->inRight[tid]; node->right->inRight[tid] = false;
						m2 = node->both->inRight[tid]; node->both->inRight[tid] = false;
						m3 = node->left->inLeft[tid]; node->left->inLeft[tid] = false;
						m4 = node->both->inLeft[tid]; node->both->inLeft[tid] = false;
						//printf ("n=%d(%d,%d)\n", node, m1,m2);
						if ((!node->isSpine) && (m1 || m3 || m2 || m4))
							addToSetNoCheck (falseSet, node);
					}
	
				}
	
			}
		}
		else
		{
			for (i=0;rightResult && i<trees->numOfElements;i++)
			{
				node = trees->elements[i];
				if (!node->right->isSatisfiable)
				{
#ifdef PROOF
					rightResId = node->right->resId;
#endif
					if (!node->isSpine)
						addToSetNoCheck (rightFalse, node->right);
					rightCovered = true;
					rightResult = false;
					*add = false;
					//printf ("L covered by %d\n", node);
				}
			}
			for (i=0;rightResult && i<trees->numOfElements;i++)
			{
				node = trees->elements[i];
				rightResult = tryToAdd (tid, firstVar, node->right, rightSet[tid][firstVar], false, rightFalse);
			}

			if (rightResult)
			{
				answer->values[firstVar] = 0;
				rightResult = newTryTernaryRec (all, tid, answer, firstVar+1, false, rightFalse, &addFalseSet, &rightResId);

				rightRun = true;
			}


			for (i=0;i<rightSet[tid][firstVar]->numOfElements;i++)
			{
				node = rightSet[tid][firstVar]->elements[i];
				bufferSMRU (tid,allSet[tid][node->firstVar], node);
				//setMemberOfRemoveU (all, node);
				node->isSelected[tid] = false;
			}

		if (!rightResult)
		{
			if ( rightRun && (allAbove (firstVar, rightFalse, middleSet[tid][firstVar], rightSet[tid][firstVar])))
			{
#ifdef PROOF
				*resId = rightResId;
				leftResId = rightResId;
				resDone = true;
#endif
				leftResult = false;
				copySet (leftFalse, rightFalse);
				//clearSet (leftFalse);
			 }
			else
			{
				for (i=0;leftResult && i<trees->numOfElements;i++)
				{
					node = trees->elements[i];
					if (!node->left->isSatisfiable)
					{
#ifdef PROOF
						leftResId = node->left->resId;
#endif
						if (!node->isSpine)
							addToSetNoCheck (leftFalse, node->left);
						leftCovered = true;
						//printf ("R covered by %d\n", node);
						leftResult = false;
						*add = false;
					}
				}
				for (i=0;leftResult && i<trees->numOfElements;i++)
				{
					node = trees->elements[i];
					leftResult = tryToAdd (tid, firstVar, node->left, leftSet[tid][firstVar], false, leftFalse);
				}

				if (leftResult)
				{
					answer->values[firstVar] = 1;
					leftResult = newTryTernaryRec (all, tid, answer, firstVar+1, true, leftFalse, &addFalseSet, &leftResId);

					if (!leftResult && allAbove (firstVar, leftFalse, middleSet[tid][firstVar], leftSet[tid][firstVar]))
					{
#ifdef PROOF
						*resId = leftResId;
						rightResId = leftResId;
						resDone = true;
#endif
						copySet (rightFalse, leftFalse);
						//clearSet (rightFalse);
					 }
					 else if (!leftResult);
					 {

					 }
				}
				for (i=0;i<leftSet[tid][firstVar]->numOfElements;i++)
				{
					node = leftSet[tid][firstVar]->elements[i];
					node->isSelected[tid] = false;
					bufferSMRU (tid,allSet[tid][node->firstVar], node);
				}
			}
			if (!leftResult)
			{
				//printSet (falseSet);printSet(rightFalse);printSet(leftFalse);
				//printf ("L="); printSet (leftFalse); printf ("R="); printSet (rightFalse);printf ("F="); printSet(falseSet);
				for (i=0;i<trees->numOfElements;i++)
				{
					node = trees->elements[i];
					node->inLeft[tid] = false;
					node->inRight[tid] = false;
					node->left->inLeft[tid] = false;
					node->left->inRight[tid] = false;
					node->both->inLeft[tid] = false;
					node->both->inRight[tid] = false;
					node->right->inLeft[tid] = false;
					node->right->inRight[tid] = false;
				}
				for (i=0;i<leftFalse->numOfElements;i++)
				{
					node = leftFalse->elements[i];
					node->inLeft[tid] = true;
				}
				for (i=0;i<rightFalse->numOfElements;i++)
				{
					node = rightFalse->elements[i];
					node->inRight[tid] = true;
				}
				if (spine[firstVar] != NULL)
				{
					node = spine[firstVar];
					node->inLeft[tid] = false;
					node->inRight[tid] = false;
					node->left->inLeft[tid] = false;
					node->both->inLeft[tid] = false;
					node->right->inRight[tid] = false;
					node->both->inRight[tid] = false;
				}
				//printSet (falseSet);printSet(leftFalse);printSet(rightFalse);
				for (i=0;i<falseSet->numOfElements;i++)
				{
					node = falseSet->elements[i];
					node->inLeft[tid] = false;
					node->inRight[tid] = false;
					node->left->inLeft[tid] = false;
					node->both->inLeft[tid] = false;
					node->right->inRight[tid] = false;
					node->both->inRight[tid] = false;
				}
				for (i=0;i<leftFalse->numOfElements;i++)
				{
					node = leftFalse->elements[i];
					if (node->inLeft[tid] && node->isSatisfiable && !setMemberOfU (middleSet[tid][firstVar], node) && !setMemberOfU (leftSet[tid][firstVar], node))
					{
						//printf ("L");
						node->inLeft[tid] = false;
						node->inRight[tid] = false;
						if (!node->isSpine)
							addToSetNoCheck (falseSet, node);
					}
				}
				//printf ("V%d", firstVar);printSet (rightFalse);printSet(middleSet[firstVar]);printSet(rightSet[firstVar]);printf("|");
				for (i=0;i<rightFalse->numOfElements;i++)
				{
					node = rightFalse->elements[i];
					//printf("[%d]", node);
					if (node->inRight[tid] && node->isSatisfiable && !setMemberOfU (middleSet[tid][firstVar], node) && !setMemberOfU (rightSet[tid][firstVar], node))
					{
						//printf ("R(%d)", node);
						node->inLeft[tid] = false;
						node->inRight[tid] = false;
						if (!node->isSpine)
							addToSetNoCheck (falseSet, node);
					}
				}
				for (i=0;i<trees->numOfElements;i++)
				{
					node = trees->elements[i];
					if (node->both->inRight[tid] || node->both->inLeft[tid])
					{
						node->both->inLeft[tid] = false;
						node->both->inRight[tid] = false;
						node->left->inLeft[tid] = false;
						node->right->inRight[tid] = false;
						if (!node->isSpine)
							addToSetNoCheck (falseSet, node);
					}
				}
				for (i=0;i<trees->numOfElements;i++)
				{
					node = trees->elements[i];
					if (node->left->inLeft[tid] && node->right->inRight[tid])
					{
						node->left->inLeft[tid] = false;
						node->right->inRight[tid] = false;
						node->both->inLeft[tid] = false;
						node->both->inRight[tid] = false;
						if ((!node->isSpine))
							addToSetNoCheck (falseSet, node);
					}
				}
					for (i=0;i<trees->numOfElements;i++)
				{
					node = trees->elements[i];
					m1 = node->right->inRight[tid]; node->right->inRight[tid] = false;
					m2 = node->both->inRight[tid]; node->both->inRight[tid] = false;
					m3 = node->left->inLeft[tid]; node->left->inLeft[tid] = false;
					m4 = node->both->inLeft[tid]; node->both->inLeft[tid] = false;
					//printf ("n=%d(%d,%d)\n", node, m1,m2);
					if ((!node->isSpine) && (m1 || m3 || m2 || m4))
						addToSetNoCheck (falseSet, node);
				}
			}
		}
	}
	}

	for (i=0;i<middleSet[tid][firstVar]->numOfElements;i++)
	{
		node = middleSet[tid][firstVar]->elements[i];
		node->isSelected[tid] = false;
		bufferSMRU (tid,allSet[tid][node->firstVar], node);
		//setMemberOfRemoveU (all, node);
	}

	if (result)
		result = leftResult || rightResult;

	//printf ("<%d>", falseSet->numOfElements);
	if (true)
	{
		removeDuplicatesSet (falseSet);
		sortSet (leftFalse); sortSet (rightFalse);

		if (falseSet->numOfElements > 0 && (equalSortedSet (falseSet,leftFalse) ||
			equalSortedSet (falseSet, rightFalse)))
		{
			if (equalSortedSet (falseSet,leftFalse))
				*resId = leftResId;
			else
				*resId = rightResId;
		}
		else

		{
#ifdef PROOF
			if (!resDone)
			{
				*resId = nextResId++;
				resolveProofClauses (globalUnTransform[firstVar],leftResId, rightResId, *resId);
//                copyIntSet(idVars[*resId], idVars[leftResId]);
//                unionToIntSet (idVars[*resId], idVars[rightResId]);
//                intSetMemberOfRemoveU (idVars[*resId], firstVar);
//                intSetMemberOfRemoveU (idVars[*resId], -firstVar);
				//printf ("<%d>(%d,%d) %d-> ", firstVar, leftResId, rightResId, *resId); printIntSet(idVars[leftResId]);printIntSet(idVars[rightResId]);printIntSet(idVars[*resId]); printf ("\n");
			}
#endif
//            printf ("Adding ");
//            printSet (falseSet);
//            printf ("\n");
			// (" <%d>\n", *resId);
			newAddFalseSubset (tid,falseSet, *resId);
			

		}
	}

		//printf ("return %d (%d %d %d)\n", firstVar, leftResId, rightResId, *resId);
	for (i=0;i<trees->numOfElements;i++)
		( (TernaryTreePtr) trees->elements[i])->isSelected[tid] = true;

	return result;
}

void setSpine (TernaryTreePtr root)
{
	while (root->isSatisfiable && root->isFalsifiable)
	{
		spine[root->firstVar] = root;
		root->isSpine = true;
		root = root->both;
	}
}
int newTryTernary (int tid, AssignmentPtr assignment, TernaryTreePtr root)
{
	int result,v1;
	
	long int resId;
	
	srand(tid);
	all = newSet();
	newFalseFound = false;
	largestSetSize = intOptions[INITIAL_SET_SIZE];
	increaseRequests = 0;
	doLogging = false;

 

	doLogging = true;

		clearSet (allSet[tid][root->firstVar]);
		addToSet (allSet[tid][root->firstVar], root);
		result = newTryTernaryRec (true, tid, assignment, root->firstVar, 0, newSet(), &v1, &resId);
		 printf ("c Thread %d result %d\n", tid, result); fflush (stdout);

	setAnswerFound (tid,NULL);
	
	return result;
}

void stopSavingFalseSets()
{
	largestSetSize = 0;
}

int buildAndTryTernary (AssignmentPtr assignment,int level, SetPtr ternaryTrees, SetPtr falseSet, int *falseId, int *resId)

{
	int allTrue, anyFalse, l, result, firstDirection, secondDirection;
	SetPtr subset;
	SetPtr firstNext, secondNext;
	SetPtr firstSet, secondSet;

	int leftSimpler, rightSimpler, firstSimpler, secondSimpler, firstRun=false, secondRun=false;
	

	TernaryTreePtr node;
	int proofChoice;
	int thisId,firstId, secondId, firstResId,secondResId;
	
#ifdef PROOF
	int leftId, rightId, leftResId, rightResId;
#endif

	SetPtr middles;



	//printf (" --------------------- level %d \n", level);


	tries++;

	allTrue = true;
	anyFalse = false;



	for (l=0; l<ternaryTrees->numOfElements && !anyFalse;l++)
	{
		node = ternaryTrees->elements[l];

		if (!node->isSatisfiable)
		{
			addToSet (falseSet, ternaryTrees->elements[l]);
#ifdef PROOF
			*resId = node->resId;
#endif
			anyFalse = true;
		}
		else if (node->isFalsifiable)
			allTrue = false;
	}
	if (anyFalse)
	{
		*falseId = 0;
		
		return false;
	}
	else if (allTrue)
	{
		return true;
	}
	else
	{
		clearSet (falseSet);
#ifdef PROOF
		if (booleanOptions[RPT_FILE])
		{
			copySet (tryCopySet, ternaryTrees);
			sortSet (tryCopySet);
			subset = findFalseSubset (tryCopySet, &thisId);
		}
		else
		{
			subset = findFalseSubset (ternaryTrees, &thisId);
		}
#else
		copyNonTrunk (tryCopySet, ternaryTrees);
		subset = findFalseSubset (tryCopySet, &thisId);
#endif


		if (subset != NULL)
		{

			*falseId = thisId;

#ifdef PROOF
			if (booleanOptions[RPT_FILE])
				*resId = setIdToResId[thisId];
#endif

			for (l=0;l<subset->numOfElements;l++)
			{
				addToSetNoCheck (falseSet, subset->elements[l]);
			}

			improvements++;
			freeSet (subset);

			if (stringOptions[DOT_FILE] != NULL)
				dotFileLine (assignment, level, 0);


			return false;
		}


		result = true;

		clearSet (nextFalseSet[0][level]);
		clearSet (nextLeft[0][level]);
		clearSet (nextRight[0][level]);
		clearSet (leftSet[0][level]);
		clearSet (rightSet[0][level]);

		middles = middleSet[0][level];
		clearSet (middles);

		if (tries == 203 && level==49)
		{
			printf ("here\n");
		}

		fillNextTernarySets (level, ternaryTrees, leftSet[0][level], rightSet[0][level], middles, &leftSimpler, &rightSimpler,
								posLiterals, negLiterals);

		leftSimpler = rightSet[0][level]->numOfElements > middles->numOfElements;
		rightSimpler = leftSet[0][level]->numOfElements > middles->numOfElements;
		
		if (tries == 203 && level==49)
		{
			printSet(leftSet[0][level]); printSet(rightSet[0][level]);
		}    
		firstDirection = false;
		if (leftSet[0][level]->numOfElements < rightSet[0][level]->numOfElements)
			firstDirection = true;

		if (leftSet[0][level]->numOfElements < rightSet[0][level]->numOfElements && rightSimpler && ! leftSimpler)
		{
			printf ("y"); exit(0);
		}
		if (leftSet[0][level]->numOfElements > rightSet[0][level]->numOfElements && !rightSimpler && leftSimpler)
		{
			printf ("z"); exit(0);
		}
		if (firstDirection)
		{
			firstSet = leftSet[0][level];
			firstDirection = true;
			firstNext = nextLeft[0][level];
			secondNext = nextRight[0][level];
			firstSimpler = leftSimpler;
			secondSimpler = rightSimpler;
		}
		else
		{
			firstSet = rightSet[0][level];
			firstDirection = false;
			firstNext = nextRight[0][level];
			secondNext = nextLeft[0][level];
			firstSimpler = rightSimpler;
			secondSimpler = leftSimpler;
	   }



		if (firstSet == leftSet[0][level])
			secondSet = rightSet[0][level];
		else
			secondSet = leftSet[0][level];

		secondDirection = !firstDirection;
		clearSet (firstNext);
		clearSet (secondNext);

		//printf ("Direction = %s\n", firstDirection?"left":"right");


		 if ( (firstSimpler))
		{
			setAssignmentValue (assignment, level+1,firstDirection);
			result = buildAndTryTernary (assignment, level+1, firstSet, firstNext, &firstId, &firstResId);


			firstRun = true;
		}
		else
			result = false;


		if (!result)
		{
			if (!firstRun ||
				(( secondSimpler) && ! (firstRun && subsetSortedSet (firstNext, middles) )
				)
			 )
			{
				setAssignmentValue (assignment, level+1,secondDirection);

				result = buildAndTryTernary (assignment, level+1, secondSet, secondNext, &secondId,&secondResId);
				secondRun = true;



			}
			if (!result)
			{
			
#ifdef PROOF
					if (booleanOptions[RPT_FILE])
					{
						copySet (sortedFirstNext, firstNext);
						sortSet (sortedFirstNext);
						copySet (sortedSecondNext, secondNext);
						sortSet (sortedSecondNext);
						copySet (sortedMiddles, middles);
						sortSet (sortedMiddles);
					}
					else
					{
						sortedFirstNext = firstNext;
						sortedSecondNext = secondNext;
						sortedMiddles = middles;
					}
#else
					{
						sortedFirstNext = firstNext;
						sortedSecondNext = secondNext;
						sortedMiddles = middles;
					}
#endif


					if ( !secondRun  || (firstRun && subsetSortedSet (sortedFirstNext, sortedMiddles) ) )
					{
						proofChoice = 1;
					}
					else if ( !firstRun || (secondRun && subsetSortedSet (sortedSecondNext, sortedMiddles) ))
					{
						proofChoice = 2;
					}
					else

						proofChoice = 3;


					if (!firstRun)
						clearSet (firstNext);
					else if (!secondRun)
						clearSet (secondNext);


					if (proofChoice == 3)
					{
/*
						if (firstRun && subsetSortedSet (firstNext, secondNext))
						{
							copySet (secondNext,firstNext);
						}
						else if (secondRun && subsetSortedSet (secondNext, firstNext))
						{
							copySet (firstNext,secondNext);
						}
*/                      
						
						addTwinCoverage (level, nextLeft[0][level], nextRight[0][level], ternaryTrees, falseSet);
#ifdef PROOF
						//printProofClause (assignment, falseSet);
#endif

					}
					else if (proofChoice == 1)
								addBothCoverage (level, firstNext, ternaryTrees, falseSet);
					else
								addBothCoverage (level, secondNext, ternaryTrees, falseSet);





#ifdef PROOF
					if (booleanOptions[RPT_FILE])
					{
						copySet (tryCopySet, falseSet);
						sortSet (tryCopySet);
						thisId = addFalseSetTop (0,tryCopySet);
					}
					else
						thisId = addFalseSetTop (0,falseSet);
 
#else
					copyNonTrunk (tryCopySet, falseSet);
					thisId = addFalseSetTop (0,tryCopySet);
#endif


#ifdef PROOF
					*falseId = thisId;
					switch (proofChoice)
					{
						case 1:
							if (booleanOptions[PRT_FILE])
								printSuper (firstId, thisId, falseSet);
							if (booleanOptions[RPT_FILE])
							{
								*resId = firstResId;
								assignSetIdToResId (thisId, *resId);
							}
							break;

						case 2:
							if (booleanOptions[PRT_FILE])
								printSuper (secondId, thisId, falseSet);
							if (booleanOptions[RPT_FILE])
							{
								*resId = secondResId;
								assignSetIdToResId (thisId, *resId);
							}
							break;
							
						case 3:
							if (firstDirection)
							{
								leftId = firstId;
								rightId = secondId;
								leftResId = firstResId;
								rightResId = secondResId;
							}
							else
							{
								leftId = secondId;
								rightId = firstId;
								leftResId = secondResId;
								rightResId = firstResId;
							}
							if (booleanOptions[PRT_FILE])
								printResolution (level+1,leftId, rightId, thisId, falseSet);
							if (booleanOptions[RPT_FILE])
							{
								*resId = nextResId++;
								assignSetIdToResId (thisId, *resId);
								resolveProofClauses(globalUnTransform[level+1], leftResId, rightResId, *resId);
							}
					}
#endif
			}
		}

	}


	return result;
}




