#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "tts.h"
#include "memutil.h"
#include "clause.h"
#include "assignment.h"
#include "proposition.h"
	

ClausePtr newClause()

{

	ClausePtr result;

	result = checkMalloc (sizeof(Clause), "newClause");

	result->numOfVariables = 0;
	
	//result->originalVariables = newIntSet();

	result->maxVariable = 0;

	result->variables = checkMalloc (sizeof(int),"newClause.variables");

	result->positive = checkMalloc (sizeof(int),"newClause.positive");

	return result;
}


void freeClause (ClausePtr clause)
{
	if (clause == NULL || clause == trueClause || clause == falseClause)
		return;
		
	free (clause->variables);
	free (clause->positive);
	free (clause);
}

ClausePtr minimizeClause (ClausePtr c)
{
	ClausePtr result;
	int v;
	
	result = newClause();


	for (v=0;v<c->numOfVariables;v++)
		if (c->positive[v])
			addClauseVariable (result, mapVariable[c->variables[v]]);
		else
			addClauseVariable (result, -mapVariable[c->variables[v]]);
			
	return result;
}
	
void sortClauseSet (SetPtr c)
{
	int i,j;
	
	qsort (c->elements, c->numOfElements, sizeof (ClausePtr), clauseCompare);
	
	j=0;
	for (i=1;i<c->numOfElements;i++)
		if (!equalClauses ((ClausePtr)c->elements[i],(ClausePtr)c->elements[j]))
		{
			j++;
			c->elements[j] = c->elements[i];
		}
		
	c->numOfElements = j+1;
	
}

int equalClauseSets (SetPtr s1, SetPtr s2)
{
	int i, result;
	if (s1->numOfElements != s2->numOfElements)
		return 0;
	else
	{
		result = 1;
		for (i=0; result && i<s1->numOfElements;i++)
			result = clauseCompare (&(s1->elements[i]),&(s2->elements[i])) == 0;
			
		return result;
	}
}

int oneVarSatisfiesClause (ClausePtr clause, int variable, int positive)

{

	int stillFalse,i;

	stillFalse = true;

	for (i=0;i<clause->numOfVariables&&stillFalse;i++)

		if (variable == clause->variables[i])

			stillFalse = positive != clause->positive[i];

	return !stillFalse;

}



void setClauseNumOfVariables (ClausePtr clause, int numOfVariables)

{


	if (numOfVariables > clause->numOfVariables)
	{
		clause->variables = checkRealloc (clause->variables, numOfVariables*sizeof(int),"setClauseNumOfVariables");
		clause->positive = checkRealloc (clause->positive, numOfVariables*sizeof(int),"setClauseNumOfVariables");
	}

	clause->numOfVariables = numOfVariables;
}

void addClauseVariableValue (ClausePtr clause, int variable, int positive)

{

//#ifdef PROOF
//    if (positive)
//        addToIntSet (clause->originalVariables, variable);
//    else
//        addToIntSet (clause->originalVariables, -variable);
//#endif
	
	clause->numOfVariables++;

	clause->variables = checkRealloc (clause->variables,clause->numOfVariables*sizeof(int),"setClauseVariable.1");

	clause->positive = checkRealloc (clause->positive,clause->numOfVariables*sizeof(int),"setClauseVariable.2");

	clause->variables[clause->numOfVariables-1] = variable;

	clause->positive[clause->numOfVariables-1] = positive;

	if (variable > clause->maxVariable)

		clause->maxVariable = variable;

}


void addClauseVariable (ClausePtr clause, int variable)

{

	int positive;
	if (variable > 0)
		positive = true;
	else
	{
		positive = false;
		variable = -variable;
	}

	addClauseVariableValue (clause, variable, positive);

}

ClausePtr trueClause, falseClause;

void initClauses()
{
	trueClause = newClause();
	falseClause = newClause();
}

int clauseContains (ClausePtr c, int v)
{
	int i,result;
	result = false;
	for(i=0;!result && i<c->numOfVariables;i++)
		result = c->variables[i] == v;
	return result;
}

void removeClauseVariable (ClausePtr clause, int variable)
{
	int pos,p;
	
	for (pos=0;pos<clause->numOfVariables && clause->variables[pos] != variable;pos++);
	if (pos < clause->numOfVariables)
	{
		for (p=pos; p<clause->numOfVariables; p++)
		{
			clause->variables[p] = clause->variables[p+1];
			clause->positive[p] = clause->positive[p+1];
		}
		clause->numOfVariables--;
	}
}

int equalClauses (ClausePtr clause1, ClausePtr clause2)
{
	int result, p;
	if (clause1->numOfVariables == clause2->numOfVariables)
	{
		result = true;
		for (p=0; result  &&
							p<clause1->numOfVariables &&
							clause1->variables[p] == clause2->variables[p] &&
							clause1->positive[p] == clause2->positive[p]; p++);
		if (p<clause1->numOfVariables)
			result = false;
	}
	else
		result = false;
	
	//printf ("%s %c= ", toStringClause(clause1),result?'=':'!');
	//printf ("%s\n",toStringClause(clause2));
	return result;
}

int clauseCountV (ClausePtr clause, int variable)
{
	int result;
	
	if (clauseContains (clause, variable))
	{
		switch (clause->numOfVariables)
		{
			case 1: result = 16;
					break;
					
			case 2: result = 8;
					break;
					
			case 3: result = 4;
					break;
					
			case 4: result = 2;
					break;
					
			default:
					result = 1;
					break;
		}
	}
	else
		result = 0;
	return -result;
}

int clauseCompare (const void *c1, const void *c2)
{
	ClausePtr clause1, clause2;
	int p, result;
	
	clause1 = * ( (ClausePtr *) c1);
	clause2 = * ( (ClausePtr *) c2);
	
	for (p=0; p<clause1->numOfVariables && p < clause2->numOfVariables && 
			clause1->variables[p] == clause2->variables[p] &&
			clause1->positive[p] == clause2->positive[p]; p++);
			
	if (p == clause1->numOfVariables)
	{
		if (p == clause2->numOfVariables)
			result = 0;
		else
			result = -1;
	}
	else if (p == clause2->numOfVariables)
		result = 1;
	else
	{
		if (clause1->variables[p] == clause2->variables[p])
		{  
			if (clause1->positive[p])
				result = 1;
			else
				result = -1;
		}
		else
		{
			if (clause1->variables[p] < clause2->variables[p])
				result = -1;
			else
				result = 1;
		}
	}
	return result;
}

int absVCompare (const void *c1, const void *c2)
{
	int i,j;

	i = * ( (int *) c1);
	j = * ( (int *) c2);
	
	if (i<0) i = -i;
	if (j<0) j = -j;
	
	if (i < j)
		return -1;
	else if (i==j)
		return 0;
	else
		return 1;
}


int clauseSize (ClausePtr clause)
{
	if (clause == trueClause || clause == falseClause)
		return 0;
	else
		return clause->numOfVariables;
}


int equalClausesExcept (ClausePtr clause1, ClausePtr clause2, int v)
{
	int result, p1, p2;
	char c1[100],c2[100];

	result = true;
	p1 = 0;
	p2 = 0;
	while(
			((p1 < clause1->numOfVariables && p2 < clause2->numOfVariables &&
			 clause1->variables[p1] == clause2->variables[p2] && clause1->positive[p1] == clause2->positive[p2]) ||
				clause1->variables[p1] == v || clause2->variables[p2] == v)
		)
	{
		if (clause1->variables[p1] != v && clause2->variables[p2] != v)
		{
			p1++; p2++;
		}
		else
		{
			if (clause1->variables[p1] == v)
				p1++;
			if (clause2->variables[p2] == v)
				p2++;
		}
	}
	while (p1 < clause1->numOfVariables && clause1->variables[p1] == v)
		p1++;
	while (p2 < clause2->numOfVariables && clause2->variables[p2] == v)
		p2++;
		
	result = p1 == clause1->numOfVariables && p2 == clause2->numOfVariables;
	
	if (false && result)
	{
		strcpy (c1, toStringClause (clause1));
		strcpy (c2, toStringClause (clause2));
		printf ("[%d] %s %s %s\n", v, c1, result?"==":"!=",c2);
	}
	
	return result;
}

ClausePtr reduceClause (ClausePtr clause, int variable, int val)
{
	ClausePtr result;
	int i,finished,retain;
	finished = false;
	
/*
	printf ("Reducing %s (%d,%d)", toStringClause(clause), variable, val);
*/
	result = newClause();


	retain = val == 2;
	for(i=0;!finished&&i<clause->numOfVariables;i++)
	{
		if (clause->variables[i] == variable)
		{
			retain = val != 2;
			if (val == clause->positive[i]?1:0)
			{
				freeClause (result);
				result = trueClause;
				finished = true;
			}
		}
		else
		{
			addClauseVariableValue (result, clause->variables[i], clause->positive[i]);
		}
	}
	if (result != trueClause)
	{
		if (!retain)
		{
			freeClause (result);
			result = trueClause;
		}
		else
		if (result->numOfVariables == 0)
		{
			freeClause (result);
			result = falseClause;
		}
	}

/*   printf (" = %s\n", result==trueClause?"T":result==falseClause?"F":toStringClause(result)); */
	return result;
}

ClausePtr clauseCurried (ClausePtr c, int v, int positive)
{
	ClausePtr result;
	int i,finished;
	finished = false;
	result = newClause();

	for(i=0;!finished&&i<c->numOfVariables;i++)
	{
		if (c->variables[i] == v)
		{
			if (positive == c->positive[i])
			{
				freeClause (result);
				result = trueClause;
				finished = true;
			}
			else
			{
			}
		}
		else
		{
			addClauseVariableValue (result, c->variables[i], c->positive[i]);
		}
	}

	if (result != trueClause && result->numOfVariables == 0)
	{
		freeClause(result);
		result = falseClause;
	}
	return result;
}

int evaluateClause (ClausePtr clause, AssignmentPtr assignment)

{

	int stillFalse,i;

	stillFalse = true;

	for (i=0; i<clause->numOfVariables && stillFalse; i++)

	{
		stillFalse = clause->positive[i] != assignment->values[clause->variables[i]];

	}
	return !stillFalse;

}
int evaluateUnMappedClause (ClausePtr clause, AssignmentPtr assignment)

{

	int stillFalse,i;

	stillFalse = true;

	for (i=0; i<clause->numOfVariables && stillFalse; i++)

	{
		stillFalse = clause->positive[i] != assignment->values[clause->variables[i]];

	}
	return !stillFalse;

}
void removeClauseDuplicates (ClausePtr c)
{
	int i,j,k, required;
	i=0;j=0;
	while (i<c->numOfVariables)
	{
		required = true;
		for (k=i+1;k<c->numOfVariables && required;k++)
			required = c->variables[i] != c->variables[k] || c->positive[i] != c->positive[k];
		if (required)
		{
			if (i != j)
			{
				c->variables[j] = c->variables[i];
				c->positive[j] = c->positive[i];
			}
			j++;
		}
		i++;
	}
	c->numOfVariables = j;
}
void removeSortedClauseDuplicates (ClausePtr c)
{
	int i,j,required;
	i=0;j=0;
	while (i<c->numOfVariables)
	{
		required = true;
		if (i+1<c->numOfVariables)
			required = c->variables[i] != c->variables[i+1] || c->positive[i] != c->positive[i+1];
		if (required)
		{
			if (i != j)
			{
				c->variables[j] = c->variables[i];
				c->positive[j] = c->positive[i];
			}
			j++;
		}
		i++;
	}
	c->numOfVariables = j;
}

static int stringClauseBufferLength=0;

static char *stringClauseBuffer=NULL;

void freeClauseSet (SetPtr s)
{
	int i;
	for (i=0;i<s->numOfElements;i++)
		freeClause (s->elements[i]);
	freeSet (s);
}

ClausePtr newCopyClause(ClausePtr clause)
{
	ClausePtr result;
	int i;
	
	result = newClause();

#ifdef PROOF
//    result->originalVariables = clause->originalVariables;
	result->resId = clause->resId;
	
#endif

	setClauseNumOfVariables (result, clause->numOfVariables);
	for (i=0;i<clause->numOfVariables;i++)
	{
		result->variables[i] = clause->variables[i];
		result->positive[i] = clause->positive[i];
	}
	return result;
}

ClausePtr sortedCopyClause(ClausePtr clause)
{
	ClausePtr result;
   
	result = newCopyClause(clause);
	sortClause (result);


	

	return result;
}

void sortClause(ClausePtr clause)
{
	int i;
	int *tempValues;
	
	//printf ("in->%s\n", toStringClause(clause));
	tempValues = (int *) checkMalloc (sizeof(int)*clause->numOfVariables,"sortClause");
	for (i=0;i<clause->numOfVariables;i++)
		tempValues[i] = clause->variables[i]*(clause->positive[i]?1:-1);
		
	qsort (tempValues, clause->numOfVariables, sizeof (int), absVCompare);
	
	for (i=0;i<clause->numOfVariables;i++)
	{
		clause->positive[i] = tempValues[i] > 0;
		clause->variables[i] = tempValues[i]*(clause->positive[i]?1:-1);
	}
	free (tempValues);
	//printf ("out->%s\n", toStringClause(clause));
}

ClausePtr mapClause (ClausePtr clause, int *map)
{
	int i;
	ClausePtr result;
	
	if (clause == NULL)
		return NULL;
	
	for (i=0;i<clause->numOfVariables;i++)
	{
		if (map[clause->variables[i]] ==0)
		   return NULL; 
	}

	//result = newCopyClause(clause);
	result = clause;
	result->maxVariable = 0;
	
	for (i=0;i<clause->numOfVariables;i++)
	{
		result->variables[i] = map[clause->variables[i]];
		if (result->variables[i] > result->maxVariable)
			result->maxVariable = result->variables[i];
	}
	sortClause(result);
	return result;
}


char *toStringClause(ClausePtr clause)

{

	char buffer[10];

	int i,length;

	if (clause == NULL)
		return "<>";
		
	length = 4+clause->numOfVariables*6;

	if (stringClauseBuffer == NULL)

	{

		stringClauseBuffer = checkMalloc (length*sizeof(char), "toStringClause");

		stringClauseBufferLength = length;

	}

	else if (length > stringClauseBufferLength)

	{

		stringClauseBuffer = checkRealloc (stringClauseBuffer,length*sizeof(char), "toStringClause.realloc");

		stringClauseBufferLength = length;

	}

	strcpy (stringClauseBuffer,"");

	strcat (stringClauseBuffer,"(");

	for (i=0; i<clause->numOfVariables; i++)

	{

		if (clause->positive[i])

			sprintf(buffer,"%d",/*globalUnTransform[*/clause->variables[i]);

		else

			sprintf(buffer,"%d",-/*globalUnTransform[*/clause->variables[i]);

		strcat (stringClauseBuffer, buffer);

		if (i < clause->numOfVariables-1)

			strcat (stringClauseBuffer," ");

	}

	strcat (stringClauseBuffer, ")");

	return stringClauseBuffer;

}




ClausePtr readClause (FILE *f)

{

	int ch;

	ClausePtr result;

	int variable;

	result = NULL;

	do

	{

		ch = fgetc(f);

	} while ( ch != EOF && ch != '(' && ch != '.');

	if (ch == '(')

	{

		result = newClause();

		while (fscanf (f, "%d", &variable) == 1)

		{

			addClauseVariable (result, variable);

		}

		do

		{

			ch = fgetc (f);

		} while (ch != ')');

	}

	return result;

}

int isClauseTautology (ClausePtr clause)
{
	int result, i, j;
	result = false;
	for (i=0;i<clause->numOfVariables && !result;i++)
		for (j=i+1;j<clause->numOfVariables && !result; j++)
			result = clause->variables[i] == clause->variables[j] && clause->positive[i] != clause->positive[j];
	
	return result;
}

int minVariable(ClausePtr clause)
{
	int v;
	int result = clause->variables[0];
	
	for (v=1;v<clause->numOfVariables;v++)
		if (clause->variables[v] < result)
			result = clause->variables[v];
			
	return result;
}
