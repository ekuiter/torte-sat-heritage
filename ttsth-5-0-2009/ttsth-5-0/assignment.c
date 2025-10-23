#include "assignment.h"

#include "clause.h"

#include "proposition.h"

#include <string.h>





AssignmentPtr newAssignment()


{


	AssignmentPtr result;


	result = checkMalloc (sizeof (Assignment), "newAssignment");
	
	return result;


}

#define assignabs(i) ((i)>0?(i):-(i))

AssignmentNodePtr newAssignmentNode()
{
	AssignmentNodePtr result;
	result = (AssignmentNodePtr) checkMalloc (sizeof(AssignmentNode), "Assignmentnode");
	result->pos = NULL;
	result->neg = NULL;
	result->both = NULL;
	result->full = false;
	
	return result;
}

AssignmentNodePtr addPartialAssignment (AssignmentNodePtr root, int pos, IntSetPtr partialAssignment)
{
	int absv;
	AssignmentNodePtr next;
	
	if (root == NULL)
	{
		root = newAssignmentNode();
		if (pos == partialAssignment->numOfElements)
			root->full = true;
		else
			root->v = assignabs(partialAssignment->elements[pos]);
	}
	if (pos == partialAssignment->numOfElements)
		root->full = true;
	else
	{
		absv = assignabs(partialAssignment->elements[pos]);
		if (root->v < absv)
		{
			root->both = addPartialAssignment (root->both, pos, partialAssignment);
		}
		else
		{
			if (root->v > absv)
			{
				next = newAssignmentNode();
				next->v = absv;
				next->both = root;
				root = next;
			}
			if (partialAssignment->elements[pos] > 0)
				root->pos = addPartialAssignment (root->pos, pos+1, partialAssignment);
			else
				root->neg = addPartialAssignment (root->neg, pos+1, partialAssignment);
		}
	}
		
	return root;

}

void reverseIntSet (IntSetPtr s)
{
	int i,j, temp;
	i=0;
	j = s->numOfElements-1;
	while (i<j)
	{
		temp = s->elements[i];
		s->elements[i] = s->elements[j];
		s->elements[j] = temp;
		i++;
		j--;
	}
}
int subsetAssignmentRec (AssignmentNodePtr root, AssignmentPtr assignment, IntSetPtr s, int limit)
{
	int result;
	
	if (root == NULL)
		return false;
	else if (root->full)
		return true;
	else if (root->v >= limit)
		return false;
	else
	{
		if (assignment->values[root->v])
		{
			result = subsetAssignmentRec (root->pos, assignment, s, limit);
			if (result) addToIntSetNoCheck (s, root->v);
		}
		else
		{
			result = subsetAssignmentRec (root->neg, assignment, s, limit);
			if (result) addToIntSetNoCheck (s, -root->v);
		}

	   if (!result)
		result = subsetAssignmentRec (root->both, assignment, s, limit);
/*
		if (result)
			temp = newIntSet();
		else
			temp = s;
		
		bothResult = subsetAssignmentRec (root->both, assignment, temp, limit);
		
		if (bothResult && result)
		{
			if (temp->numOfElements < s->numOfElements)
				copyIntSet (s, temp);
			freeIntSet (temp);
		}
		result = result || bothResult;
*/
	}
	return result;
}

int subsetPartialAssignmentRec (AssignmentNodePtr root, IntSetPtr pa, int pos, int limit)
{
	int result, absv;
   
	if (root == NULL)
		return false;
	else if (root->full)
		return true;
	else if (root->v >= limit || pos >= pa->numOfElements)
		return false;
	else
	{
		absv = assignabs (pa->elements[pos]);
		if (root->v > absv)
			return subsetPartialAssignmentRec (root, pa, pos+1, limit);
		else if (root->v < absv)
			return subsetPartialAssignmentRec (root->both, pa, pos, limit);
		if (pa->elements[pos] > 0)
		{
			result = subsetPartialAssignmentRec (root->pos, pa, pos+1, limit);
		}
		else
		{
			result = subsetPartialAssignmentRec (root->neg, pa, pos+1, limit);
		}
		if (!result)
			result = subsetPartialAssignmentRec (root->both, pa, pos+1, limit);

	}
	return result;
}

IntSetPtr subsetAssignment (AssignmentNodePtr root, AssignmentPtr assignment, int limit)
{
	IntSetPtr s;
	s = newIntSet();
	if (subsetAssignmentRec (root, assignment, s, limit))
	{
		reverseIntSet (s);
		return s;
	}
	else
	{
		freeIntSet (s);
		return NULL;
	}
}

int subsetPartialAssignment (AssignmentNodePtr root, IntSetPtr pa, int limit)
{
	return subsetPartialAssignmentRec (root, pa, 0, limit);

}
int mergePartialAssignments (IntSetPtr result, IntSetPtr a, IntSetPtr b, int limit)
{
	int i,j;
	
	i = 0;
	j = 0;
	clearIntSet (result);
	while (i<a->numOfElements && assignabs(a->elements[i]) < limit && j<b->numOfElements && assignabs(b->elements[j]) < limit)
	{
		if (assignabs(a->elements[i]) < assignabs(b->elements[j]))
			addToIntSetNoCheck (result, a->elements[i++]);
		else
		{
			if (assignabs(a->elements[i]) == assignabs(b->elements[j]))
			{
				if (a->elements[i] != b->elements[j]) return false;
				i++;
			}
			addToIntSetNoCheck (result, b->elements[j++]);
		}
	}
	while (i<a->numOfElements && assignabs(a->elements[i]) < limit)
		addToIntSetNoCheck (result, a->elements[i++]);
	while (j<b->numOfElements && assignabs(b->elements[j]) < limit)
		addToIntSetNoCheck (result, b->elements[j++]);
		
	return true;
}


void setAssignmentNumOfVariables (AssignmentPtr assignment, int numOfVariables)


{


	int i;


	assignment->numOfVariables = numOfVariables;


	assignment->values = checkMalloc ((assignment->numOfVariables+1)*sizeof(int),"setAssignmentNumOfVariables");


	for (i=1;i<=assignment->numOfVariables;i++)


		assignment->values[i] = true;


}


	



void setAssignmentValue (AssignmentPtr assignment, int variable, int value)


{


	assignment->values[variable] = value;


}





static int stringAssignmentBufferLength=0;


static char *stringAssignmentBuffer=NULL;





char *toStringAssignment(AssignmentPtr assignment)


{


	char buffer[10];


	int i,length;


	length = 4+(assignment->numOfVariables)*9;


	if (stringAssignmentBuffer == NULL)


	{


		stringAssignmentBuffer = checkMalloc (length*sizeof(char), "toStringAssignment");


		stringAssignmentBufferLength = length;


	}


	else if (length > stringAssignmentBufferLength)


	{


		stringAssignmentBuffer = checkRealloc (stringAssignmentBuffer,length*sizeof(char), "toStringAssignment.realloc");


		stringAssignmentBufferLength = length;


	}


	strcpy (stringAssignmentBuffer,"");


	strcat (stringAssignmentBuffer,"[");


	for (i=1; i<=assignment->numOfVariables; i++)


	{


		if (assignment->values[i] == 1)


			sprintf (buffer, "%d", i);


		else 

if (assignment->values[i] == 0)
			sprintf (buffer, "%d", -i);


		else
			sprintf (buffer, "(%d)%d", assignment->values[i],i);

		
		strcat (stringAssignmentBuffer, buffer);


		if (i < assignment->numOfVariables)


			strcat (stringAssignmentBuffer,",");


	}


	strcat (stringAssignmentBuffer,"]");


	return stringAssignmentBuffer;


	


}



AssignmentPtr copyAssignment (AssignmentPtr assignment)
{
	AssignmentPtr result;
	int v;
	
	result = newAssignment();
	setAssignmentNumOfVariables (result, assignment->numOfVariables);
	for (v=1;v<=assignment->numOfVariables;v++)
		setAssignmentValue (result, v, assignment->values[v]);
	return result;
}

void loadMappedAssignment (AssignmentPtr dest, AssignmentPtr src, int *map)
{
	int v;
	for (v=1;v<=src->numOfVariables;v++)
		if (map[v] != 0)
			dest->values[map[v]] = src->values[v];
}

AssignmentPtr mapAssignment (AssignmentPtr assignment, int numOfVariables, int *map)
{
	AssignmentPtr result;
	
	if (assignment == NULL)
		result = NULL;
	else
	{
		result = newAssignment();
		setAssignmentNumOfVariables (result, numOfVariables);
		loadMappedAssignment (result, assignment, map);
	}
	
	return result;
}   
void setUsedAssignmentValues (AssignmentPtr assignment, int *used)
{
	int v;
	for (v=1;v<=assignment->numOfVariables;v++)
		assignment->values[v] = used[v] ? true: -1;
}


void advanceAssignment(AssignmentPtr assignment)


{


	int position=1;


	while (position<=assignment->numOfVariables && !assignment->values[position])


	{


		assignment->values[position] = true;


		position++;


	}


	if (position <= assignment->numOfVariables)


		assignment->values[position] = false;


}
