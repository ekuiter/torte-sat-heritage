#ifndef TTS_H
#define TTS_H

#include "proposition.h"
#include "set.h"

extern double mylog2 (double x);

typedef struct SubProblemStruct
{
	int numOfVariables;
	PropositionPtr proposition, permProposition;
	int *mapVariable, *unMapVariable, *permute, *unPermute, *map,*unMap;
	AssignmentPtr assignment, permAssignment;
	int **d, **initialD, **r;
} SubProblem, *SubProblemPtr;



extern int *globalMapVariable, *globalUnMapVariable, *singletonUsage, *isSingleton, *globalUsed, *globalPermute,
*globalUnpermute, *globalTransform, *globalUnTransform;

#endif
