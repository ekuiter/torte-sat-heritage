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
