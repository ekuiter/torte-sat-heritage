#include "proposition.h"
#include "set.h"
#include "dimacs.h"
#include "math.h"
#include "tts.h"
#include "options.h"

#ifdef PROOF
#include "proof.h"
#endif

void sortProposition (PropositionPtr p);

void initPropositions()
{
	trueProposition = newProposition(1);
	falseProposition = newProposition(1);
}

PropositionPtr trueProposition, falseProposition;

PropositionPtr propositionExcluding (PropositionPtr p, int v)
{
	PropositionPtr result;
	int c;

	result = newProposition(1);
	for (c=0;c<p->numOfClauses;c++)
		if (!clauseContains (p->clauses[c],v))
			addPropositionClause (result,p->clauses[c]);

	if (result->numOfClauses == 0)
	{
		freeProposition (result);
		result = trueProposition;
	}

	return result;
}

PropositionPtr propositionCurried (PropositionPtr p, int v, int positive)
{
	PropositionPtr result;
	int c,finished;
	ClausePtr newClause;

	finished = false;
	result = newProposition(1);
	for (c=0;!finished&&c<p->numOfClauses;c++)
		if (clauseContains (p->clauses[c],v))
		{
			newClause = clauseCurried (p->clauses[c], v, positive);
			if (newClause == falseClause)
			{
				freeProposition (result);
				result = falseProposition;

				finished = true;
			}
			else if (newClause != trueClause)
				addPropositionClause (result,newClause);
		}

	if (result != falseProposition && result->numOfClauses == 0)
	{
		freeProposition (result);
		result = trueProposition;
	}

	return result;
}



PropositionPtr newProposition(int initialCapacity)



{



	PropositionPtr result;



	result = checkMalloc (sizeof (Proposition), "newProposition");



	result->numOfClauses = 0;



	result->maxVariable = 0;
	result->capacity = initialCapacity;

	result->clauses

 = checkMalloc (sizeof (ClausePtr)*result->capacity, "newProposition.1");



	return result;

}





void freeProposition (PropositionPtr proposition)

{

	int i;

	if (proposition == trueProposition || proposition == falseProposition)
		return;
		
		
	for (i=0; i<proposition->numOfClauses; i++)

		freeClause (proposition->clauses[i]);

	free (proposition->clauses);

	free (proposition);

}



void freePropositionList (ListPtr list)
{
	LinkPtr link;
	
	for (link=list->first;link!=NULL;link=link->next)
		freeProposition (link->element);
		
	freeList (list);
}



int propositionSize (PropositionPtr proposition)
{
	int result;
	if (proposition == trueProposition || proposition == falseProposition)
		return 0;
	else
	{
		result = 1;
	}
	return result;
}

double propositionListSize (ListPtr list)
{
	double result;
	int s;
	LinkPtr link;
	result = 0.0;
	if (list->length < 2)
		return 0.0;
		
	for (link=list->first->next;link!=NULL;link=link->next)
	{
		s = getPropositionSize (link->element);
		result += s;
	}
	


/*
	printf ("Size of \n");
	for (link=list->first;link!=NULL;link=link->next)
		printf (" = %s\n\n",toStringProposition(link->element));
	printf ("= %f\n\n",result);   
*/
	return result;
	
}

void addPropositionClause (PropositionPtr proposition, ClausePtr clause)
{
	proposition->numOfClauses++;
	if (proposition->numOfClauses > proposition->capacity)
	{
		proposition->capacity = 1.5*(proposition->capacity) + 1;
		proposition->clauses = checkRealloc (proposition->clauses,proposition->capacity*sizeof(ClausePtr),"addPropositionClause.1");
	}
	proposition->clauses[proposition->numOfClauses-1] = clause;
	if (clause->maxVariable > proposition->maxVariable)
		proposition->maxVariable = clause->maxVariable;
}







int getPropositionSize(PropositionPtr proposition)



{



	int result,i;



	result = 0;



	for (i=0;i<proposition->numOfClauses;i++)



		if (proposition->clauses[i] != NULL)
			result += proposition->clauses[i]->numOfVariables;



	return result;



}







static char *stringPropositionBuffer = NULL;
static int stringPropositionBufferLength = 0;

char *toStringProposition(PropositionPtr proposition)



{



	int i,length;

	if (proposition == trueProposition)
		return "True";
	else if (proposition == falseProposition)
		return "False";

	length = 2 + 6*getPropositionSize(proposition) + 2*proposition->numOfClauses;



	if (stringPropositionBuffer == NULL)



	{



		stringPropositionBuffer = checkMalloc (length*sizeof(char), "toStringProposition");



		stringPropositionBufferLength = length;



	}



	else if (length > stringPropositionBufferLength)



	{



		stringPropositionBuffer = checkRealloc (stringPropositionBuffer,length*sizeof(char), "toStringProposition.1");



		stringPropositionBufferLength = length;



	}



	strcpy (stringPropositionBuffer,"");


	if (proposition == trueProposition)
		strcat (stringPropositionBuffer, "True");
	else
		for (i=0; i<proposition->numOfClauses; i++)
			strcat (stringPropositionBuffer, toStringClause (proposition->clauses[i]));



	return stringPropositionBuffer;



}


PropositionPtr removePropositionDuplicates (PropositionPtr p)
{
	int i;
	if (p != trueProposition)
		for (i=0;i<p->numOfClauses;i++)
			removeClauseDuplicates (p->clauses[i]);
			
	return p;
}

PropositionPtr removePropositionTautologies (PropositionPtr p)
{
	int i,j;
	if (p != trueProposition)
	{
		i=0; j=0;
		while (i < p->numOfClauses)
		{
			if (!isClauseTautology (p->clauses[i]))
			{
				if (i != j)
					p->clauses[j] = p->clauses[i];
				j++;
			}
			i++;
		}
		p->numOfClauses = j;
		if (p->numOfClauses == 0)
			p = trueProposition;
	}       
	return p;
}

int countUsed (PropositionPtr p, int maxVariable, int *globalUsed)
{
	int result, v,c,v1;
	ClausePtr clause;
	result = 0;

	for (v=1;v<=maxVariable;v++)
	{
		globalUsed[v] = false;
	 }

	for (c=0;c<p->numOfClauses;c++)
	{
		clause = p->clauses[c];
		for (v=0;v<clause->numOfVariables;v++)
		{
			v1 = clause->variables[v];
			globalUsed[v1] = true;
		}
	}
	
	for (v=1;v<=maxVariable;v++)
		if (globalUsed[v])
			result++;
			
	return result;
}

PropositionPtr removePropositionSingletons (PropositionPtr p, int maxVariable,
											int *mapVariable, int *unMapVariable, int *used,
											int *singletonUsage, int *isSingleton,
											int *unitClauseUsage, int *isUnitClause)
{
	int v,v1,v2, vv,c,lastUsed=0, newSingleton, newUnitClause, c1, c2, clauseNeeded, singletonCount;
	ClausePtr clause;
	
#ifdef PROOF
	long int *unitVarResIds;
	long int newResId;
#endif
	
#ifdef PROOF
	unitVarResIds = (long int *) checkMalloc ( (1+maxVariable)*sizeof(long int),"");
#endif  
	
	for (v=1;v<=maxVariable;v++)
	{
		isSingleton[v] = false;
		isUnitClause[v] = false;
		mapVariable[v] = 0;
	 }

	newSingleton = true;
	newUnitClause = false;
	while (newSingleton || newUnitClause)
	{
		for (v=1;v<=maxVariable;v++)
		{
			if (used[v] && !isSingleton[v])
				singletonUsage[v] = -1;
			if (used[v] && !isUnitClause[v])
				unitClauseUsage[v] = -1;
		}

		for (c=0;c<p->numOfClauses;c++)
		{
			clause = p->clauses[c];
/*
			printf ("<");
			for (v1=0;v1<clause->numOfVariables;v1++)
				printf ("%d ", clause->positive[v1]?clause->variables[v1]:-clause->variables[v1]);
			printf (">\n");
*/
			if (clause->numOfVariables == 1)
			{
				v1 = clause->variables[0];
				if (used[v1])
				{
					if (unitClauseUsage[v1] == -1)
					{

						unitClauseUsage[v1] = clause->positive[0];
#ifdef PROOF
						unitVarResIds[v1] = clause->resId;
#endif
						

					}
				}
			}
			for (v=0;v<clause->numOfVariables;v++)
			{
				v1 = clause->variables[v];
				if (used[v1])
				{
					if (singletonUsage[v1] == -1)
						singletonUsage[v1] = clause->positive[v];
					else if (singletonUsage[v1] != -2)
						if (singletonUsage[v1] != clause->positive[v])
							singletonUsage[v1] = -2;
				}
			}
		}
		
		c1 = 0;
		c2 = 0;
		while (c1 < p->numOfClauses)
		{
			clause = p->clauses[c1];
			clauseNeeded = true;

/*
			printf ("(");
			for (v1=0;v1<clause->numOfVariables;v1++)
				printf ("%d ", clause->positive[v1]?clause->variables[v1]:-clause->variables[v1]);
			printf (")\n");
*/

			v1=0;
			v2=0;
			
			while (v1 < clause->numOfVariables && clauseNeeded)
			{
				vv = clause->variables[v1];
				if (used[vv] && !isUnitClause[vv] && (unitClauseUsage[vv] == clause->positive[v1]))
					clauseNeeded = false;
				else if (used[vv] && !isUnitClause[vv] && (unitClauseUsage[vv] == 1-clause->positive[v1]))
				{
#ifdef PROOF
					newResId = nextResId++;
					if (booleanOptions[RPT_FILE])
					{
						resolveProofClauses (clause->positive[v1]?globalUnMapVariable[vv]:-globalUnMapVariable[vv]
						, unitVarResIds[vv],clause->resId, newResId);
						clause->resId = newResId;
					}
#endif
				}    
				else if (used[vv] && !isSingleton[vv] && singletonUsage[vv] == -2)
				{
					if (v1 != v2)
					{
						clause->variables[v2] = clause->variables[v1];
						clause->positive[v2] = clause->positive[v1];
					}
					v2++;
				}
				else if (used[vv] && (singletonUsage[vv] == clause->positive[v1] || singletonUsage[vv] == -1))
					clauseNeeded = false;
				else
				{
				}
				v1++;
			}
			clause->numOfVariables = v2;
			if (clauseNeeded && clause->numOfVariables == 0)
			{
				return falseProposition;
			}
			if (clauseNeeded && clause->numOfVariables > 0)
			{
				if (c1 != c2)
					p->clauses[c2] = p->clauses[c1];
				c2++;
			}
			c1++;
		}
		p ->numOfClauses = c2;
		newSingleton = false;
		newUnitClause = false;
		for (v=1;v<=maxVariable;v++)
		{
			if (used[v] && !isSingleton[v] && (singletonUsage[v] == -1 || singletonUsage[v] == 0 || singletonUsage[v] == 1))
			{
				newSingleton = true;
				isSingleton[v] = true;
			}
			if (used[v] && !isUnitClause[v] && (unitClauseUsage[v] == 0 || unitClauseUsage[v] == 1))
			{
				newUnitClause = true;
				isUnitClause[v] = true;
			}
		}
	}

	singletonCount = 0;
	   
	lastUsed = 0;
	for (v=1;v<=maxVariable;v++)
		if (used[v] && !isSingleton[v] && !isUnitClause[v])
		{
			unMapVariable[++lastUsed] = v;
			mapVariable[v] = lastUsed;
		}

			
	if (p->numOfClauses == 0)
	{
		p = trueProposition;
	}  

#ifdef PROOF
	free (unitVarResIds);
#endif

	return p;
}


PropositionPtr mapProposition (PropositionPtr p, int *map)
{
	ClausePtr clause;
	int c, clauseCount;
	PropositionPtr result;
	ClausePtr *newClauses;
	
	newClauses = (ClausePtr *) checkMalloc (p->numOfClauses*sizeof(ClausePtr),"Clauses");
	
	//result = newProposition(p->numOfClauses);
	
	clauseCount = 0;
	
	for (c=0; c<p->numOfClauses; c++)
	{
		clause = mapClause (p->clauses[c], map);
		if (clause != NULL)
		{
			//addPropositionClause (result, clause);
			newClauses[clauseCount++] = clause;
			p->clauses[c] = NULL;
		}
	}
	
	if (clauseCount == 0)
	{
		//freeProposition(result);
		result = trueProposition;
	}
	else
	{
		result = newProposition(clauseCount);
		for (c=0;c<clauseCount;c++)
			addPropositionClause (result, newClauses[c]);
	}

	free (newClauses);
	
	return result;
}


int evaluateProposition (PropositionPtr proposition, AssignmentPtr assignment)



{



	int stillTrue,i;



	stillTrue = true;



	for (i=0; i<proposition->numOfClauses && stillTrue; i++)



		stillTrue = evaluateClause(proposition->clauses[i],assignment);



	return stillTrue;



}

int weightedSum (int *a, int numOfElements)
{
	int result,i;
	result = 0;
	for (i=0;i<numOfElements;i++)
		result = result*4 + a[i];
	return result;
}



int *mapVariable = NULL, *unMapVariable = NULL;

int indexOfSmallest (int *a, int count)
{
	int i, result, smallest;

	for (result=0;a[result]==0;result++);
	smallest = a[result];

	for (i=1;i<=count;i++)
		if (0 <a[i] && a[i] < smallest)
		{
			result = i;
			smallest = a[i];
		}

	return result;
}

int mm1[]={0 ,2,12,17,33,9,  42,16,26,13,44, 23,30,10,35,45,
			29,31,3,43,46,  19,28,6,32,41,  27,24,11,15,38,
			22,25,18,21,8,  1,40,7,5,4,     14,20,34,36,37,39};

int mm2[]={0 ,
			18,8,29,16,3,  41,43,
			36,11,30,50,20,
			6,9,35,42,38,  32,15,10,48,
			47,1,45,33,22,  49,34,12,17,
			7,39,28,37,13, 46,21,4,24,23,
			40,44,2,27,19, 31,5,14,25,26  };

int *variablesUsed;
int **relatedVariable;


double weightForwards (int v, int v1,int *unMapVariable,int addedCount,int **linkage)
{
	double result;
	int i;
	result = 0;
	for (i=1;i<=addedCount-v1;i++)
		result += linkage[v][unMapVariable[v1+i]]*i*i;
	
	return result;
}

double weightBackwards (int v, int v1,int *unMapVariable,int addedCount,int **linkage)
{
	double result;
	int i;
	result = 0;
	for (i=1;i<v1;i++)
		result +=  linkage[v][unMapVariable[v1-i]]*i*i;
		
	return result;
}

double singleWeightedScore (int l, int linkage)
{
	double result = linkage * l * l;
	return result;
}

double weightedScore (int p, int v, int p1, int p2, int *unMapVariable,int addedCount,int **linkage)
{
	int i;
	double result = 0;
	for (i=1;i<=addedCount;i++)
		if (i != p1 && i != p2)
			result += singleWeightedScore (i>p?i-p:p-i, linkage[v][unMapVariable[i]]);
	return result;
}


int betterThan (int v,int addedCount, int v1,int v3,int *newUnMap,int **linkage)
{
	int i,k,result = true;
	for (i=addedCount;i>=v && result;i--)
	{
		k = i+v1-1;
		if ( k >= 1 && k <= addedCount && linkage[newUnMap[v3]][newUnMap[k]] > 0)
			result = false;
		k = i+v3-1;
		if ( k >= 1 && k <= addedCount && linkage[newUnMap[v1]][newUnMap[k]] > 0)
			result = false;
		k = i+v1-1;
		if ( k >= 1 && k <= addedCount && linkage[newUnMap[k]][newUnMap[v3]] > 0)
			result = false;
		k = i+v3-1;
		if ( k >= 1 && k <= addedCount && linkage[newUnMap[k]][newUnMap[v1]] > 0)
			result = false;
	 }
	return result;
}





int mapOverallCost (int *map, int **linkage, PropositionPtr p)
{
	int i,j,result,v1,c, max, min,found;
	int *m;
	ClausePtr clause;
	
	result = 0;
	for (c=0; c<p->numOfClauses;c++)
	{
		clause = p->clauses[c];
		max = 0;
		min = p->maxVariable;
		for (i=0;i<clause->numOfVariables;i++)
		{
			v1 = mapVariable[(int)(clause->variables[i])];
			found = false;
			m = map;
			
			for (j=1;v1!=*m++;j++);
			if (j > max)
				max = j;
			if (j<min)
				min = j;
		}
		result += max-min;
	}
	return result;     
}



int lowestCost;
SetPtr lowestSet = NULL;





int *usageValues, usedCount, originalNumOfVariables;



PropositionPtr readProposition (FILE *f)
{
	PropositionPtr result;
	ClausePtr clause;

	result = NULL;
	clause = readClause(f);

	if (clause != NULL)
		result = newProposition(1);

	while (clause != NULL)
	{
		addPropositionClause (result, clause);
		clause = readClause(f);
	}


	if (result != NULL)
	{
		mapVariable = checkRealloc (mapVariable,(result->maxVariable+1)*sizeof(int),"mapVariable");
		unMapVariable = checkRealloc (unMapVariable,(result->maxVariable+1)*sizeof(int),"unMapVariable");
	}

	return result;



}

PropositionPtr make3SAT (PropositionPtr p)
{
	PropositionPtr result;
	int c, maxVariable,extraVariables, v,v1, firstNewVariable;
	ClausePtr clause, extraClause;
	result = newProposition(1);
	
	maxVariable = p->maxVariable;
	//printf ("max variable = %d\n", maxVariable);
	
	for (c=0;c<p->numOfClauses;c++)
	{
		clause = p->clauses[c];
		assert (clause->numOfVariables <= 6);
		
		if (clause->numOfVariables <= 3)
			addPropositionClause (result, clause);
		else
		{
			//printf ("converting %s\n", toStringClause (clause));
			extraVariables = clause->numOfVariables/2;
			firstNewVariable = maxVariable+1;
			for (v=0;v<extraVariables;v++)
			{
				extraClause = newClause();
				for (v1=2*v;v1<2*(v+1);v1++)
					addClauseVariable (extraClause, clause->variables[v1]*
											(clause->positive[v1]?1:-1));
				addClauseVariable (extraClause, ++maxVariable);
				//printf ("adding %s\n", toStringClause(extraClause));
				addPropositionClause (result, extraClause);
			}
			extraClause = newClause();
			for (v=firstNewVariable;v<=maxVariable;v++)
				addClauseVariable(extraClause, -v);
			if (2*extraVariables != clause->numOfVariables)
					addClauseVariable (extraClause, clause->variables[clause->numOfVariables-1]*
											(clause->positive[clause->numOfVariables-1]?1:-1));
			addPropositionClause (result, extraClause);
				//printf ("e adding %s\n", toStringClause(extraClause));
		}
	}
	
	return result;
}


int equalPropositions (PropositionPtr p1, PropositionPtr p2)
{
	int c;
	
	if (p1 == p2)
		return true;
		
	for (c=0; c<p1->numOfClauses && c < p2->numOfClauses &&
			equalClauses (p1->clauses[c],p2->clauses[c]); c++);
			
	if (false && c == p1->numOfClauses && c == p2->numOfClauses && c > 1)
	{
		printf ("%s = \n",toStringProposition(p1));
		printf ("%s\n\n",toStringProposition(p1));
		
	}
	return c == p1->numOfClauses && c == p2->numOfClauses;
}

void sortProposition (PropositionPtr p)
{
	qsort (p->clauses, p->numOfClauses, sizeof (ClausePtr), clauseCompare);
}

void sortClauses (PropositionPtr p)
{
	int i;
	for (i=0;i<p->numOfClauses;i++)
	{
		sortClause (p->clauses[i]);
	}
}

static int *varPresent=NULL,varPresentSize=0;

int propositionListVarCount (ListPtr list, int variable)
{
	int v,c,result=0;
	ClausePtr clause;
	LinkPtr link;
	PropositionPtr proposition;
	
	link = list->length <= 1 ? NULL : list->first->next;
	
	for (;link!=NULL;link=link->next)
	{
		proposition = link->element;
		if (propositionContains (proposition, variable))
		{
			if (2*proposition->maxVariable>=varPresentSize)
			{
				varPresentSize = 2*proposition->maxVariable+1;
				varPresent = (int *)checkRealloc (varPresent,sizeof(int)*varPresentSize,"varPresent");
			}
			for (v=1;v<=2*proposition->maxVariable;v++)
				varPresent[v] = false;
				
			for (c=0;c<proposition->numOfClauses;c++)
			{
				clause = proposition->clauses[c];
				for (v=0;v<clause->numOfVariables;v++)
					if (clause->positive[v])
						varPresent[2*clause->variables[v]] = true;
					else
						varPresent[2*clause->variables[v]-1] = true;
			}
			result = 0;
			
			for (v=1;v<=2*proposition->maxVariable;v++)
				if (varPresent[v])
					result++;
		}    
	}
			
	return result;

}

int propositionVarCount (PropositionPtr proposition)
{
	int v,c,result;
	ClausePtr clause;
	
	if (2*proposition->maxVariable>=varPresentSize)
	{
		varPresentSize = 2*proposition->maxVariable+1;
		varPresent = (int *)checkRealloc (varPresent,sizeof(int)*varPresentSize,"varPresent");
	}
	for (v=1;v<=2*proposition->maxVariable;v++)
		varPresent[v] = false;
		
	for (c=0;c<proposition->numOfClauses;c++)
	{
		clause = proposition->clauses[c];
		for (v=0;v<clause->numOfVariables;v++)
			if (clause->positive[v])
				varPresent[2*clause->variables[v]] = true;
			else
				varPresent[2*clause->variables[v]-1] = true;
	}
	
	result = 0;
	
	for (v=1;v<=2*proposition->maxVariable;v++)
		if (varPresent[v])
			result++;
			
	return result;

}

int propositionContains (PropositionPtr proposition, int variable)
{
	int result,c;
	result = false;
	for (c=0;!result&&c<proposition->numOfClauses;c++)
		result = clauseContains (proposition->clauses[c],variable);
	return result;
}

double propositionCount (PropositionPtr proposition, int variable)
{
	int c;
	double result;
	result = 0.0;
	for (c=0;c<proposition->numOfClauses;c++)
	{
		result += clauseContains (proposition->clauses[c],variable)?1:0;
	}
	return result/proposition->numOfClauses;
}

double propositionListCount (ListPtr list, int variable,PropositionPtr trunk)
{
	double result;
	LinkPtr link;
	result = 0;
	for (link=list->first;link!=NULL;link=link->next)
		result+= propositionCount (link->element,variable);
	return result-20*propositionCount (trunk,variable);
}

int addToPropositionList (ListPtr l, PropositionPtr p)
{
	int found;
	LinkPtr link;
	found = false;
	for (link = l->first; !found && link != NULL; link=link->next)
		found = equalPropositions (link->element, p);
		
	if ( (p!=trueProposition && p != falseProposition) && !found)
		addListLast (l, p);
	else
		freeProposition (p);
		
	return !found;
}

PropositionPtr sortedCopyProposition (PropositionPtr p)
{
	PropositionPtr result;
	int c;
	result = newProposition(1);
	
	for (c=0;c<p->numOfClauses;c++)
		addPropositionClause (result, sortedCopyClause(p->clauses[c]));
		
	sortProposition (result);
	
	return result;
}
