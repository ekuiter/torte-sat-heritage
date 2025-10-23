#include "permute.h"
#include "cputime.h"
#include "ternarytree.h"
#include "tts.h"
#include "options.h"
#include "bag.h"
#include "set.h"
#include "list.h"
#include "assoclist.h"
#include "dimacs.h"

#include <stdio.h>
#include <math.h>
#include <assert.h>


int clauseWidth (ClausePtr clause, int *permute)
{
	int min, max,v;
	
	min = permute[clause->variables[0]];
	max = permute[clause->variables[0]];

	for (v=1;v<clause->numOfVariables;v++)
	{
		if (permute[clause->variables[v]]<min)
			min = permute[clause->variables[v]];
		if (permute[clause->variables[v]]>max)
			max = permute[clause->variables[v]];
	}

	return 1+max-min;
}


double newPriority (int v, int pos, SetPtr clauses, int *clauseStatus, int *clauseStart, int *clauseVariableCounts, PropositionPtr p)
{
	ClausePtr clause;
	double result = 0;
	int c, c1;

	for (c=0;c<clauses->numOfElements;c++)
	{
		c1 = (int)clauses->elements[c];
		clause = p->clauses[c1];
		if (clauseStatus[c1] == 0)
			result -= 0;
		else if (clauseStatus[c1] == 1)
		{

			{
				if (clause->numOfVariables == 2)
					result += 200;

				switch (clause->numOfVariables-clauseVariableCounts[c1])
				{
						case 1: result += 100*(30.0/(30+pos-clauseStart[c1]));
								break;
						case 2: result += 10*(30.0/(30+pos-clauseStart[c1]));
								break;
						case 3: result += 5*(30.0/(30+pos-clauseStart[c1]));
								break;
						case 4: result += 2*(30.0/(30+pos-clauseStart[c1]));
								break;
						default: result += 1*(30.0/(30+pos-clauseStart[c1]));
								break;
				}
			}

		}
		else
			result -= 0;
	}

	return result;
}
void clauseWeightPermute1 (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v,v1,v2,v3,v4,c,c1,
		swap, bestV=1;
	SetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, *neighbours, set, threeSet;
	ClausePtr clause;
	int *varStatus, *clauseStatus, *clauseVarCounts, *clauseStart;
	double *priority, *weight, bestPriority;

	int recentClauseCount=50;


	ListPtr recentClauses;
	LinkPtr link, nextLink;
	SetPtr thisClause, opened, closed;

	recentClauses = newList();



	//printProblemSize (p->numOfClauses, p->clauses, numOfVariables, permute, unPermute);

	priority = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	weight = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	varStatus = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	clauseStatus = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseStart = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVarCounts = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVariableSets = (SetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(SetPtr),"neighbours");

	variableClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	variableMaxClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	neighbours = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");

	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newSet();
		variableMaxClauseSets[v] = newSet();
		neighbours[v] = newSet();
	}


	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newSet();
		clause = p->clauses[c];
		clauseVarCounts[c] = 0;
		clauseStatus[c] = 0;
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToSet (variableClauseSets[clause->variables[v]], (void *)c);
			addToSet (clauseVariableSets[c], (void *)clause->variables[v]);
			for (v1=0;v1<clause->numOfVariables;v1++)
				if (v != v1)
					addToSet (neighbours[clause->variables[v]], (void *)clause->variables[v1]);
		}
	}


	for (v=1;v<=numOfVariables;v++)
	{
		priority[v] = 0.0;
		weight[v] = 0.0;
		varStatus[v] = 0;
		priority[v] =  0;

	}

	set = newSet();
	threeSet = newSet();
	opened = newSet();
	closed = newSet();

	for (v=1;v<=numOfVariables;v++)
	{
		//printf ("%d\n",v);fflush(NULL);
		if (v==1)
		{
			bestV = numOfVariables;
		}

/*
		else if ((bestV = matchingPair (bestV, varStatus, variableClauseSets[bestV], clauseVariableSets)) != -1)
		{
		}
*/
		else if (recentClauses->length == 0)
		{
			swap = false;
			v1 = v;
			while (!swap)
			{
				v1--;
				v2 = unPermute[v1];
				for (v3=0;!swap && v3<neighbours[v2]->numOfElements;v3++)
				{
				   v4 = (int)neighbours[v2]->elements[v3];
				   if (varStatus[v4] == 0)
				   {
						bestV = v4;
						swap = true;
				   }
				}
			}
		}

		else
		{
			clearSet (set);
			link = recentClauses->first;
			for (c=0;c<recentClauseCount && link != NULL; (c++,link=link->next))
			{
				thisClause = (SetPtr) link->element;
				for (v1=0;v1<thisClause->numOfElements;v1++)
				{
					v2 = (int)thisClause->elements[v1];
					if (varStatus[v2] == 0)
						addToSet (set, (void *) v2);
				}
			}
			bestV = (int)set->elements[0];
			bestPriority = priority[bestV];
			for (v1=1;v1<set->numOfElements;v1++)
			{
				v2 = (int)set->elements[v1];
				if (priority[v2] > bestPriority)
				{
					bestV = v2;
					bestPriority = priority[v2];
				}
			}
			if (v % 20 == 0)
				for (v1=1;v1<=numOfVariables;v1++)
					priority[v1] /=1.5;
					
		   //printf ("      <%d>",bestV);

		}

		v2 = 0;
		for (c=0;c<variableClauseSets[bestV]->numOfElements;c++)
		{
			c1 = (int)variableClauseSets[bestV]->elements[c];
			for (v2=0;v2<clauseVariableSets[c1]->numOfElements;v2++)
			{
				v3 = (int)clauseVariableSets[c1]->elements[v2];
				priority[v3]+= 10.0/(1.0 + clauseVariableSets[c1]->numOfElements-clauseVarCounts[c1]);
			}
			clauseVarCounts[c1]++;
			if (clauseVarCounts[c1] == clauseVariableSets[c1]->numOfElements)
				v2++;
			if (clauseStatus[c1] == 0)
			{
				clauseStatus[c1] = 1;
				clauseStart[c1] = v;
				addListFirst (recentClauses, newCopySet(clauseVariableSets[c1]));
			}
		}
	   // printf ("Level %d, variable %d, closed %d\n", v, bestV, v2);

		for (link = recentClauses->first;link != NULL;)
		{
			thisClause = (SetPtr) link->element;
			nextLink = link->next;
			setMemberOfRemoveU (thisClause, (void *) bestV);
			if (thisClause->numOfElements == 0)
			{
				removeList (recentClauses, link);
				freeSet (thisClause);
			}
			link = nextLink;
		}

		while (recentClauses->length > recentClauseCount)
		{
			freeSet (removeListLast (recentClauses));
		}



		varStatus [bestV] = 1;
		unPermute[v] = bestV;


	}

	for (v=1;v<=numOfVariables;v++)
		permute[unPermute[v]] = v;


	for (v=1;v<=numOfVariables;v++)
	{
		freeSet (variableClauseSets[v]);
	}
	free (variableClauseSets);

	for (c=0;c<p->numOfClauses;c++)
	{
		freeSet (clauseVariableSets[c]);
	}
	free (clauseVariableSets);
	freeList (recentClauses);

}

void normalOrder(PropositionPtr p, int numOfVariables, int *unPermute)
{
	int i;

	for (i=1;i<=numOfVariables;i++)
		unPermute[i] = i;
}

int randomVariable (int numOfVariables)
{
	return 1+((double)random())*(numOfVariables)/((unsigned int)RAND_MAX+(unsigned int)1);
}

#define iabs(i) (i>0?i:-i)

#define contrib(a,b,n) ((a-b))
//#define contrib(a,b,n) ((a-b))

int maxWeight;

int weightedClause (int max, int min, int numOfVariables)
{
	int result;
	
	result = pow ( (double)(max-min),1.0);
	
	return result;
	return result;
}

int clauseGainWhenFlip2 (ClausePtr clause, int v1, int v2, int *permute, int numOfVariables)
{
	int min=0, max=0, otherMin, otherMax, v, vv, result, newMin=0, newMax=0;

	result = 0;

	if (clause->numOfVariables != 1)
	{
		min = permute[clause->variables[0]];
		max = permute[clause->variables[0]];

		if (v1 == clause->variables[0])
		{
			otherMin = permute[clause->variables[1]];
			otherMax = permute[clause->variables[1]];
		}
		else
		{
			otherMin = min;
			otherMax = max;
		}

		for (v=1;v<clause->numOfVariables;v++)
		{
			vv = clause->variables[v];
			if (permute[vv] > max)
				max = permute[vv];
			if (permute[vv] < min)
				min = permute[vv];
			if (vv != v1)
			{
			if (permute[vv] > otherMax)
				otherMax = permute[vv];
			if (permute[vv] < otherMin)
				otherMin = permute[vv];
			}
		}

		newMin = min;
		newMax = max;
		
		if (permute[v1] == min)
		{
			if (permute[v2] < otherMin)
				newMin = permute[v2];
			else
			{
				newMin = otherMin;
				if (permute[v2] > max)
					newMax = permute[v2];
			}
		}
		else if (permute[v1] == max)
		{
			if (permute[v2] > otherMax)
				newMax = permute[v2];
			else
			{
				newMax = otherMax;
				if (permute[v2] < min)
					newMin = permute[v2];
			}
		}
		else if (permute[v2] < min)
		{
			newMin = permute[v2];
		}
		else if (permute[v2] > max)
		{
			newMax = permute[v2];
		}
	}
	return weightedClause (newMax,newMin, numOfVariables)-weightedClause(max,min, numOfVariables);

}

int clauseSetGainWhenFlip2 (SetPtr clauses, int v1, int v2, ClausePtr *allClauses, int *permute, int numOfVariables)
{
	int c1, c2;
	ClausePtr clause;
	int result;

	result = 0;
	for (c1=0;c1<clauses->numOfElements;c1++)
	{
		c2 = (int)clauses->elements[c1];
		clause = allClauses[c2];
		result += clauseGainWhenFlip2 (clause, v1, v2, permute, numOfVariables);
	}
	return result;
}

int gainWhenFlip2 (int v1, int v2, ClausePtr *allClauses, int *permute, SetPtr *variableClauseSets, int numOfVariables)
{
	int result;
	SetPtr clauses;

	result = 0;
	clauses = newSet();
	copySet (clauses, variableClauseSets[v1]);
	differenceToSet (clauses, variableClauseSets[v2]);
	result += clauseSetGainWhenFlip2 (clauses, v1, v2, allClauses, permute, numOfVariables);
	copySet (clauses, variableClauseSets[v2]);
	differenceToSet (clauses, variableClauseSets[v1]);
	result += clauseSetGainWhenFlip2 (clauses, v2, v1, allClauses, permute, numOfVariables);

	//printf ("%d ", result);
	freeSet (clauses);
	return result;

}

int beatProbability (double power)
{
	int result;
	double p;

	if (power > 0.0)
		result = 1;
	else
	{
		p = exp (power);
		if (p >= 1)
			result = 1;
		else
			result = random() < p*RAND_MAX;
	}
	return result;
}

void alterPermutation (int oldPos, int newPos, int size, int numOfVariables,
							int *oldPermute, int *oldUnPermute,
							int *newPermute, int *newUnPermute)
{
	int v,*buffer;
	

	//printf ("\nAlter %d to %d (size %d)\n", oldPos, newPos, size);
	
	buffer = (int *) checkMalloc (size*sizeof(int),"");
	for (v=0;v<size;v++)
		buffer[v] = oldUnPermute[oldPos+v];
	   
	for (v=1;v<=numOfVariables;v++)
	{
		newUnPermute[v] = oldUnPermute[v];
	}
	
	for (v=oldPos;v<=numOfVariables-size;v++)
		newUnPermute[v] = newUnPermute[v+size];
		
	for(v=numOfVariables;v>=newPos+size;v--)
		newUnPermute[v] = newUnPermute[v-size];
		
	for (v=0;v<size;v++)
		newUnPermute[newPos+v] = buffer[v];
		
	for (v=1;v<=numOfVariables;v++)
		newPermute[newUnPermute[v]] = v;

/*
	for (v=1;v<=numOfVariables;v++)
		printf ("%d ", oldUnPermute[v]);
	printf ("\n");
	for (v=1;v<=numOfVariables;v++)
		printf ("%d ", newUnPermute[v]);
	printf ("\n\n");
*/   
		   
	free (buffer);
}
void combinedPermute (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v,v1,v2,v3,v4,c,
		swap;
	SetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, *neighbours;
	ClausePtr clause;
	int *status;
	double temperature, alpha;
	int swapCount, gain, defaultSwapCount;
	int maxNumOfVariables=500, maxNumOfClauses=1000;
	int *newPermute, *newUnPermute, *posWeightDelta, *leftCutDelta, *middleCutDelta, *rightCutDelta;
	int arrangementDelta, cutDelta, cutDelta1,
		bestArrangementDelta, previousArrangementDelta=0,
		newPos, bestNewPos, maxSize, size, localSwap;
	int insideMin, insideMax, outsideMin, outsideMax;
	int leftCut, middleCut, rightCut;



	if (maxNumOfVariables > numOfVariables)
		maxNumOfVariables = numOfVariables;
		
	if (maxNumOfClauses > p->numOfClauses)
		maxNumOfClauses = p ->numOfClauses;



	//printProblemSize (p->numOfClauses, p->clauses, numOfVariables, permute, unPermute);

	status = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	clauseVariableSets = (SetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(SetPtr),"neighbours");

	variableClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	variableMaxClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	neighbours = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	leftCutDelta = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	middleCutDelta = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	rightCutDelta = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	newPermute = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	newUnPermute = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	posWeightDelta = (int *) checkMalloc ( (2+numOfVariables)*sizeof(int),"neighbours");
	
	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newSet();
		variableMaxClauseSets[v] = newSet();
		neighbours[v] = newSet();
	}


	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newSet();
		clause = p->clauses[c];
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToSet (variableClauseSets[clause->variables[v]], (void *)c);
			addToSet (clauseVariableSets[c], (void *)clause->variables[v]);
			for (v1=0;v1<clause->numOfVariables;v1++)
				if (v != v1)
					addToSet (neighbours[clause->variables[v]], (void *)clause->variables[v1]);
		}
	}


	for (v=1;v<=numOfVariables;v++)
	{
		permute[v] = v;
		unPermute[v] = v;
	}

	temperature = 2*numOfVariables;
	alpha = 0.99;
	defaultSwapCount = 50;
	while (temperature > 2)
	{
			
		swapCount = defaultSwapCount;

		
		for (swapCount=0;swapCount < maxNumOfClauses*pow((double)maxNumOfVariables,2)/5000; swapCount++)


		//while (rollingAverage < 0.0)

		{
			v1 = randomVariable (numOfVariables);
			v2 =  randomVariable (numOfVariables);
			//for (v1=1;v1<numOfVariables;v1++)
				//for (v2=v1+1;v2<=numOfVariables;v2++)
				{
					gain = gainWhenFlip2 (v1, v2, p->clauses, permute, variableClauseSets, numOfVariables);
					if (beatProbability (- ((double)gain)/temperature))
					{
						v3 = permute[v1];
						permute[v1] = permute[v2];
						permute[v2] = v3;
						unPermute[permute[v1]] = v1;
						unPermute[permute[v2]] = v2;
					}
				}

		}
		temperature *= alpha;
	}
	
	for (v1=1;v1<numOfVariables;v1++)
		for (v2=v1+1;v2<v1+5 && v2<=numOfVariables;v2++)
		{
			gain = gainWhenFlip2 (v1, v2, p->clauses, permute, variableClauseSets, numOfVariables);
			if (gain < 0)
			{
				//printf (".");;
				v3 = permute[v1];
				permute[v1] = permute[v2];
				permute[v2] = v3;
				unPermute[permute[v1]] = v1;
				unPermute[permute[v2]] = v2;
			}
		}
	
	swap = true;
	maxSize = 0;
	while (swap || maxSize < 20)
	{                        
		swap = false;
		
		maxSize += 10;
		
		
		if (maxSize > 20)
			maxSize = 20;

		
		for (size=1;size<=maxSize;size++)
		{
			localSwap = true;
			while (localSwap)
			{
				localSwap = false;
				for (v4=1;v4<=numOfVariables;v4++)
				{
					v = permute[v4];
					
					//printf ("\nx %d %d (size = %d)\n", v4, v, size);
					if (v <= numOfVariables-size)
					{
						
					//printf ("\n%d %d (size = %d)\n", v4, v, size);
					
					for (v1=1;v1<v;v1++)
						newUnPermute[v1] = unPermute[v1];
					for (v1=v;v1<=numOfVariables-size;v1++)
						newUnPermute[v1] = unPermute[v1+size];
					for (v1=1;v1<=size;v1++)
						newUnPermute[numOfVariables+v1-size] = unPermute[v+v1-1];
						

					for (v1=1;v1<=numOfVariables;v1++)
						newPermute[newUnPermute[v1]] = v1;
						
/*
					printf ("\nv = %d\n", v);
					for (v1=1; v1<= numOfVariables; v1++)
						printf ("%d ", newUnPermute[v1]);
					printf ("\n");
*/
					for (v1=1;v1<=numOfVariables;v1++)
					{
						leftCutDelta[v1] = 0;
						middleCutDelta[v1] = 0;
						rightCutDelta[v1] = 0;
					}
				   
					
					for (newPos=1;newPos<=1+numOfVariables-size;newPos++)
					{
						posWeightDelta[newPos] = 0;
					}

					for (c=0;c<p->numOfClauses;c++)
					{
						outsideMin = numOfVariables+1;
						outsideMax = -1;
						insideMin = size+1;
						insideMax = -1;
						clause=p->clauses[c];
						for (v1=0;v1<clause->numOfVariables;v1++)
						{
							v2 = clause->variables[v1];
							v3 = newPermute[v2];
							if (v3<=numOfVariables-size)
							{
								if (v3 < outsideMin)
									outsideMin = v3;
								if (v3 > outsideMax)
									outsideMax = v3;
							}
							else
							{
								v3 = v3+size-numOfVariables;
								if (v3 < insideMin)
									insideMin = v3;
								if (v3 > insideMax)
									insideMax = v3;
							}
						}
						if (outsideMax >= 1) // not totally contained
						{
							if (insideMax == -1) // totally outside region
							{
								posWeightDelta[outsideMin+1]+=size;
								posWeightDelta[outsideMax+1]-=size;
						   }
							else
							{
								
								leftCutDelta[1]++;
								leftCutDelta[outsideMin+1]--;
								middleCutDelta[outsideMin+1]++;
								middleCutDelta[outsideMax+1]--;
								rightCutDelta[outsideMax+1]++;
								
								posWeightDelta[1] += 1+outsideMin+size-insideMin;
								posWeightDelta[outsideMin+1]-=1+size-insideMin;
								posWeightDelta[outsideMax+1]+= insideMax-1;
							 }
						}
						
					}
					
					//bestArrangement = permutedArrangement (numOfVariables, p->numOfClauses, permute,clauseVariableSets);
												
					bestNewPos = v;
					
					bestArrangementDelta = p->numOfClauses*numOfVariables;
					arrangementDelta = 0;
					
					leftCut=0;
					middleCut=0;
					rightCut=0;
					
					cutDelta1 = 0;
					for (newPos=1;newPos<=1+numOfVariables-size;newPos++)
						//if (newPos != v)
						{
							cutDelta = 0;
							arrangementDelta += posWeightDelta[newPos];
							


							leftCut += leftCutDelta[newPos];
							middleCut += middleCutDelta[newPos];
							rightCut += rightCutDelta[newPos];
						   
							cutDelta1 += size*middleCutDelta[newPos];
							
							cutDelta1 += rightCut-leftCut;
							
							//pw += pwd[newPos];
							
							//printf ("left = (%d,%d,%d)%d %d <%d>(%d,%d)\n", leftCut, middleCut, rightCut, cutDelta, cutDelta1, cutDelta1+pw, pw, pwd[newPos]);

							//printf ("<%d->%d>", newPos, arrangementDelta+cutDelta1);
														
							if (arrangementDelta+cutDelta1 < bestArrangementDelta)
							{
								bestArrangementDelta = arrangementDelta+cutDelta1;
								bestNewPos = newPos;
							}                         
							if (v == newPos)
							{
								//printf ("*");
								previousArrangementDelta = arrangementDelta+cutDelta1;
							}
						   
 
						}
					//printf ("\n");
					if (bestArrangementDelta < previousArrangementDelta)
					{
						//printf ("%d->%d(%d %d %d)\n", previousArrangementDelta,bestArrangementDelta, size, v, bestNewPos);
						swap = true;
						localSwap = true;
						alterPermutation (v, bestNewPos, size, numOfVariables, permute, unPermute, newPermute, newUnPermute);
					
						for (v1=1;v1<=numOfVariables;v1++)
						{
							permute[v1] = newPermute[v1];
							unPermute[v1] = newUnPermute[v1];
						}
					}
					}
				}
			}
		}
	}
	
   
	for (v=1;v<=numOfVariables;v++)
	{
		freeSet (variableClauseSets[v]);
	}
	free (variableClauseSets);

	for (c=0;c<p->numOfClauses;c++)
	{
		freeSet (clauseVariableSets[c]);
	}
	free (clauseVariableSets);


}
double newSetPriority (int s, int v, int *clauseStatus, int *clauseStart, SetPtr clauses, SetPtr *clauseVariableSets, SetPtr thisSet)
{
	double result = 0;
	int c,c1;
	SetPtr temp = newSet();

	for (c=0;c<clauses->numOfElements;c++)
	{
		c1 = (int) clauses->elements[c];
		if (clauseStatus[c1] == 0)
		{
			result -= 1;
		}
		else
		{
			copySet (temp, clauseVariableSets[c1]);
			differenceToSet (temp, thisSet);
			switch (temp->numOfElements)
			{
				case 0: result += 30;
						break;
				case 1: result += 10;
						break;
				case 2: result +=5;
						break;
				default:result += 1;
						break;
			}
		}
	}

	freeSet(temp);
	return result;
}

double newVarPriority (int v, int *clauseStatus, SetPtr clauses, int *clauseVarCounts, SetPtr *clauseVariableSets)
{
	double result = 0;
	int c,c1;

	for (c=0;c<clauses->numOfElements;c++)
	{
		c1 = (int) clauses->elements[c];
		if (clauseStatus[c1] == 0)
		{
			result -= 1;
		}
		else
		{
			switch (clauseVariableSets[c1]->numOfElements-clauseVarCounts[c1])
			{
				case 1: result += 5;
						break;
				case 2: result += 50;
						break;
				case 3: result += 2;
						break;
				default:result += 1;
						break;
			}
			switch (clauseVarCounts[c1])
			{
				case 0: result -= 0;
						break;
				case 1: result -= 0;
						break;
				case 2: result += 40;
						break;
				case 3: result += 0;
						break;
				default:result += 0;
						break;
			}

		}
	}

	return result;
}

void mergeFirstPermute (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int baseV, thisV, nextV,bestV=1, v,v1, v2,v3, v4, mergingFinished, linkFound;
	int s,s1,s2,bestS;
	int c,c1,c2,c3, clauseSize;

	SetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, *neighbours,
			mergedSet, varSet, allSets, *setClauseSets, *clauseSetSets, setNeighbours;
	ClausePtr clause;
	int *varStatus, *clauseStatus, *clauseVarCounts, *clauseStart,*varMap, numOfSets, *setStatus;

	double *priority,*setPriority,bestSetPriority,bestVarPriority, thisPriority;


	//printProblemSize (p->numOfClauses, p->clauses, numOfVariables, permute, unPermute);

	priority = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	varStatus = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	varMap = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	clauseStatus = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseStart = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVarCounts = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVariableSets = (SetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(SetPtr),"neighbours");

	variableClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	variableMaxClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	neighbours = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");

	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newSet();
		variableMaxClauseSets[v] = newSet();
		neighbours[v] = newSet();
		varStatus[v] = 0;
	}

	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newSet();
		clause = p->clauses[c];
		clauseVarCounts[c] = 0;
		clauseStatus[c] = 0;
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToSet (variableClauseSets[clause->variables[v]], (void *)c);
			addToSet (clauseVariableSets[c], (void *)clause->variables[v]);
			for (v1=0;v1<clause->numOfVariables;v1++)
				if (v != v1)
					addToSet (neighbours[clause->variables[v]], (void *)clause->variables[v1]);
		}
	}

	allSets = newSet();
	mergingFinished = false;
	while (!mergingFinished)
	{
		for (baseV=1;baseV<=numOfVariables && varStatus[baseV] != 0;baseV++);
		if (baseV > numOfVariables)
		{
			mergingFinished = true;
		}
		else
		{
			varStatus[baseV] = 1;
			mergedSet = newSet();
			addToSet (mergedSet, (void *) baseV);
			v1 = 0;
			clauseSize = 1;
			while (clauseSize < 6 &&  mergedSet->numOfElements < 10)
			{
				v1 = v1 % mergedSet->numOfElements;
				if (v1 == 0)
					clauseSize++;
				thisV = (int) mergedSet->elements[v1];
				v1++;
				for (c=0;c<variableClauseSets[thisV]->numOfElements && mergedSet->numOfElements<10;c++)
				{
					c1 = (int)variableClauseSets[thisV]->elements[c];
					if (clauseVariableSets[c1]->numOfElements <= clauseSize)
					{
						varSet = clauseVariableSets[c1];
						for (v4 = 0; v4 < varSet->numOfElements;v4++)
						{
							nextV = (int)varSet->elements[v4];
							if (nextV != thisV)
							{
								if (varStatus[nextV] == 0)
								{
									linkFound = mergedSet->numOfElements == 1;
									for (v2=0;!linkFound && v2<mergedSet->numOfElements;v2++)
									{
										v3 = (int)mergedSet->elements[v2];
										for (c2=0;!linkFound && c2<variableClauseSets[v3]->numOfElements;c2++)
										{
											c3 = (int)variableClauseSets[v3]->elements[c2];
											if (c3 != c1 && clauseVariableSets[c3]->numOfElements <= clauseSize && setMemberOfU (clauseVariableSets[c3], (void *)nextV))
												linkFound = true;
										}
									}
									if (linkFound)
									{
										varStatus[nextV] = 1;
										addToSet (mergedSet, (void *)nextV);
									}
								}
							}
						}
					}
				}
			}
			for (v=0;v<mergedSet->numOfElements;v++)
			{
				varMap[(int)mergedSet->elements[v]] = allSets->numOfElements;
			}
			addToSet (allSets, mergedSet);
			printSet (mergedSet);printf ("\n");
		}


	}

	numOfSets = allSets->numOfElements;
	clauseSetSets = (SetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(SetPtr),"neighbours");

	setClauseSets = (SetPtr *) checkMalloc ( (numOfSets)*sizeof(SetPtr),"neighbours");
	setStatus = (int *) checkMalloc ( (numOfSets)*sizeof(int),"neighbours");

	for (v=0;v<numOfSets;v++)
	{
		setClauseSets[v] = newSet();
		setStatus[v] = 0;
	}


	for (c=0;c<p->numOfClauses;c++)
	{
		clauseSetSets[c] = newSet();
		clause = p->clauses[c];
		for (v=0;v<clause->numOfVariables;v++)
		{
			v1 = clause->variables[v];
			if (!subsetSet(clauseVariableSets[c],allSets->elements[varMap[v1]]))
			{
				addToSet (setClauseSets[varMap[v1]], (void *)c);
				addToSet (clauseSetSets[c], (void *)varMap[v1]);
			}
		}
	}


	setPriority = (double *) checkMalloc ( (numOfSets)*sizeof(double),"neighbours");
	for (s=0;s<numOfSets;s++)
	{
		setPriority[s] = newSetPriority (s, 1, clauseStatus, clauseStart, setClauseSets[s], clauseVariableSets, allSets->elements[s]);

	}

	setNeighbours = newSet();

	v = 1;
	for (s=0;s<numOfSets;s++)
	{

		for(bestS=0;setStatus[bestS]!=0;bestS++);
		bestSetPriority = setPriority[s];
		for (s1 = bestS+1;s1<numOfSets;s1++)
			if (setStatus[s1] == 0)
				if (setPriority[s1] > bestSetPriority)
				{
					bestS = s1;
					bestSetPriority = setPriority[s1];
				}

		setStatus[bestS] = 1;
		//printf ("Set %d\n", bestS);
		varSet = allSets->elements[bestS];

		for (v1=0;v1<varSet->numOfElements;v1++)
		{
			bestVarPriority = -1000000000.0;

			for (v2=0;v2<varSet->numOfElements;v2++)
			{
				v3 = (int)varSet->elements[v2];
				if (varStatus[v3] == 1)
				{
					thisPriority = newVarPriority (v3, clauseStatus, variableClauseSets[v3], clauseVarCounts, clauseVariableSets);
					if (thisPriority > bestVarPriority)
					{
						bestV = v3;
						bestVarPriority = thisPriority;
					}
				}
			}
			varStatus[bestV] = 2;
			permute[bestV] = v;
			unPermute[v] = bestV;
			//printf ("bestV = %d\n",bestV);
			for (c=0;c<variableClauseSets[bestV]->numOfElements;c++)
			{
				c1 = (int)variableClauseSets[bestV]->elements[c];
				if (clauseStatus[c1] == 0)
				{
					clauseStatus[c1] = 1;
					clauseStart[c1] = v;
				}
				setMemberOfRemoveU (clauseVariableSets[c1],(void *)bestV);
			}

			v++;
		}

		clearSet (setNeighbours);
		for (c=0;c<setClauseSets[bestS]->numOfElements;c++)
		{
			c1 = (int)setClauseSets[bestS]->elements[c];
			clauseStatus[c1] = 1;
			//differenceToSet (clauseVariableSets[c1], varSet);
			for (s1=0;s1<clauseSetSets[c1]->numOfElements;s1++)
			{
				s2 = (int)clauseSetSets[c1]->elements[s1];
				addToSet (setNeighbours,(void *)s2);
			}
		}
		for (s1=0;s1<setNeighbours->numOfElements;s1++)
		{
			s2 = (int)setNeighbours->elements[s1];
			setPriority[s2] += newSetPriority (s2, v, clauseStatus, clauseStart, setClauseSets[s2], clauseVariableSets, allSets->elements[s2]);
		}
	}
}

void fastPermute (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int bestV, v,v1, v2;
	int c,c1;

	SetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, *neighbours;
	ClausePtr clause;
	int *varStatus, *clauseStatus, *clauseVarCounts, *clauseStart,*varMap;

	double *priority,bestVarPriority, thisPriority;


	//printProblemSize (p->numOfClauses, p->clauses, numOfVariables, permute, unPermute);

	priority = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	varStatus = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	varMap = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	clauseStatus = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseStart = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVarCounts = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVariableSets = (SetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(SetPtr),"neighbours");

	variableClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	variableMaxClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	neighbours = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");

	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newSet();
		variableMaxClauseSets[v] = newSet();
		neighbours[v] = newSet();
		varStatus[v] = 0;
	}

	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newSet();
		clause = p->clauses[c];
		clauseVarCounts[c] = 0;
		clauseStatus[c] = 0;
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToSet (variableClauseSets[clause->variables[v]], (void *)c);
			addToSet (clauseVariableSets[c], (void *)clause->variables[v]);
			for (v1=0;v1<clause->numOfVariables;v1++)
				if (v != v1)
					addToSet (neighbours[clause->variables[v]], (void *)clause->variables[v1]);
		}
	}




	for (v=1;v<=numOfVariables;v++)
	{

		for(bestV=0;varStatus[bestV]!=0;bestV++);
		bestVarPriority = newVarPriority (bestV, clauseStatus, variableClauseSets[bestV], clauseVarCounts, clauseVariableSets);

		for (v2=1;v2<=numOfVariables;v2++)
		{
			if (varStatus[v2] == 0)
			{
				thisPriority = newVarPriority (v2, clauseStatus, variableClauseSets[v2], clauseVarCounts,  clauseVariableSets);
				if (thisPriority > bestVarPriority)
				{
					bestV = v2;
					bestVarPriority = thisPriority;
				}
			}
		}
		varStatus[bestV] = 2;
		permute[bestV] = v;
		unPermute[v] = bestV;
			//printf ("bestV = %d\n",bestV);
		for (c=0;c<variableClauseSets[bestV]->numOfElements;c++)
		{
			c1 = (int)variableClauseSets[bestV]->elements[c];
			if (clauseStatus[c1] == 0)
			{
				clauseStatus[c1] = 1;
				clauseStart[c1] = v;
			}
			clauseVarCounts[c1]++;
		}

	}
}

int findArrangement (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v,v1,c,c1,outsideMin,outsideMax,leftCut,maxCut,v2,v3,*clauseLengths;
	SetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, set;
	ClausePtr clause;
	int *clauseStatus, *clauseVarCounts;

	int arrangement;


	clauseStatus = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVarCounts = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseLengths = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVariableSets = (SetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(SetPtr),"neighbours");

	variableClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");
	variableMaxClauseSets = (SetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(SetPtr),"neighbours");

	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newSet();
		variableMaxClauseSets[v] = newSet();
	}


	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newSet();
		clause = p->clauses[c];
		clauseVarCounts[c] = clause->numOfVariables;
		clauseLengths[c] = 0;
		clauseStatus[c] = 0;
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToSet (variableClauseSets[clause->variables[v]], (void *)c);
			addToSet (clauseVariableSets[c], (void *)clause->variables[v]);
		}
	}



	arrangement = 0;
	maxCut = 0;
	leftCut = 0;
	for (c=0;c<p->numOfClauses;c++)
	{
		clauseStatus[c] = 0;
		clauseVarCounts[c] = clauseVariableSets[c]->numOfElements;
	}

	for (v=1;v<=numOfVariables;v++)
	{
		for (c=0;c<p->numOfClauses;c++)
			if (clauseStatus[c] == 1)
				clauseLengths[c]++;
		set = variableClauseSets[unPermute[v]];
		for (c=0;c<set->numOfElements;c++)
		{
			c1 = (int) (set->elements[c]);
			clauseVarCounts[c1]--;
			if (clauseStatus[c1] == 0)
			{
				clauseStatus[c1] = 1;
				clauseLengths[c1]++;
				leftCut++;
			}
			if (clauseVarCounts[c1] == 0 && clauseStatus[c1] == 1)
			{
				clauseStatus[c1] = 2;
				leftCut--;
			}
		}
		if (leftCut > maxCut)
			maxCut = leftCut;
	}

 /*
	for (c=0;c<p->numOfClauses;c++)
	if (clauseVariableSets[c]->numOfElements > 2)
	{
		//printSet (clauseVariableSets[c]);
		printf ("Clause %d, length %d\n", c, clauseLengths[c]);
	}
*/
	for (c=0;c<p->numOfClauses;c++)
	{
		outsideMin = numOfVariables+1;
		outsideMax = -1;
		clause=p->clauses[c];
		for (v1=0;v1<clause->numOfVariables;v1++)
		{
			v2 = clause->variables[v1];
			v3 = permute[v2];
			if (v3 < outsideMin)
				outsideMin = v3;
			if (v3 > outsideMax)
				outsideMax = v3;
		}
		arrangement += 1+outsideMax-outsideMin;
	}

	printf ("c Arrangment = %d Max cut = %d\n", arrangement, maxCut);
	return arrangement;
}


void dumpRules (PropositionPtr p)
{
	FILE *f;
	int v,c;
	ClausePtr clause;
	char rule[200];

	f = fopen ("x.r","w");

	for (c=0;c<p->numOfClauses;c++)
	{
		clause = p->clauses[c];
		for (v=1;v<=p->maxVariable;v++)
			rule[v] = '*';
		for (v=0;v<clause->numOfVariables;v++)
			if (clause->positive[v])
				rule[clause->variables[v]] = '0';
			else
				rule[clause->variables[v]] = '1';
		for (v=1;v<=p->maxVariable;v++)
			fprintf (f, "%c", rule[v]);
		fprintf (f," %d\n", 1);
	}

	fclose (f);
}

void labelGraph (PropositionPtr p)
{
	FILE *f;
	int v,v1,c;
	ClausePtr clause;

	f = fopen ("x.g","w");

	for (c=0;c<p->numOfClauses;c++)
	{
		clause = p->clauses[c];
		for (v=0;v<clause->numOfVariables;v++)
			for (v1=v+1;v1<clause->numOfVariables;v1++)
				fprintf (f, "%d %d 1\n", clause->variables[v], clause->variables[v1]);
	}

	fclose (f);
}

void noPermute (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v;
	for (v=1;v<=numOfVariables;v++)
	{
		permute[v] = v;
		unPermute[v] = v;
	}
}

double findShift (int iterations, int numOfVariables, int numOfClauses, int lower, double shift)
{
	int limit;
	double result;
	limit = numOfVariables*numOfVariables*5;
	//limit = numOfVariables*numOfClauses;
	
	//ratio = ((double)numOfClauses) / numOfVariables;
	
	//if (lower > limit) return 0;
	
	//if (lower < numOfVariables) return numOfVariables;
	
	//return 1.0;
	

	result = (numOfVariables*numOfVariables)/((double) 2*numOfVariables + lower);
	
	//result = numOfVariables*( 1.0 - log ( 1.0 + (double) lower) / log ( (double) limit))/2.0;

	//result = numOfVariables*numOfVariables/((double) 10*numOfVariables + lower);
	
	return result;
}

double weightSum (double *ws, int n)
{
	double result;
	int i;
	result = 0;
	for (i=0;i<n;i++)
		result += ws[i];
	return result;
}


void findPermutation (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v;
	FILE *f;
	int method;
	char comment[100];

	for (v=1;v<=numOfVariables;v++)
		unPermute[v] = v;

	method = intOptions[PERMUTATION_METHOD];
	if (method == 100)
	{
		if (p->maxVariable < 200 && p->numOfClauses < 350)
			method = 3;
		else
			method = 4;
	}
	sprintf (comment, "using permutation method %d", method);
	dimacsComment (comment);
	
	if (stringOptions[PERMUTATION_IN] == NULL)
	{
		switch (method)
		{
			case 0:
				noPermute (p, d, initialD, r, numOfVariables, permute, unPermute);
				break;

			case 1:
				mergeFirstPermute (p, d, initialD, r, numOfVariables, permute, unPermute);
				break;

			case 2:
				fastPermute (p, d, initialD, r, numOfVariables, permute, unPermute);
				break;

			case 3:
				combinedPermute (p, d, initialD, r, numOfVariables, permute, unPermute);
				break;

			case 4:
				clauseWeightPermute1 (p, d, initialD, r, numOfVariables, permute, unPermute);
				break;

		}
	}
	else
		readPermutation (numOfVariables, unPermute, permute);



	for (v=1;v<=numOfVariables;v++)
	{
		permute[unPermute[v]] = v;
	}

	//dumpRules (p);
	//labelGraph (p);
	
	if (booleanOptions[PRINT_ARRANGEMENT])
		findArrangement (p, d, initialD, r, numOfVariables, permute, unPermute);

	if (stringOptions[PERMUTATION_OUT] != NULL)
	{
		f = fopen (stringOptions[PERMUTATION_OUT],"w");
		for (v=1;v<=numOfVariables;v++)
			fprintf (f,"%d\n", unPermute[v]);
		fclose (f);
	}


	if (!booleanOptions[SOLVE])
		exit(1);

}


void readPermutation (int numOfVariables, int *unPermute, int *permute)
{
	FILE *vars;
	int v1,v2,ok;

	for (v1=1;v1<=numOfVariables;v1++)
		permute[v1] = 0;

	vars = fopen (stringOptions[PERMUTATION_IN],"r");
	ok = vars != NULL;

	if (!ok)
	{
		fprintf (stderr, "Could not open %s for reading\n",stringOptions[PERMUTATION_IN]);
		exit(1);
	}
	for (v1=1;ok && v1<=numOfVariables;v1++)
	{
		ok = fscanf (vars,"%d",&v2) == 1;
		if (ok)
		{
			if (permute[v2] != 0)
			{
				printf ("%d entered twice at lines %d and %d \n",v2,permute[v2],v1);
				exit(0);
			}
			permute[v2] = v1;
			unPermute[v1] = v2;
		}
	}


	for (v1=1;v1<=numOfVariables;v1++)
	{
		if (permute[v1] == 0)
		{
			printf ("%d omitted\n",v1);
			exit(0);
		}
	}

}
