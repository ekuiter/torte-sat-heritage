#ifndef ASSIGNMENT_H



#define ASSIGNMENT_H

#include "intset.h"


typedef struct


{


	int numOfVariables;


	int *values;


} Assignment, *AssignmentPtr;

typedef struct AssignmentNodeStruct
{
	int v;
	struct AssignmentNodeStruct *pos,*neg,*both;
	int full;
} AssignmentNode, *AssignmentNodePtr;

extern AssignmentNodePtr newAssignmentNode();
extern AssignmentNodePtr addPartialAssignment (AssignmentNodePtr root, int pos, IntSetPtr partialAssignment);
extern IntSetPtr subsetAssignment (AssignmentNodePtr root, AssignmentPtr assignment, int limit);
extern int mergePartialAssignments (IntSetPtr result, IntSetPtr a, IntSetPtr b, int limit);



extern AssignmentPtr newAssignment();

extern void loadMappedAssignment (AssignmentPtr dest, AssignmentPtr src, int *map);


extern AssignmentPtr mapAssignment (AssignmentPtr assignment, int numOfVariables, int *map);


extern AssignmentPtr copyAssignment (AssignmentPtr assignment);


extern void  setAssignmentValue (AssignmentPtr Assignment, int variable, int value);

extern void setUsedAssignmentValues (AssignmentPtr assignment, int *globalMapVariable);

extern void setAssignmentNumOfVariables (AssignmentPtr Assignment, int numOfVariables)
;   





extern char *toStringAssignment(AssignmentPtr assignment)
;



extern void advanceAssignment(AssignmentPtr assignment)
;



#endif
