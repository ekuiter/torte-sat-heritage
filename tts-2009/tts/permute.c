#include "permute.h"
#include "cputime.h"
#include "ternarytree.h"
#include "tts.h"
#include "options.h"
#include "set.h"
#include "intset.h"
#include "list.h"
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


double newPriority (int v, int pos, IntSetPtr clauses, int *clauseStatus, int *clauseStart, int *clauseVariableCounts, PropositionPtr p)
{
	ClausePtr clause;
	double result = 0;
	int c, c1;

	for (c=0;c<clauses->numOfElements;c++)
	{
		c1 = clauses->elements[c];
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


void clauseWeightPermute10 (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v,v1,v2,c,c1,
		bestV=1;
	IntSetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, *neighbours, set, threeSet;
	ClausePtr clause;
	int *varStatus, *clauseStatus, *clauseVarCounts, *clauseStart;
	double *priority, *weight, bestPriority;



	ListPtr recentClauses;

	recentClauses = newList();



	//printProblemSize (p->numOfClauses, p->clauses, numOfVariables, permute, unPermute);

	priority = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	weight = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	varStatus = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	clauseStatus = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseStart = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVarCounts = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVariableSets = (IntSetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(IntSetPtr),"neighbours");

	variableClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	variableMaxClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	neighbours = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");

	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newIntSet();
		variableMaxClauseSets[v] = newIntSet();
		neighbours[v] = newIntSet();
	}


	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newIntSet();
		clause = p->clauses[c];
		clauseVarCounts[c] = 0;
		clauseStatus[c] = 0;
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToIntSet (variableClauseSets[clause->variables[v]], c);
			addToIntSet (clauseVariableSets[c], clause->variables[v]);
			for (v1=0;v1<clause->numOfVariables;v1++)
				if (v != v1)
					addToIntSet (neighbours[clause->variables[v]], clause->variables[v1]);
		}
	}


	for (v=1;v<=numOfVariables;v++)
	{
		priority[v] = 0.0;
		weight[v] = 0.0;
		varStatus[v] = 0;
		priority[v] =  neighbours[v]->numOfElements;

	}

	set = newIntSet();
	threeSet = newIntSet();

	for (v=1;v<=numOfVariables;v++)
	{
		//printf ("%d\n",v);fflush(NULL);

		bestV = 1;
		while (varStatus[bestV] == 2) bestV++;
		bestPriority = priority[bestV];
		for (v1=bestV+1;v1<=numOfVariables;v1++)
		{
			if (varStatus[v1] != 2 && priority[v1] > priority[bestV])
			{
				bestV = v1;
				bestPriority = priority[bestV];
			}
		}
		varStatus[bestV] = 2;
		v2 = 0;
		for (c=0;c<neighbours[bestV]->numOfElements;c++)
		{
			c1 = neighbours[bestV]->elements[c];
			priority[c1] += 10;                
			
		}

		unPermute[v] = bestV;


	}

	for (v=1;v<=numOfVariables;v++)
		permute[unPermute[v]] = v;


	for (v=1;v<=numOfVariables;v++)
	{
		freeIntSet (variableClauseSets[v]);
	}
	free (variableClauseSets);

	for (c=0;c<p->numOfClauses;c++)
	{
		freeIntSet (clauseVariableSets[c]);
	}
	free (clauseVariableSets);
	freeList (recentClauses);

}

void addPriority (int count, int *varStatus, double *priority, double weight, int v, IntSetPtr vs, IntSetPtr *clauseVariableSets, IntSetPtr *variableClauseSets, int *clauseVarCounts)
{
	int v1,v2,c1,c2;
	
	//printf ("%d %e\n",v, weight);
	addToIntSet (vs, v);
	if (count>0)
	{
		for (c1=0;c1<variableClauseSets[v]->numOfElements;c1++)
		{
			c2 = variableClauseSets[v]->elements[c1];
			for (v1=0;v1<clauseVariableSets[c2]->numOfElements;v1++)
			{
				v2 = clauseVariableSets[c2]->elements[v1];
				if (varStatus[v2] == 0 && !intSetMemberOfU (vs,v2))
				{
					priority [v2] += weight/(1.0+clauseVariableSets[c2]->numOfElements-clauseVarCounts[c2]);
					addPriority (count-1, varStatus, priority, weight/3, v2, vs, clauseVariableSets, variableClauseSets, clauseVarCounts);
				}
			}
		}
	}
	intSetMemberOfRemoveU (vs, v);
}

void clauseWeightPermute1 (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v,v1,v2,v3,v4,v5,c,c1,
		swap, bestV=1,count, numLits;
	IntSetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, *neighbours, set, threeSet;
	ClausePtr clause;
	int *varStatus, *clauseStatus, *clauseVarCounts, *clauseStart;
	double *priority, *weight, bestPriority;

	int recentClauseCount=500;

	
	ListPtr recentClauses;
	LinkPtr link, nextLink;
	IntSetPtr thisClause;

	recentClauses = newList();


	//printProblemSize (p->numOfClauses, p->clauses, numOfVariables, permute, unPermute);

	priority = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	weight = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	varStatus = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	clauseStatus = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseStart = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVarCounts = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVariableSets = (IntSetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(IntSetPtr),"neighbours");

	variableClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	variableMaxClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	neighbours = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");

	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newIntSet();
		variableMaxClauseSets[v] = newIntSet();
		neighbours[v] = newIntSet();
	}

	numLits = 0;
	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newIntSet();
		clause = p->clauses[c];
		numLits += clause->numOfVariables;
		clauseVarCounts[c] = 0;
		clauseStatus[c] = 0;
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToIntSet (variableClauseSets[clause->variables[v]], c);
			addToIntSet (clauseVariableSets[c], clause->variables[v]);
			for (v1=0;v1<clause->numOfVariables;v1++)
				if (v != v1)
					addToIntSet (neighbours[clause->variables[v]], clause->variables[v1]);
		}
	}
	if (numLits > 20000 ||p->numOfClauses/numOfVariables > 30 )
		count= 1;
	else if (numLits > 10000 || p->numOfClauses/numOfVariables > 5|| numLits/p->numOfClauses > 6)
		count= 2;
	else
		count = 3;
	printf ("c count = %d\n", count);


	for (v=1;v<=numOfVariables;v++)
	{
		priority[v] = 0.0;
		weight[v] = 0.0;
		varStatus[v] = 0;
		priority[v] =  0;
	}

	set = newIntSet();
	threeSet = newIntSet();

	for (v=1;v<=numOfVariables;v++)
	{
		//printf ("%d\n",v);fflush(NULL);
		if (v==1)
		{
			bestV = 1;
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
			clearIntSet (set);
			link = recentClauses->first;
			for (c=0;c<recentClauseCount && link != NULL; (c++,link=link->next))
			{
				thisClause = (IntSetPtr) link->element;
				for (v1=0;v1<thisClause->numOfElements;v1++)
				{
					v2 = thisClause->elements[v1];
					if (varStatus[v2] == 0)
						addToIntSet (set, v2);
				}
			}
			
			bestV = set->elements[0];
			bestPriority = priority[bestV];
			for (v1=1;v1<set->numOfElements;v1++)
			{
				v2 = set->elements[v1];
				if (priority[v2] > bestPriority)
				{
					bestV = v2;
					bestPriority = priority[v2];
				}
			}
			if (v % 1 == 0)
				for (v1=1;v1<=numOfVariables;v1++)
					priority[v1] *= 0.995;
					
		   //printf ("      <%d>",bestV);

		}
/*
		bestV = 1;
		while (varStatus[bestV] != 0) bestV++;
		bestPriority = priority[bestV];
		for (v1=bestV+1;v1<=numOfVariables;v1++)
		{
			if (varStatus[v1] == 0 && priority[v1] > bestPriority)
			{
				bestV = v1;
				bestPriority = priority[v1];
			}
		}
		
*/
		   varStatus [bestV] = 1;
		clearIntSet (threeSet);
		 addPriority (count, varStatus, priority, 100.0, bestV, threeSet, clauseVariableSets, variableClauseSets, clauseVarCounts);

		v2 = 0;
		for (c=0;c<variableClauseSets[bestV]->numOfElements;c++)
		{
			c1 = variableClauseSets[bestV]->elements[c];
			for (v2=0;false && v2<clauseVariableSets[c1]->numOfElements;v2++)
			{
				v3 = (int)clauseVariableSets[c1]->elements[v2];
				priority[v3] += 10.0/( 2.0+clauseVariableSets[c1]->numOfElements-clauseVarCounts[c1]);
				for (v4=0;v4<neighbours[v3]->numOfElements;v4++)
				{
					v5 = (int)neighbours[v3]->elements[v4];
					priority[v5] += 0.5;
				
				}
			
			}
			clauseVarCounts[c1]++;
			if (clauseVarCounts[c1] == clauseVariableSets[c1]->numOfElements)
				v2++;
			if (clauseStatus[c1] == 0)
			{
				clauseStatus[c1] = 1;
				clauseStart[c1] = v;
				addListFirst (recentClauses, newCopyIntSet(clauseVariableSets[c1]));
			}
		}
	   // printf ("Level %d, variable %d, closed %d\n", v, bestV, v2);

		for (link = recentClauses->first;link != NULL;)
		{
			thisClause = (IntSetPtr) link->element;
			nextLink = link->next;
			intSetMemberOfRemoveU (thisClause, bestV);
			if (thisClause->numOfElements == 0)
			{
				removeList (recentClauses, link);
				freeIntSet (thisClause);
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
		freeIntSet (variableClauseSets[v]);
	}
	free (variableClauseSets);

	for (c=0;c<p->numOfClauses;c++)
	{
		freeIntSet (clauseVariableSets[c]);
	}
	free (clauseVariableSets);
	freeList (recentClauses);
}

typedef struct AssocEntryStruct
{
	int v, w;
} AssocEntry, *AssocEntryPtr;

AssocEntryPtr newAssocEntry(int v, int w)
{
	AssocEntryPtr result = (AssocEntryPtr) checkMalloc (sizeof(AssocEntry),"assoclist");
	result->v = v;
	result->w = w;
	return result;
}

SetPtr *assocLists;

void increaseAssocWeight (int v1, int v2, int w)
{
	SetPtr s;
	int i,found;
	
	//printf ("x %d %d %d\n", v1, v2, w);
	s = assocLists[v1];
	found = false;
	for (i=0;!found && i<s->numOfElements;i++)
	{
		found = ((AssocEntryPtr) s->elements[i])->v == v2;
	}
	if (found)
		((AssocEntryPtr) s->elements[i-1])->w += w;
	else
		addToSetNoCheck (s, newAssocEntry (v2,w));
}

void clauseWeightPermute2 (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v,v1,v2,v3,v4,v5,v6,c,c1,c2,c3,
		swap, bestV=1;
	IntSetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, *neighbours, set;
	ClausePtr clause,clause1,clause2,clause3;
	int *varStatus, *clauseStatus, *clauseVarCounts, *clauseStart;
	double *priority, *weight, bestPriority;
	ListPtr recentClauses;
	LinkPtr link, nextLink;
	IntSetPtr thisClause;
	AssocEntryPtr e;

	int recentClauseCount;
	if (recentClauseCount < 50) recentClauseCount = 50;
	recentClauseCount = 50;



	recentClauses = newList();



	//printProblemSize (p->numOfClauses, p->clauses, numOfVariables, permute, unPermute);

	priority = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	weight = (double *) checkMalloc ( (1+numOfVariables)*sizeof(double),"neighbours");
	varStatus = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	clauseStatus = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseStart = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVarCounts = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVariableSets = (IntSetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(IntSetPtr),"neighbours");

	variableClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	variableMaxClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	neighbours = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");

	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newIntSet();
		variableMaxClauseSets[v] = newIntSet();
		neighbours[v] = newIntSet();
	}


	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newIntSet();
		clause = p->clauses[c];
		clauseVarCounts[c] = 0;
		clauseStatus[c] = 0;
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToIntSet (variableClauseSets[clause->variables[v]], c);
			addToIntSet (clauseVariableSets[c], clause->variables[v]);
			for (v1=0;v1<clause->numOfVariables;v1++)
				if (v != v1)
					addToIntSet (neighbours[clause->variables[v]], clause->variables[v1]);
		}
	}

	assocLists = (SetPtr *) checkMalloc ((1+numOfVariables)*sizeof(SetPtr),"");
	for (v=1;v<=numOfVariables;v++)
		assocLists[v] = newSet();
		
	for (v=1;v<=numOfVariables;v++)
	{
		for (c1=0;c1<variableClauseSets[v]->numOfElements;c1++)
		{
			clause1 = p->clauses[variableClauseSets[v]->elements[c1]];
			for (v1=0;v1<clause1->numOfVariables;v1++)
			{
				v2 = clause1->variables[v1];
				increaseAssocWeight (v,v2,4);
				for (c2=0;c2<variableClauseSets[v2]->numOfElements;c2++)
				{
					clause2 = p->clauses[variableClauseSets[v2]->elements[c2]];
					if (clause2 != clause1)
					{
						for (v3=0;v3<clause2->numOfVariables;v3++)
						{
							v4 = clause2->variables[v3];
							increaseAssocWeight (v,v4,2);
							 for (c3=0;c3<variableClauseSets[v4]->numOfElements;c3++)
							{
								clause3 = p->clauses[variableClauseSets[v4]->elements[c3]];
								if (clause3 != clause1 && clause3 != clause2)
								{
									for (v5=0;v5<clause3->numOfVariables;v5++)
									{
										v6 = clause3->variables[v5];
										increaseAssocWeight (v,v6,1);
									}
								}
							}
					   }
				   }
				}
			}
		}
	}    
	
	printf ("c lists built\n"); fflush (stdout);
//    for (v=1;v<=numOfVariables;v++)
//    {
//        printf ("v = %d\n",v);
//        for (v1=0;v1<assocLists[v]->numOfElements;v1++)
//        {
//            e = assocLists[v]->elements[v1];
//            printf("%d -> %d ", e->v, e->w);
//            printf ("\n");
//        }
//    }
	for (v=1;v<=numOfVariables;v++)
	{
		priority[v] = 1.0;
		weight[v] = 0.0;
		varStatus[v] = 0;

	}

	for (v=1;v<=numOfVariables;v++)
	{
		bestV = 1;
		while (varStatus[bestV] != 0) bestV++;
		bestPriority = priority[bestV];
		for (v1=bestV+1;v1<=numOfVariables;v1++)
		{
			
			if (varStatus[v1] == 0 && priority[v1] > bestPriority)
			{
				bestV = v1;
				bestPriority = priority[bestV];
			}
		}
		varStatus[bestV] = 1;
		permute[bestV] = v;
		unPermute[v] = bestV;
		
		for (v1=0;v1<assocLists[bestV]->numOfElements;v1++)
		{
			e = assocLists[bestV]->elements[v1];
			priority[e->v] += (e->w);
		}
		
	}

return;

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
			clearIntSet (set);
			link = recentClauses->first;
			for (c=0;c<recentClauseCount && link != NULL; (c++,link=link->next))
			{
				thisClause = (IntSetPtr) link->element;
				for (v1=0;v1<thisClause->numOfElements;v1++)
				{
					v2 = thisClause->elements[v1];
					if (varStatus[v2] == 0)
						addToIntSet (set, v2);
				}
			}
			bestV = set->elements[0];
			bestPriority = priority[bestV];
			for (v1=1;v1<set->numOfElements;v1++)
			{
				v2 = set->elements[v1];
				if (priority[v2] > bestPriority)
				{
					bestV = v2;
					bestPriority = priority[v2];
				}
			}
			if (v % 100  == 0)
				for (v1=1;v1<=numOfVariables;v1++)
					priority[v1] /=1.01;
					
		   //printf ("      <%d>",bestV);

		}

		v2 = 0;
		for (c=0;c<variableClauseSets[bestV]->numOfElements;c++)
		{
			c1 = variableClauseSets[bestV]->elements[c];
			
			for (v2=0;v2<clauseVariableSets[c1]->numOfElements;v2++)
			{
				v3 = (int)clauseVariableSets[c1]->elements[v2];
				priority[v3]+= 10.0 /( 0.5 + clauseVariableSets[c1]->numOfElements-clauseVarCounts[c1]) ;
				
			}
			clauseVarCounts[c1]++;
			if (clauseStatus[c1] == 0)
			{
				clauseStatus[c1] = 1;
				clauseStart[c1] = v;
				addListFirst (recentClauses, newCopyIntSet(clauseVariableSets[c1]));
			}
		}
	   // printf ("Level %d, variable %d, closed %d\n", v, bestV, v2);

		for (link = recentClauses->first;link != NULL;)
		{
			thisClause = (IntSetPtr) link->element;
			nextLink = link->next;
			intSetMemberOfRemoveU (thisClause, bestV);
			if (thisClause->numOfElements == 0)
			{
				removeList (recentClauses, link);
				freeIntSet (thisClause);
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
		freeIntSet (variableClauseSets[v]);
	}
	free (variableClauseSets);

	for (c=0;c<p->numOfClauses;c++)
	{
		freeIntSet (clauseVariableSets[c]);
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
	return 1+((double)rand())*(numOfVariables)/((unsigned int)RAND_MAX+(unsigned int)1);
}

#define iabs(i) (i>0?i:-i)

#define contrib(a,b,n) ((a-b))
//#define contrib(a,b,n) ((a-b))

int maxWeight;

double weightedClause (int max, int min, int numOfVariables)
{
	double result;
	
	
	//result = -((double) 10*numOfVariables*numOfVariables)/( (double) 10*numOfVariables + (max - min));
	result = pow ( (double) (max-min), (double) 1.0);
	
	return result;
}


double clauseGainWhenFlip2 (ClausePtr clause, int v1, int v2, int *permute, int numOfVariables)
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

double clauseSetGainWhenFlip2 (IntSetPtr clauses, int v1, int v2, ClausePtr *allClauses, int *permute, int numOfVariables)
{
	int c1, c2;
	ClausePtr clause;
	double result;

	result = 0;
	for (c1=0;c1<clauses->numOfElements;c1++)
	{
		c2 = clauses->elements[c1];
		clause = allClauses[c2];
		result += clauseGainWhenFlip2 (clause, v1, v2, permute, numOfVariables);
	}
	return result;
}

double gainWhenFlip2 (int v1, int v2, ClausePtr *allClauses, int *permute, IntSetPtr *variableClauseSets, int numOfVariables)
{
	double result;
	IntSetPtr clauses;

	result = 0;
	clauses = newIntSet();
	copyIntSet (clauses, variableClauseSets[v1]);
	differenceToIntSet (clauses, variableClauseSets[v2]);
	result += clauseSetGainWhenFlip2 (clauses, v1, v2, allClauses, permute, numOfVariables);
	copyIntSet (clauses, variableClauseSets[v2]);
	differenceToIntSet (clauses, variableClauseSets[v1]);
	result += clauseSetGainWhenFlip2 (clauses, v2, v1, allClauses, permute, numOfVariables);

	//printf ("%d ", result);
	freeIntSet (clauses);
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
			result = rand() < p*RAND_MAX;
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
void combinedPermute (int init, PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v,v1,v2,v3,v4,v5,c,
		swap;
	IntSetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, *neighbours;
	ClausePtr clause;
	int *status;
	double temperature, alpha, gain, bestGain, currentGain;
	int swapCount, defaultSwapCount;
	int maxNumOfVariables=200, maxNumOfClauses=500;
	int *newPermute, *newUnPermute, *posWeightDelta, *leftCutDelta, *middleCutDelta, *rightCutDelta, *bestPermute;
	int arrangementDelta, cutDelta, cutDelta1,
		bestArrangementDelta, previousArrangementDelta=0,
		newPos, bestNewPos, maxSize, size, localSwap;
	int insideMin, insideMax, outsideMin, outsideMax;
	int leftCut, middleCut, rightCut;
	
	IntSetPtr vn;



	if (maxNumOfVariables > numOfVariables)
		maxNumOfVariables = numOfVariables;
		
	if (maxNumOfClauses > p->numOfClauses)
		maxNumOfClauses = p ->numOfClauses;



	//printProblemSize (p->numOfClauses, p->clauses, numOfVariables, permute, unPermute);

	status = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	clauseVariableSets = (IntSetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(IntSetPtr),"neighbours");

	variableClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	variableMaxClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	neighbours = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	leftCutDelta = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	middleCutDelta = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	rightCutDelta = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	newPermute = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	newUnPermute = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	posWeightDelta = (int *) checkMalloc ( (2+numOfVariables)*sizeof(int),"neighbours");
	bestPermute = (int *) checkMalloc ( (1+numOfVariables)*sizeof(int),"neighbours");
	
	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newIntSet();
		variableMaxClauseSets[v] = newIntSet();
		neighbours[v] = newIntSet();
	}


	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newIntSet();
		clause = p->clauses[c];
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToIntSet (variableClauseSets[clause->variables[v]], c);
			addToIntSet (clauseVariableSets[c], clause->variables[v]);
			for (v1=0;v1<clause->numOfVariables;v1++)
				if (v != v1)
					addToIntSet (neighbours[clause->variables[v]],clause->variables[v1]);
		}
	}


	if (init)
	{
	   for (v=1;v<=numOfVariables;v++)
		{
			permute[v] = v;
			unPermute[v] = v;
		}
	}
	for (v=1;v<=numOfVariables;v++)
	{
		bestPermute[v] = permute[v];
	}
	
	bestGain = 0;
	currentGain = 0;

	temperature = numOfVariables;
	alpha = 0.99;
	if (init)
		defaultSwapCount = maxNumOfClauses*pow((double)maxNumOfVariables,2)/5000;
	else
		defaultSwapCount = maxNumOfVariables*10;
	
	//defaultSwapCount = 10*pow((double)maxNumOfVariables,1);
	//defaultSwapCount = 10000;
	for (v4=0;init && v4<1;v4++)
	{
	if (init)
		temperature = 5*numOfVariables;
	else
		temperature = 5;
	while (temperature > 0.2)
	{
			
		swapCount = defaultSwapCount;

		
		for (swapCount=0;swapCount < defaultSwapCount; swapCount++)


		//while (rollingAverage < 0.0)

		{
			v1 = randomVariable (numOfVariables);
			if (temperature > 0)
				v2 = randomVariable (numOfVariables);
			else
			{
				if (v1 > 1) vn = neighbours[v1-1]; else vn = neighbours[v1+1];
					v2 =  vn->elements[randomVariable (vn->numOfElements)-1];
			}
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
						
						currentGain += gain;
						if (currentGain < bestGain)
						{
							bestGain = currentGain;
							for (v1=1;v1<=numOfVariables;v1++)
								bestPermute[v1] = permute[v1];
						}
					}
				}

		}
		temperature *= alpha;
	}
	}

	for (v1=1;v1<=numOfVariables;v1++)
		permute[v1] = bestPermute[v1];   
	for (v1=1;v1<=numOfVariables;v1++)
		unPermute[permute[v1]] = v1;   




	for (v4=1;v4<numOfVariables;v4++)
		for (v5=v4+1;v5<v4+5 && v5<=numOfVariables;v5++)
		{
			v1 = unPermute[v4];
			v2 = unPermute[v5];
			gain = gainWhenFlip2 (v1, v2, p->clauses, permute, variableClauseSets, numOfVariables);
			if (gain < 0)
			{
				printf (".");;
				v3 = permute[v1];
				permute[v1] = permute[v2];
				permute[v2] = v3;
				unPermute[permute[v1]] = v1;
				unPermute[permute[v2]] = v2;
			}
		}
   

	swap = true;
	maxSize = 0;
	while (true &&(swap || maxSize < 20))
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
		freeIntSet (variableClauseSets[v]);
	}
	free (variableClauseSets);

	for (c=0;c<p->numOfClauses;c++)
	{
		freeIntSet (clauseVariableSets[c]);
	}
	free (clauseVariableSets);


}
double newSetPriority (int s, int v, int *clauseStatus, int *clauseStart, IntSetPtr clauses, IntSetPtr *clauseVariableSets, IntSetPtr thisSet)
{
	double result = 0;
	int c,c1;
	IntSetPtr temp = newIntSet();

	for (c=0;c<clauses->numOfElements;c++)
	{
		c1 = clauses->elements[c];
		if (clauseStatus[c1] == 0)
		{
			result -= 1;
		}
		else
		{
			copyIntSet (temp, clauseVariableSets[c1]);
			differenceToIntSet (temp, thisSet);
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

	freeIntSet(temp);
	return result;
}

double newVarPriority (int v, int *clauseStatus, IntSetPtr clauses, int *clauseVarCounts, SetPtr *clauseVariableSets)
{
	double result = 0;
	int c,c1;

	for (c=0;c<clauses->numOfElements;c++)
	{
		c1 = clauses->elements[c];
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



int findArrangement (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v,v1,c,c1,outsideMin,outsideMax,leftCut,maxCut,v2,v3,*clauseLengths;
	IntSetPtr *variableClauseSets, *clauseVariableSets, *variableMaxClauseSets, set;
	ClausePtr clause;
	int *clauseStatus, *clauseVarCounts;

	int arrangement;


	clauseStatus = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVarCounts = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseLengths = (int *) checkMalloc ( (p->numOfClauses)*sizeof(int),"neighbours");
	clauseVariableSets = (IntSetPtr *) checkMalloc ( (1+p->numOfClauses)*sizeof(IntSetPtr),"neighbours");

	variableClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");
	variableMaxClauseSets = (IntSetPtr *) checkMalloc ( (1+numOfVariables)*sizeof(IntSetPtr),"neighbours");

	for (v=1;v<=numOfVariables;v++)
	{
		variableClauseSets[v] = newIntSet();
		variableMaxClauseSets[v] = newIntSet();
	}


	for (c=0;c<p->numOfClauses;c++)
	{
		clauseVariableSets[c] = newIntSet();
		clause = p->clauses[c];
		clauseVarCounts[c] = clause->numOfVariables;
		clauseLengths[c] = 0;
		clauseStatus[c] = 0;
		for (v=0;v<clause->numOfVariables;v++)
		{
			addToIntSet (variableClauseSets[clause->variables[v]], c);
			addToIntSet (clauseVariableSets[c], clause->variables[v]);
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

void reverseOrder (int *permute, int *unPermute, int n)
{
	int temp,i;
	
	for (i=0;i<n/2;i++)
	{
		temp = permute[i+1];
		permute[i+1] = permute[n-(i)];
		permute[n-(i)] = temp;
	}
	
	for (i=1;i<=n;i++)
		unPermute[permute[i]] = i;
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

typedef struct FORCEItemStruct
{
	double cog;
	int variable;
} FORCEItem, *FORCEItemPtr;

FORCEItemPtr newForceItem()
{
	FORCEItemPtr result;
	
	result = (FORCEItemPtr) checkMalloc (sizeof(FORCEItem),"ForceItem");
	
	return result;
}

int FORCEItemCompare1 (const void *pv1, const void *pv2)
{
	FORCEItemPtr v1,v2;
	v1 = *( (FORCEItemPtr *) pv1);
	v2 = *( (FORCEItemPtr *) pv2);

	//printf ("%d - %d\n", v1->variable, v2->variable);
	
	if (v1->cog > v2->cog)
		return -1;
	else if (v1->cog == v2->cog)
		return 0;
	else
		return 1;

}

int FORCEItemCompare (const void *pv1, const void *pv2)
{
	FORCEItemPtr v1,v2;
	v1 = *( (FORCEItemPtr *) pv1);
	v2 = *( (FORCEItemPtr *) pv2);

	//printf ("%d - %d\n", v1->variable, v2->variable);
	
	if (v1->cog > v2->cog)
		return 1;
	else if (v1->cog == v2->cog)
		return 0;
	else
		return -1;

}

void findPermutation (PropositionPtr p, int **d, int **initialD, int **r, int numOfVariables, int *permute, int *unPermute)
{
	int v;
	FILE *f;
	int method;
	int numLits=0,c;
	char comment[100];

	for (c=0;c<p->numOfClauses;c++)
		numLits += p->clauses[c]->numOfVariables;
		
	for (v=1;v<=numOfVariables;v++)
		unPermute[v] = v;

	method = intOptions[PERMUTATION_METHOD];
	if (method == 100)
	{
		if (numLits < 3000 && 4*numOfVariables >=  p->numOfClauses)
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


			case 3:
				combinedPermute (true, p, d, initialD, r, numOfVariables, permute, unPermute);
				break;

			case 4:
				clauseWeightPermute1 (p, d, initialD, r, numOfVariables, permute, unPermute);
				break;

			case 5:
				clauseWeightPermute1 (p, d, initialD, r, numOfVariables, permute, unPermute);
				combinedPermute (false, p, d, initialD, r, numOfVariables, permute, unPermute);
				break;
			case 6:
				clauseWeightPermute2 (p, d, initialD, r, numOfVariables, permute, unPermute);
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
