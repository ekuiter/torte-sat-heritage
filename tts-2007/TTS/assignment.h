#ifndef ASSIGNMENT_H



#define ASSIGNMENT_H



typedef struct


{


	int numOfVariables;


	int *values;


} Assignment, *AssignmentPtr;





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
