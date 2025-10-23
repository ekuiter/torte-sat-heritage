#include "memutil.h"
#include "subsettree.h"
#include "set.h"
#include "ternarytree.h"
#include "dimacs.h"
#include "options.h"
#include "tts.h"

#include <stdio.h>

#ifdef THREADS
#include <pthread.h>

pthread_mutex_t newSubsetMutex;
#endif

#define SUBSETTREEBUFFERSIZE 10000
SubsetTree **subsetTreeBuffer;
static int *nextBufferPos;

static int emptySetAdded = false;
static int emptySetId;
int buffersAllocated;
static SetPtr *returnedItems;

SubsetTreePtr endPoint;

SetPtr allsubsets, allbuffers;

#ifdef PROOF
	int nextSetId;
#endif

void initSubsetTreeMemory()
{
	char message[1000];
	int t;
	
	allsubsets = newSet();
	allbuffers = newSet();
	
	subsetTreeBuffer = (SubsetTreePtr *) checkMalloc (numThreads*sizeof(SubsetTreePtr),"");
	returnedItems = (SetPtr *) checkMalloc (numThreads*sizeof(SetPtr),"");
	nextBufferPos = (int *) checkMalloc (numThreads*sizeof(int),"");
	

	for (t=0;t<numThreads;t++)
	{
		nextBufferPos[t] = SUBSETTREEBUFFERSIZE;
		returnedItems[t] = newSet();
		buffersAllocated = 1;
	}

	sprintf (message, "Size of subset node = %d", sizeof (SubsetTree));
	dimacsComment (message);


	


	//endPoint->minSetSize = 0;
}
void initSubsetTree()
{
#ifdef THREADS
	checkP (pthread_mutex_init (&newSubsetMutex,NULL));
#endif

#ifdef PROOF
	nextSetId = 1;
#endif
 
	endPoint = checkMalloc (sizeof(SubsetTree),"endPoint");
	endPoint->tree=NULL;
	endPoint->present=NULL;
	endPoint->absent=NULL;
	initSubsetTreeMemory();
}


void returnSubsetTree (SubsetTreePtr s, int tid)
{
	addToSetNoCheck (returnedItems[tid], s);
}

void resetTrees()
{
	int i;
	TernaryTreePtr node;
	
	printf ("c Resetting subsets\n");
	for (i=0;i<allTTNodes->numOfElements;i++)
	{
		node = allTTNodes->elements[i];
		node->subsets = NULL;
	}
	
	for (i=0;i<allbuffers->numOfElements;i++)
		free ( (SubsetTreePtr *) allbuffers->elements[i]);
		
	initSubsetTreeMemory();
}

SubsetTreePtr newSubsetTree (int tid)
{
	
	SubsetTreePtr result;


	subsetptrs++;
	
	if (returnedItems[tid]->numOfElements > 0)
	{
		result = returnedItems[tid]->elements[--returnedItems[tid]->numOfElements];
	}
	else
	{
		if (nextBufferPos[tid]==SUBSETTREEBUFFERSIZE)
		{
	
	#ifdef THREADS
			checkP (pthread_mutex_lock(&newSubsetMutex));
	#endif
	
			subsetTreeBuffer[tid] = (SubsetTreePtr) checkMallocContinue (SUBSETTREEBUFFERSIZE*sizeof (SubsetTree),"SubsetTree");
			if (subsetTreeBuffer[tid] != NULL)
				addToSetNoCheck (allbuffers, subsetTreeBuffer[tid]);
			buffersAllocated++;
	
	#ifdef THREADS
			checkP (pthread_mutex_unlock(&newSubsetMutex));
	#endif

			if (subsetTreeBuffer[tid] == NULL)
			{
	#ifdef THREADS
				printf ("c Multithreaded version cannot reclaim memory\n");
				exit (dimacsUNKNOWN);
	#endif
	
				resetTrees();
				return NULL;
			}

				
			nextBufferPos[tid] = 0;
		}
	
		result = &subsetTreeBuffer[tid][nextBufferPos[tid]++];
	}

	
	//result = checkMalloc (sizeof (SubsetTree),"SubsetTree");
	result->tree = NULL;
	result->absent = NULL;
	result->present = NULL;
	//result->count = 0;
	//result->minSetSize = 10000;

	//addToSetNoCheck (allsubsets, result);

	return result;
}

void finalsetprint()
{
	int i;
	int histo[21];
	SubsetTreePtr s;
	for (i=0;i<21;i++)
		histo[i] = 0;
	for (i=0;i<allsubsets->numOfElements;i++)
	{
		s = allsubsets->elements[i];
		if (s->present == endPoint)
		{
//            if (s->count >= 20)
//                histo[20]++;
//            else
//                histo[s->count]++;
		}
	}
	for (i=0;i<19;i++)
		printf ("%d -> %d\n", i, histo[i]);
	printf (">= 20 -> %d\n", histo[20]);
}
void putBackSubsetTree(int tid)
{
	subsetptrs--;
	
	if (nextBufferPos[tid] != 0)
		nextBufferPos[tid]--;

}

void freeSubsetTree (SubsetTreePtr n)
{
	//printf ("FST %d %d\n",FST++,NST);
	return;
	
	if (n != NULL)
	{
		freeSubsetTree (n->present);
		freeSubsetTree (n->absent);
		free (n);
	}
}

int subsetOrEqual (SubsetTreePtr startPoint, int index, SetPtr set, SetPtr s, int *id)
{
	int result, secondResult, inter1, position=0,i;
	int tempid;
	SubsetTreePtr n;
	SetPtr temp = NULL;
	inter1 = true;
	
	if (startPoint != NULL)
	{
		//printf ("<%d,%d,%d>",startPoint->tree,set->elements[index],index);
	}
	
	
	if (startPoint == NULL)
		result = false;
	else if (startPoint->present == NULL && startPoint->absent == NULL)
	{
		result = true;
#ifdef PROOF
		*id = startPoint->setId;
#endif
	 }

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
				result = subsetOrEqual (n->absent, index+1, set, s, id);
				if (result)
				{

					temp = newSet();
					secondResult = subsetOrEqual (n->present, index+1, set, temp, &tempid);
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
							*id = tempid;
						}
					}
					freeSet (temp);

				}
				else
				{
					//printf ("P1");
					result = subsetOrEqual (n->present, index+1, set, s, id);
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
				result = subsetOrEqual (n->absent, index, set, s, id);
			}
		}
	}

	return result;
}

/*
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
*/

int isFalseSubset (SubsetTreePtr start, SetPtr set, SetPtr s)
{
	int id;
	
	return subsetOrEqual (start, 0, set, s, &id);
}

SetPtr findFalseSubset (SetPtr s, int *id)
{
	SetPtr result,temp;
	int i,found, found1, tempId;
	
	if (emptySetAdded)
	{
		//printf ("empty set found\n");
		*id = emptySetId;
		return newSet();
	}

	found = false;
	found1 = false;
	result = newSet();
	temp = newSet();

 
	for (i=0;i<s->numOfElements;i++)
	{
		clearSet (temp);
		if (subsetOrEqual (((TernaryTreePtr)s->elements[i])->subsets, i, s, temp, &tempId))
		{
			
			if (!found)
			{
				copySet (result, temp);
				*id = tempId;
			}
			else if (temp->numOfElements < result->numOfElements)
			{
				//printf ("%d < %d\n", temp->numOfElements, result->numOfElements);
				copySet (result, temp);
#ifdef PROOF
				*id = tempId;
#endif
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

SubsetTreePtr addFalseSetSub (int tid, SubsetTreePtr startPoint, int index, SetPtr set, int *id)
{
	SubsetTreePtr newNode, result;

	result = startPoint;
	if (startPoint == NULL)
	{
		result = newSubsetTree(tid);
	}
	if (index == set->numOfElements)
	{
		result->present = NULL;
		result->absent = NULL;
#ifdef PROOF
		*id = nextSetId++;
		result->setId = *id;
#endif
	}
	else
	{
		if ( result->tree == NULL)
			result->tree = set->elements[index];
		if ( result->tree == set->elements[index])
		{
			result->present = addFalseSetSub (tid, result->present, index+1, set, id);
		}
		else if ( result->tree < set->elements[index])
		{
			result->absent = addFalseSetSub (tid, result->absent, index, set, id);
		}
		else
		{
			newNode = newSubsetTree(tid);
			newNode->tree = set->elements[index];
			newNode->absent = result;
			newNode->present = addFalseSetSub (tid, newNode->present, index+1, set, id);
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


int addFalseSetTop (int tid, SetPtr set)
{
	TernaryTreePtr r;
	int id;
	
	if (set->numOfElements == 0)
	{
		emptySetAdded = true;
#ifdef PROOF
		emptySetId = nextSetId++;
		id = emptySetId;
#endif
	}
	else
	{
		r = set->elements[0];

		r->subsets =  addFalseSetSub (tid, r->subsets, 0 , set, &id);
	}

	return id;

}
/*
void addBFalseSetTop (void * n, SetPtr set )
{

	((TernaryTreePtr )n)->subsets =  addFalseSetSub (((TernaryTreePtr )n)->subsets, 0 , set);

 
}
*/
