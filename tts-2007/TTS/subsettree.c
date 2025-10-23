#include "memutil.h"
#include "subsettree.h"
#include "set.h"
#include "ternarytree.h"
#include "counts.h"

#include <stdio.h>
#include <assert.h>

#define SUBSETTREEBUFFERSIZE 10000
SubsetTree *subsetTreeBuffer;
static int nextBufferPos=SUBSETTREEBUFFERSIZE;

static int emptySetAdded = false;

static SubsetTreePtr endPoint;

void initSubsetTree()
{
	endPoint = newSubsetTree();
	//endPoint->minSetSize = 0;
}

SubsetTreePtr newSubsetTree ()
{
	SubsetTreePtr result;

	if (nextBufferPos==SUBSETTREEBUFFERSIZE)
	{
		subsetTreeBuffer = (SubsetTreePtr) checkMalloc (SUBSETTREEBUFFERSIZE*sizeof (SubsetTree),"SubsetTree");
		nextBufferPos = 0;
	}
	result = &subsetTreeBuffer[nextBufferPos++];
	
	//result = checkMalloc (sizeof (SubsetTree),"SubsetTree");
	result->tree = NULL;
	result->absent = NULL;
	result->present = NULL;
	//result->minSetSize = 10000;


	return result;
}

void putBackSubsetTree()
{

	if (nextBufferPos != 0)
		nextBufferPos--;

}

void freeSubsetTree (SubsetTreePtr n)
{
	//printf ("FST %d %d\n",FST++,NST);
	return;
	
	if (n != NULL)
	{
		freeSubsetTree (n->present);
		freeSubsetTree (n->absent);
		unCount(2);
		free (n);
	}
}

int subsetOrEqual (SubsetTreePtr startPoint, int index, SetPtr set, SetPtr s
#ifdef PROOF
,ProofClausePtr *proofClause
#endif
																)
{
	int result, secondResult, inter1, position=0,i;
	SubsetTreePtr n;
	SetPtr temp = NULL;
	inter1 = true;
	
	if (startPoint != NULL)
	{
		//printf ("<%d,%d,%d>",startPoint->tree,set->elements[index],index);
	}
	
	
	if (startPoint == NULL)
		result = false;
#ifdef PROOF
	else if (startPoint->present == NULL && startPoint->absent == NULL)
	{
		result = true;
		*proofClause = startPoint->proofClause;
	}
#else
	else if (startPoint == endPoint)
	{
		result = true;
	 }
#endif
/*
	else if ( startPoint->minSetSize > set->numOfElements-index)
	{
		printf ("%d ",startPoint->minSetSize - (set->numOfElements-index));fflush(stdout);
		result = false;
	}
*/
	else
	{
		n = startPoint;
		while (
			index < set->numOfElements && set->elements[index] < n->tree)
		{
			index++;
		}
		if (index == set->numOfElements)
			result = false;
		else
		{
			if (set->elements[index] == n->tree)
			{
				if (s != NULL)
					position = s->numOfElements;
				result = subsetOrEqual (n->absent, index+1, set, s
#ifdef PROOF
,proofClause
#endif
																		);
				if (result)
				{

					temp = newSet();
					secondResult = subsetOrEqual (n->present, index+1, set, temp
#ifdef PROOF
,proofClause
#endif
																			);
					if (secondResult)
					{
						if (s != NULL)
						{
							addToSetNoCheck (temp, n->tree);
						}
						if (temp->numOfElements < s->numOfElements-position)
						{
							//printf ("%d << %d\n", temp->numOfElements,s->numOfElements-position);
							for (i=0;i<temp->numOfElements;i++)
								s->elements[position+i] = temp->elements[i];
							s->numOfElements = position + temp->numOfElements;
						}
					}
					freeSet (temp);

				}
				else
				{
					//printf ("P1");
					result = subsetOrEqual (n->present, index+1, set, s
#ifdef PROOF
,proofClause
#endif
																			);
					if (result)
					{
						if (s != NULL)
						{
							addToSetNoCheck (s, n->tree);
						}
					}
				}
		   }
			else
			{
				//printf ("A2");
				result = subsetOrEqual (n->absent, index, set, s
#ifdef PROOF
,proofClause
#endif
																	);
			}
		}
	}

	return result;
}

SetPtr findBFalseSubset (void * n,SetPtr s)
{
	SetPtr result;
	result = newSet();
	if (subsetOrEqual (((TernaryTreePtr) n)->subsets, 0, s, result))
	{
		sortSet (result);
		//printf ("found %d ", n); printSet(result); printf ("\n");
	}
	else
	{
		freeSet (result);
		result = NULL;
	}
	return result;
}

SetPtr findFalseSubset (SetPtr s)
{
	SetPtr result,temp;
	int i,found, found1;
	
	if (emptySetAdded)
	{
		printf ("empty set found\n");
		return newSet();
	}

	found = false;
	found1 = false;
	result = newSet();
	temp = newSet();

 
	for (i=0;i<s->numOfElements;i++)
	{
		clearSet (temp);
		if (subsetOrEqual (((TernaryTreePtr)s->elements[i])->subsets, i, s, temp
 #ifdef PROOF
 , proofClause
 #endif
																						))
		{
			
			if (!found)
				copySet (result, temp);
			else if (temp->numOfElements < result->numOfElements)
			{
				//printf ("%d < %d\n", temp->numOfElements, result->numOfElements);
				copySet (result, temp);
			}

			found = true;
		}
	}
 
	freeSet (temp);
	//found = subsetOrEqual (topTree, 0, s, result);
	 
	if (found)
	{
		sortSet (result);
	}
	else
	{
		freeSet (result);
		result = NULL;
	}
	
	return result;
}

SubsetTreePtr addFalseSetSub (SubsetTreePtr startPoint, int index, SetPtr set
#ifdef PROOF
	,ProofClausePtr proofClause
#endif
	)
{
	SubsetTreePtr newNode, result;

	result = startPoint;
	if (startPoint == NULL)
	{
		result = newSubsetTree();
	}
	if (index == set->numOfElements)
	{
#ifdef PROOF
		if (result->present != NULL)
			freeSubsetTree ( result->present);
		result->present = NULL;
		if (result->absent != NULL)
			freeSubsetTree ( result->absent);
		result->absent = NULL;
		result->proofClause = proofClause;
		
#else
		if (startPoint == NULL)
			putBackSubsetTree();
		result = endPoint;
#endif

	}
	else
	{
		if ( result->tree == NULL)
			result->tree = set->elements[index];
		if ( result->tree == set->elements[index])
		{
			result->present = addFalseSetSub (result->present, index+1, set
#ifdef PROOF
,proofClause
#endif
										);
		}
		else if ( result->tree < set->elements[index])
		{
			result->absent = addFalseSetSub (result->absent, index, set
#ifdef PROOF
,proofClause
#endif
										);
		}
		else
		{
			newNode = newSubsetTree();
			newNode->tree = set->elements[index];
			newNode->absent = result;
			newNode->present = addFalseSetSub (newNode->present, index+1, set
#ifdef PROOF
,proofClause
#endif
													);
			result = newNode;
		 }
	}
/*
	if (result->absent != NULL && result->absent->minSetSize < result->minSetSize)
		result->minSetSize = result->absent->minSetSize;
	if (result->present != NULL && result->present->minSetSize+1 < result->minSetSize)
		result->minSetSize = result->present->minSetSize+1;
*/
	return result;
}


void addFalseSetTop (SetPtr set
#ifdef PROOF
,ProofClausePtr proofClause
#endif
								)
{
	TernaryTreePtr r;
	
	if (set->numOfElements == 0)
	{
		emptySetAdded = true;
	}
	else
	{
		r = set->elements[0];

		r->subsets =  addFalseSetSub (r->subsets, 0 , set);
	}

 

}

void addBFalseSetTop (void * n, SetPtr set )
{

	((TernaryTreePtr )n)->subsets =  addFalseSetSub (((TernaryTreePtr )n)->subsets, 0 , set);

 
}
