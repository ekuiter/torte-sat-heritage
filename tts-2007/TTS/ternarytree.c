#include "memutil.h"
#include "ternarytree.h"
#include "list.h"
#include "proposition.h"
#include "tts.h"
#include "set.h"
#include "bag.h"
#include "options.h"
#include "hashtable.h"

#include <stdio.h>
#include <assert.h>

TernaryTreePtr newTernaryTree()
{
	TernaryTreePtr result;
	result = checkMalloc (sizeof(TernaryTree),"TernaryTree");
	result->isFalsifiable = true;
	result->isSatisfiable = true;
	result->left = NULL;
	result->both = NULL;
	result->right = NULL;
	result->subsets = NULL;
	result->clauses = NULL;
	result->numOfClauses = 0;
	result->firstVar = 1<<30;
	result->prevFirstVar = 0;
	result->numOfVars = 0;


#ifdef PROOF
	result->falsifier = -1;
#endif

//    result->ps = malloc(2000);

	return result;
}

char stringClauseList[50000];





int prevFirstVarCompare (void * u1, void * u2)
{
	TernaryTreePtr t1,t2;

	t1 = *((TernaryTreePtr *)u1);
	t2 = *((TernaryTreePtr *)u2);

	if (t1->prevFirstVar > t2->prevFirstVar)
		return -1;
	else if (t1->prevFirstVar < t2->prevFirstVar)
		return 1;
	else
		return 0;
}
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
		result += (i+1)*clauseHashFunction (clauses->elements[i]);
	return result;
}

unsigned long ttHashFunction (void * t)
{
	return clausesHashFunction (((TernaryTreePtr)t)->clauses);
}
int ttHashKeyEqual (void * t1, void * t2)
{
	return equalClauseSets (((TernaryTreePtr)t1)->clauses, ((TernaryTreePtr)t2)->clauses);
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

static SetPtr tryCopySet, *nextFalseSet, *nextLeft, *nextRight;
static SetPtr  *leftSet, *rightSet, *middleSet;

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

#ifdef PROOF
		result = newTernaryTree();
		result->isSatisfiable = false;
		result->isFalsifiable = true;
		result->falsifier = baseProofClauses[p->falsifyingClauseNum];
#else
		result = falseTernaryTree;
#endif
	}
	else
	{
		leftP = propositionCurried(p,variable,true);
		result->left = buildTernaryTree (variable+1, false, leftP);
		bothP = propositionExcluding(p,variable);
		result->both = buildTernaryTree (variable+1, isCentre, bothP);
		rightP = propositionCurried(p,variable,false);
		result->right = buildTernaryTree (variable+1, false, rightP);
		result->leftSimpler =
			result->right->isFalsifiable;
			//(leftP->numOfClauses + bothP->numOfClauses < p->numOfClauses);

		result->rightSimpler =
			result->left->isFalsifiable;
			//(rightP->numOfClauses + bothP->numOfClauses < p->numOfClauses);

	}

	addListLast (&levelLists[variable-1], result);

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




int tries, improvements;
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


	//qsort (test->elements, test->numOfElements, sizeof (TernaryTreePtr), prevFirstVarCompare);

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

		if (node->leftSimpler)
			*leftSimpler = true;
		if (node->rightSimpler)
			*rightSimpler = true;

		if (node->clauses != NULL && node->firstVar > level+1)
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
	removeDuplicatesSet (leftSet);
	removeDuplicatesSet (rightSet);
	removeDuplicatesSet (middleSet);


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
	printf ("%d",(int)t);
	if (t == trueTernaryTree)
		printf ("(T)");
	else if (t == falseTernaryTree)
		printf ("(F)");

	else
		printf ("(%d,%d,%d)<%s>", (int)t->left, (int)t->both, (int)t->right, "" /*t->ps*/);

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
SetPtr *posVarClauseSets, *negVarClauseSets;
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

void insertUnitClauses (SetPtr vars, SetPtr clauses)
{
	int i, v;
	ClausePtr clause;

	for (i=0;i<clauses->numOfElements;i++)
	{
		clause = clauses->elements[i];
		v = clause->positive[0]?clause->variables[0]:-clause->variables[0];
		addToSetNoCheck (vars, (void *)(v+maxVariable));
	}
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

int ternaryTreeComparator (void * t1, void * t2)
{
	ClausePtr c1, c2;
	int v1,v2,p1,p2,result;
	SetPtr s1,s2,ss1,ss2;

	if (booleanOptions[MERGE_CLAUSES])
	{
		s1 = ((TernaryTreePtr)t1)->clauses;
		s2 = ((TernaryTreePtr)t2)->clauses;

		ss1 = newSet();
		ss2 = newSet();
		insertUnitClauses (ss1,s1);
		insertUnitClauses (ss2,s2);

		result = setOrder (ss1,ss2);

		//printSet (ss1); printf (" "); printSet (ss2); printf (" %d\n", result);

		freeSet (ss1);
		freeSet (ss2);


		return result;
	}
	else
	{
		c1 = (ClausePtr) (((TernaryTreePtr)t1)->clauses->elements[0]);
		c2 = (ClausePtr) (((TernaryTreePtr)t2)->clauses->elements[0]);

		v1 = c1->variables[0];
		v2 = c2->variables[0];

		p1 = c1->positive[0];
		p2 = c2->positive[0];
		if (p1 < p2)
			return -1;
		else if (p1 > p2)
			return 1;
		else
		{
			if (v1 < v2)
				return p1?-1:1;
			else if (v1 > v2)
				return p1?1:-1;
			else
				return 0;
		}
	}
}

int ternaryTreeComparator1 (const void ** pt1, const void ** pt2)
{
	TernaryTreePtr t1,t2;
	int n1,n2;

	t1 = *((TernaryTreePtr *)pt1);
	t2 = *((TernaryTreePtr *)pt2);

	if (t1->clauses == NULL)
		return -1;
	else if (t2->clauses == NULL)
		return 1;
	else
	{
		n1 = t1->clauses->numOfElements;
		n2 = t2->clauses->numOfElements;
		if (n1<n2)
			return 1;
		else if (n1 > n2)
			return -1;
		else
			return 0;
	}
}

int ternaryTreeComparator2 (const void ** pt1, const void ** pt2)
{
	TernaryTreePtr t1,t2;

	t1 = *((TernaryTreePtr *)pt1);
	t2 = *((TernaryTreePtr *)pt2);

	if (t1->numOfVars < t2->numOfVars)
		return 1;
	if (t1->numOfVars > t2->numOfVars)
		return -1;
	if (t1->firstVar > t2->firstVar)
		return 1;
	if (t1->firstVar < t2->firstVar)
		return -1;
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

SetPtr *negTrees, *posTrees;


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





int *tempCopyCounts;
int previousTrunkFirstVar;

SetPtr tempCopy;


void initTernaryTrees (PropositionPtr p)
{
	int v,v1,v3;
	int c;
	int maxLevel;
	int minPosVar, minNegVar;

	ClausePtr clause;

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


	//levelListNextId = (int *) checkMalloc ( (1+maxLevel)*sizeof (int *), "ids");
	leftSet = (SetPtr *) checkMalloc ((1+maxLevel)*sizeof(SetPtr),"");
	rightSet = (SetPtr *) checkMalloc ((1+maxLevel)*sizeof(SetPtr),"");
	middleSet = (SetPtr *) checkMalloc ((1+maxLevel)*sizeof(SetPtr),"");

	//negTrees = (SetPtr *) checkMalloc ((1+maxLevel)*sizeof(SetPtr),"");
	//posTrees = (SetPtr *) checkMalloc ((1+maxLevel)*sizeof(SetPtr),"");

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

	for (v=0;v<=maxLevel;v++)
	{
		leftSet[v] = newSet();
		rightSet[v] = newSet();
		middleSet[v] = newSet();
	}

	tryCopySet = newSet();
	nextFalseSet = newSetArray (1+maxLevel);
	nextLeft = newSetArray (1+maxLevel);
	nextRight = newSetArray (1+maxLevel);

	mainStem = newSet();

	for (c=0;c<p->numOfClauses;c++)
	{
		addToSetNoCheck (mainStem, (void *) p->clauses[c]);
	}
	mainStemClauses = mainStem->numOfElements;


	posVarClauseSets = (SetPtr *) checkMalloc ( (1+p->maxVariable)*sizeof(SetPtr),"sets");
	negVarClauseSets = (SetPtr *) checkMalloc ( (1+p->maxVariable)*sizeof(SetPtr),"sets");

	posVarClauseCounts = (int *) checkMalloc ( (1+p->maxVariable)*sizeof(int),"sets");
	negVarClauseCounts = (int *) checkMalloc ( (1+p->maxVariable)*sizeof(int),"sets");

	tempCopyCounts = (int *) checkMalloc ( (1+p->numOfClauses)*sizeof(int),"sets");

	for (v=1; v <= p->maxVariable; v++)
	{
		posVarClauseSets[v] = newSet();
		negVarClauseSets[v] = newSet();
		posVarClauseCounts[v] = 0;
		negVarClauseCounts[v] = 0;
	}

	for (c=0;c<p->numOfClauses;c++)
	{
		clause = p->clauses[c];
		tempCopyCounts[c] = clause->numOfVariables;
		v3 = minVariable(clause);
		minPosVar = p->maxVariable+1;
		minNegVar = p->maxVariable+1;
		for (v=0;v<clause->numOfVariables;v++)
		{
			v1 = clause->variables[v];
			if (clause->positive[v])
			{
				addToSetNoCheck (posVarClauseSets[v1], (void *) c);
				if (v1<minPosVar)
					minPosVar = v1;
			}
			else
			{
				addToSetNoCheck (negVarClauseSets[v1], (void *) c);
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
	, int *clauseNum
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
		*clauseNum = clause->realClauseNum;
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
	int result=true;

	if (clauses->numOfElements > 4)
		return false;

	ClausePtr clause;
	int i;
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
		if (( (TernaryTreePtr) in->elements[i])->clauses != NULL)
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




int clauseSetHasVar (SetPtr s)
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

TernaryTreePtr ternaryTreeFromProposition (SetPtr clauses)
{
	TernaryTreePtr result;
	int newEntry;


#ifdef PROOF
	int falsifier;
#endif

	//printf ("ttfp level(%d)", level);pClauseSet (clauses);

	if (thesePosLiterals == NULL)
	{
		thesePosLiterals = newSet();
		theseNegLiterals = newSet();
	}

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
				result->clauses = clauses;
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
												, & falsifier
#endif
												))
	{
		//freeSet (clauses);
#ifdef PROOF
		result = newTernaryTree();
		result->isSatisfiable = false;
		result->isFalsifiable = true;
		result->falsifier = baseProofClauses[falsifier];
		result->id = levelListNextId[0]++;
#else
		result = falseTernaryTree;
#endif
		//return result;
	}
	else
	{


#ifndef PROOF
		if (false && clauses->numOfElements == 1 && ((ClausePtr)clauses->elements[0])->numOfVariables == 1)
		{
			//result =  unitClauseTree (level, clauses);
			//return result;
		}
		else
#endif

		{
			sortClauseSet (clauses);
			ttHashEntry->clauses = clauses;
			result = hashTableSearchInsert (ttHashTable, ttHashEntry);
/*            for (c=0;c<clauses->numOfElements;c++)
				printf ("%s",toStringClause(clauses->elements[c]));
*/
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
			}

			//return result;
		}
	}
	if (clauses != NULL)
		result->firstVar = clauseSetFirstVar (clauses);



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

void assignTernaryTreeChildren (int variable, TernaryTreePtr node)
{
	int c,c1, direction;
	SetPtr posClauses, negClauses, leftClauses, bothClauses, rightClauses;
	SetPtr clauses;
	ClausePtr clause;

	clauses = node->clauses;

	if (!node->isSatisfiable || !node->isFalsifiable)
		return;

	if (clauses == NULL)
	{
		leftClauses = newSet();
		negClauses = negVarClauseSets[variable];
		for (c=0; c<negClauses->numOfElements;c++)
		{
			c1 = (int)negClauses->elements[c];
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

		node->left = ternaryTreeFromProposition (leftClauses);
		node->both = ternaryTreeFromProposition (NULL);
		node->right = ternaryTreeFromProposition (rightClauses);
		node->leftSimpler = node->right->isFalsifiable;
		node->rightSimpler = node->left->isFalsifiable;

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
		node->left = ternaryTreeFromProposition (leftClauses);
		node->both = ternaryTreeFromProposition (bothClauses);
		node->right = ternaryTreeFromProposition (rightClauses);
		node->leftSimpler = node->right != trueTernaryTree;
		node->rightSimpler = node->left != trueTernaryTree;

		//freeSet (clauses);

	}

	if (node->clauses == NULL && node->isSatisfiable && node->isFalsifiable)
	{
		node->prevFirstVar = variable;
	}
	else
	{
		if (node->left->prevFirstVar < node->prevFirstVar)
			node->left->prevFirstVar = node->prevFirstVar;
		if (node->right->prevFirstVar < node->prevFirstVar)
			node->right->prevFirstVar = node->prevFirstVar;
		if (node->both->prevFirstVar < node->prevFirstVar)
			node->both->prevFirstVar = node->prevFirstVar;
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


TernaryTreePtr populateTernaryTree (int trunkFirstVar, TernaryTreePtr t)
{
	if (t->both == NULL)
	{
		if (t->clauses == NULL)
			t->firstVar = trunkFirstVar;
		assignTernaryTreeChildren (t->firstVar, t);
		t->left = populateTernaryTree (trunkFirstVar+1, t->left);
		t->both = populateTernaryTree (trunkFirstVar+1, t->both);
		t->right = populateTernaryTree (trunkFirstVar+1, t->right);
		if (!t->left->isSatisfiable && !t->right->isSatisfiable)
		{
			return falseTernaryTree;
		}
		else if (!t->left->isFalsifiable && !t->right->isFalsifiable && !t->both->isFalsifiable)
		{
			return trueTernaryTree;
		}
	}
	return t;
}

TernaryTreePtr  buildAllTernary (SetPtr clauses)
{
	TernaryTreePtr result;


	result = ternaryTreeFromProposition (clauses);
	result = populateTernaryTree (1,result);
	return result;
}




int buildAndTryTernary (AssignmentPtr assignment,int level, SetPtr ternaryTrees, SetPtr falseSet)

{
	int allTrue, anyFalse, l, result, firstDirection, secondDirection;
	SetPtr subset;
	SetPtr firstNext, secondNext;
	SetPtr firstSet, secondSet;

	int leftSimpler, rightSimpler, firstSimpler, secondSimpler, firstRun=false, secondRun=false;
	

	TernaryTreePtr node;
	int proofChoice;
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
			anyFalse = true;
		}
		else if (node->isFalsifiable)
			allTrue = false;
	}
	if (anyFalse)
	{
		return false;
	}
	else if (allTrue)
	{
		return true;
	}
	else
	{
		copyNonTrunk (tryCopySet, ternaryTrees);
		clearSet (falseSet);
		subset = findFalseSubset (tryCopySet);

		if (subset != NULL)
		{


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

		clearSet (nextFalseSet[level]);
		clearSet (nextLeft[level]);
		clearSet (nextRight[level]);
		clearSet (leftSet[level]);
		clearSet (rightSet[level]);

		middles = middleSet[level];
		clearSet (middles);


		fillNextTernarySets (level, ternaryTrees, leftSet[level], rightSet[level], middles, &leftSimpler, &rightSimpler,
								posLiterals, negLiterals);

		firstDirection = false;
		if (leftSet[level]->numOfElements < rightSet[level]->numOfElements)
			firstDirection = true;

		if (firstDirection)
		{
			firstSet = leftSet[level];
			firstDirection = true;
			firstNext = nextLeft[level];
			secondNext = nextRight[level];
			firstSimpler = leftSimpler;
			secondSimpler = rightSimpler;
		}
		else
		{
			firstSet = rightSet[level];
			firstDirection = false;
			firstNext = nextRight[level];
			secondNext = nextLeft[level];
			firstSimpler = rightSimpler;
			secondSimpler = leftSimpler;
	   }



		if (firstSet == leftSet[level])
			secondSet = rightSet[level];
		else
			secondSet = leftSet[level];

		secondDirection = !firstDirection;
		clearSet (firstNext);
		clearSet (secondNext);

		//printf ("Direction = %s\n", firstDirection?"left":"right");



		if ( (firstSimpler))
		{
			setAssignmentValue (assignment, level+1,firstDirection);
			result = buildAndTryTernary (assignment, level+1, firstSet, firstNext);
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
				result = buildAndTryTernary (assignment, level+1, secondSet, secondNext);
				secondRun = true;
			}
			if (!result)
			{


					if ( !secondRun /* || (firstRun && subsetSortedSet (firstNext, middles) ) */)
					{
						proofChoice = 1;
					}
					else if ( !firstRun || (secondRun && subsetSortedSet (secondNext, middles) ))
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
						
						addTwinCoverage (level, nextLeft[level], nextRight[level], ternaryTrees, falseSet);
					}
					else if (proofChoice == 1)
								addBothCoverage (level, firstNext, ternaryTrees, falseSet);
					else
								addBothCoverage (level, secondNext, ternaryTrees, falseSet);



					copyNonTrunk (tryCopySet, falseSet);
					addFalseSetTop (tryCopySet);


			}
		}

	}


	return result;
}
